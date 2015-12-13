#include "myHead.h"

using namespace std;

bool hasEnding(string const &fullString, string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}

int myTcpSend(int socket, const char *b, int bufferSize) {

	int result;
	int send_size = 0;
	char *buffer = (char *) b;

	// ensure all send out
	while (send_size < bufferSize) {
		result = send(socket, buffer + send_size, bufferSize - send_size, MSG_NOSIGNAL);
		if (result == 0 || result == -1) return result;
		send_size += result;
	}

	return send_size;
}

int myTcpRecv(int socket, const char *b, int bufferSize) {

	int result;
	int recv_size = 0;
	char *buffer = (char *) b;

	// ensure all send out
	while (recv_size < bufferSize) {
		result = recv(socket, buffer + recv_size, bufferSize - recv_size, 0);
		if (result == 0 || result == -1) return result;
		recv_size += result;
	}

	return recv_size;
}

bool myContentToStream(int socket, std::ostream &os, int contentSize) {

	// buffer
	int bufferSize = BUFFER_SIZE;
	char buffer[BUFFER_SIZE];

	int gotSize = 0, result;
	while (gotSize < contentSize) {
		result = myTcpRecv(socket, buffer, min(bufferSize, contentSize - gotSize));
		if (result <= 0) return false;

		gotSize += result;
		os.write(buffer, result);
	}

	return true;
}

bool myHttpHeaderRecv(int socket, char *buffer, int bufferSize, int *httpPackageSizeGot) {

	char *headerEnding;
	int gotBytes = 0, result;
	memset(buffer, '\0', bufferSize);

	int endingLength = strlen(HTTP_REQUEST_ENDING);

	// loop until request end
	do {
		// provide getting >1 packet at the same time...
		result = recv(socket, buffer + gotBytes, 1, 0);
		if (result == 0) {
			perror("myHttpHeaderRecv(): unexpected client socket close");
			return false;
		}
		if (result == -1) {
			perror("recv()");
			return false;
		}
		gotBytes += result;
		if (gotBytes >= bufferSize - 1) return false;

		// search for request ending
		headerEnding = strstr(buffer + gotBytes - endingLength, HTTP_REQUEST_ENDING);

	} while (headerEnding == NULL);

	// append NULL at the end
	*headerEnding = '\0';
	*httpPackageSizeGot = gotBytes;

	return true;
}

bool myHttpBodyRecv(int socket, char *headerBuffer, int bufferSize, int httpPackageSizeGot, std::ostream &os) {

	// get content length from header (find start position, find ending position, convert to int)
	int contentLength = 0;
	char *contentLengthBegin = strstr(headerBuffer, HTTP_CONTENT_HEADER);
	if (contentLengthBegin != NULL) {

		// skip header prefix
		contentLengthBegin += strlen(HTTP_CONTENT_HEADER);

		// content-length may be last line of the header
		char *contentLengthEnd = strstr(contentLengthBegin, constants::HTTP_line_break.c_str());
		string tmpString;
		if (contentLengthEnd == NULL) {
			tmpString.assign(contentLengthBegin);
		} else {
			tmpString.assign(contentLengthBegin, contentLengthEnd - contentLengthBegin);
		}

		// string to int
		contentLength = stoi(tmpString);
	}

	cout << "[myHttpBodyRecv()]\t" << "contentLength: " << contentLength << endl;
	
	// write body content to stream buffer
	if (contentLength > 0) {

		// take body from buffer
		char *bodyStart = headerBuffer + strlen(headerBuffer) + strlen(HTTP_REQUEST_ENDING);
		int bodyGot = httpPackageSizeGot - (bodyStart - headerBuffer);
		os.write(bodyStart, bodyGot);

		cout << "[myHttpBodyRecv()]\t" << "bodyStart: " << bodyStart << endl;
		cout << "[myHttpBodyRecv()]\t" << "bodyGot: " << bodyGot << endl;
		cout << "[myHttpBodyRecv()]\t" << "take more body: " << (contentLength - bodyGot > 0) << endl;

		// take body from socket
		if (contentLength - bodyGot > 0) {
			if (!myContentToStream(socket, os, contentLength - bodyGot)) {
				perror("Error: myContentToStream() during myRequestRecv()");
				return false;
			}
		}
	}

	return true;
}

bool myResponseRecv(int socket, std::ostream &os) {

	int httpPackageSizeGot;
	char buffer[BUFFER_SIZE];
	int bufferSize = BUFFER_SIZE;

	// get response header from socket
	if (!myHttpHeaderRecv(socket, buffer, bufferSize, &httpPackageSizeGot)) {
		perror("Error: myResponseRecv() => myHttpHeaderRecv()");
		return false;
	}

	// debug
	cout << "myResponseRecv() => buffer:\n" << buffer << endl;
	cout << endl;

	// check 200 OK
	if (strstr(buffer, "200") != (buffer + constants::REQUEST_default_http_version.length() + constants::HTTP_inline_delimiter.length())) {
		perror("Error: myResponseRecv() => HTTP not 200 OK");
		return false;
	}

	// get body
	if (!myHttpBodyRecv(socket, buffer, bufferSize, httpPackageSizeGot, os)) {
		perror("Error: myResponseRecv() => myHttpBodyRecv()");
		return false;
	}

	return true;
}

bool myRequestRecv(int socket, char *buffer, int bufferSize, std::ostringstream &oss) {

	int httpPackageSizeGot;

	// get request header from socket
	if (!myHttpHeaderRecv(socket, buffer, bufferSize, &httpPackageSizeGot)) {
		perror("Error: myRequestRecv() => myHttpHeaderRecv()");
		return false;
	}

	// only GET request expected, no body part
	if (strncmp(buffer, constants::HTTP_GET.c_str(), constants::HTTP_GET.length()) == 0) return true;

	// get body
	if (!myHttpBodyRecv(socket, buffer, bufferSize, httpPackageSizeGot, oss)) {
		perror("Error: myRequestRecv() => myHttpBodyRecv()");
		return false;
	}

	cout << "myRequestRecv() => header:\n" << buffer << endl;

	return true;
}

bool parseAndValidateRequest(const std::string &request, std::string &method, std::string &url, std::string &httpVersion) {

	// parse first line
	size_t lastPosition = 0, newPosition = 0;
	newPosition = request.find(constants::HTTP_line_break);
	// request may only have 1 line
	std::string requestLine;
	if (newPosition == std::string::npos) requestLine = request;
	else requestLine = request.substr(lastPosition, newPosition);

	// get request method
	newPosition = requestLine.find(constants::HTTP_inline_delimiter, lastPosition);
	if (newPosition == std::string::npos) return false;
	method = request.substr(lastPosition, newPosition - lastPosition);
	lastPosition = newPosition + constants::HTTP_inline_delimiter.length();

	// get request url
	newPosition = requestLine.find(constants::HTTP_inline_delimiter, lastPosition);
	if (newPosition == std::string::npos) return false;
	url = request.substr(lastPosition, newPosition - lastPosition);
	lastPosition = newPosition + constants::HTTP_inline_delimiter.length();

	// parse absolute path, skip the domain name or IP part
	if (url.compare(0, constants::HTTP_absolute_path_prefix.length(), constants::HTTP_absolute_path_prefix) == 0) {
		newPosition = url.find(constants::HTTP_path_delimiter, constants::HTTP_absolute_path_prefix.length());
		url = url.substr(newPosition);
	}
	// get request httpVersion
	httpVersion = request.substr(lastPosition, requestLine.length() - lastPosition);
	if (httpVersion.compare("HTTP/1.0") != 0 && httpVersion.compare("HTTP/1.1") != 0) return false;

	// cout << method << endl;
	// cout << url << endl;
	// cout << httpVersion << endl;

	return true;
}





void construtHttpResponseHeader(std::string &responseHeader, bool isOK, const std::string &httpVersion, const std::string &contentType, long contentLength, bool isKeepAlive) {

	responseHeader = httpVersion + constants::HTTP_inline_delimiter;
	if (!isOK) {
		responseHeader += "404 Not Found" + constants::HTTP_line_break;
	} else {
		responseHeader += "200 OK" + constants::HTTP_line_break;
		if (contentLength > 0L) {
			responseHeader += "Content-Type: " + contentType + constants::HTTP_line_break;
			responseHeader += "Content-Length: " + to_string(contentLength) + constants::HTTP_line_break;
		}
		if (isKeepAlive) {
			responseHeader += "Connection: keep-alive" + constants::HTTP_line_break;
		}
	}

	// mark header ending
	responseHeader += constants::HTTP_line_break;
}

bool getFileResponse(int socket, const std::string &filePath, const std::string &httpVersion, bool isKeepAlive) {

	// open file
	FILE *file;
	file = fopen(filePath.c_str(), "rb");
	size_t fileBufferSize = BUFFER_SIZE, readBytes;
	char fileBuffer[BUFFER_SIZE];

	// handle and send file
	std::string responseHeader;
	if (file == NULL) {
		// file not exist
		construtHttpResponseHeader(responseHeader, false, httpVersion, NULL, 0L, false);
		if (myTcpSend(socket, responseHeader.c_str(), responseHeader.length()) <= 0) return false;
	} else {

		// TODO check file isDir

		// get file size
		fseek(file, 0L, SEEK_END);
		const long contentLength = ftell(file);
		fseek(file, 0L, SEEK_SET);

		// create and send header
		std::string contentType("application/octet-stream");
		construtHttpResponseHeader(responseHeader, true, httpVersion, contentType, contentLength, isKeepAlive);
		if (myTcpSend(socket, responseHeader.c_str(), responseHeader.length()) <= 0) return false;

		// read and send file content
		int sentResult;
		long sentTotal = 0;
		size_t bytesShouldSent;
		do {
			// read from file
			bytesShouldSent = (std::min)(contentLength - sentTotal, (long) fileBufferSize);
			readBytes = fread(fileBuffer, 1, bytesShouldSent, file);
			if (readBytes != bytesShouldSent) return false;

			// send to client
			sentResult = myTcpSend(socket, fileBuffer, readBytes);
			if (sentResult <= 0) return false;
			sentTotal += bytesShouldSent;
		} while (sentTotal < contentLength);

		fclose(file);
	}

	// debug
	cout << "\ngetFileResponse() => header:\n" << responseHeader << endl;

	return true;
}

bool contentResponse(int socket, const std::string &httpVersion, bool isKeepAlive, const std::string &content, long contentLength) {

	std::string responseHeader;
	std::string contentType("application/octet-stream");

	construtHttpResponseHeader(responseHeader, true, httpVersion, contentType, contentLength, isKeepAlive);
	if (myTcpSend(socket, responseHeader.c_str(), responseHeader.length()) <= 0) return false;
	if (contentLength > 0) {
		cout << "contentResponse() => contentLength: " << contentLength << endl;
		if (myTcpSend(socket, content.c_str(), contentLength) <= 0) return false;
	}
	
	// debug
	cout << "contentResponse() => header:\n" << responseHeader;
	cout << "contentResponse() => content:\n" << content << endl;
	cout << endl;

	return true;
}

bool createAndSendResponse(int socket, const std::string &url, const std::string &httpVersion, const std::string &content, long contentLength) {

	if (url.length() != 0) {
		return getFileResponse(socket, url, httpVersion, false);
	} else {
		return contentResponse(socket, httpVersion, true, content, contentLength);
	}

	return false;
}

bool createAndSendOK(int socket, const std::string &httpVersion) {
	std::string responseHeader;
	construtHttpResponseHeader(responseHeader, true, httpVersion, constants::EMPTY_STRING, 0L, true);
	if (myTcpSend(socket, responseHeader.c_str(), responseHeader.length()) <= 0) return false;
	// debug
	cout << "createAndSendOK() => header\n" << responseHeader << endl;

	return true;
}

bool createAndSendRequest(int socket, bool isGet, const std::string &url, const std::string &httpVersion, bool isKeepAlive, const char *content, int contentSize) {

	// 1st request line
	std::string header = ((isGet) ? constants::HTTP_GET : constants::HTTP_POST);
	header += constants::HTTP_inline_delimiter + url;
	header += constants::HTTP_inline_delimiter + httpVersion + constants::HTTP_line_break;

	// keep alive header
	if (isKeepAlive) {
		header += constants::HTTP_connection_keep_alive + constants::HTTP_line_break;
	}

	// content type and length header
	if (!isGet && contentSize > 0) {
		header += constants::POST_content_type + constants::HTTP_line_break;
		header += "Content-Length: " + std::to_string(contentSize) + constants::HTTP_line_break;
	}

	// header ending
	header += constants::HTTP_line_break;

	// send header and content
	if (myTcpSend(socket, header.c_str(), header.length()) <= 0) return false;
	// debug
	cout << "createAndSendRequest() => header\n" << header;
	if (!isGet && contentSize > 0) {
		if (myTcpSend(socket, content, contentSize) <= 0) return false;
		// debug
		cout << "createAndSendRequest() => body\n" << content << endl;
	}

	return true;
}
