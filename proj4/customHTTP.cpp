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

bool myContentToStream(int socket, ostringstream &oss, int contentSize) {

	// buffer
	int bufferSize = BUFFER_SIZE;
	char buffer[BUFFER_SIZE];

	int gotSize = 0, result;
	while (gotSize < contentSize) {
		result = myTcpRecv(socket, buffer, min(bufferSize, contentSize - gotSize));
		if (result <= 0) return false;

		gotSize += result;
		oss.write(buffer, result);
	}

	return true;
}

bool myRequestRecv(int socket, char *buffer, int bufferSize, std::ostringstream &oss) {

	int gotBytes = 0, result;
	char *requestEnding;
	memset(buffer, '\0', bufferSize);

	// loop until request end
	do {
		result = recv(socket, buffer + gotBytes, bufferSize - gotBytes, 0);
		if (result == 0) break;
		if (result == -1) {
			perror("recv()");
			return false;
		}
		gotBytes += result;
		if (gotBytes >= bufferSize - 1) return false;

		// search for request ending
		requestEnding = strstr(buffer, HTTP_REQUEST_ENDING);

	} while (requestEnding == NULL);

	// append NULL at the end
	*requestEnding = '\0';

	// only GET request expected, no body part
	if (oss == NULL) return true;

	// get content length from header
	int contentLength = 0;
	char *contentLengthBegin = strstr(buffer, HTTP_CONTENT_HEADER);
	if (contentLengthBegin != NULL) {
		contentLengthBegin += strlen(HTTP_CONTENT_HEADER);
		char *contentLengthEnd = strstr(contentLengthBegin, HTTP_line_break.c_str());
		string tmpString(contentLengthBegin, contentLengthEnd - contentLengthBegin);
		contentLength = stoi(tmpString);
	}

	// write body content to stream buffer
	if (contentLength > 0) {

		// take body from buffer
		char *bodyStart = requestEnding + strlen(HTTP_REQUEST_ENDING);
		int bodyGot = gotBytes - (bodyStart - buffer);
		oss.write(bodyStart, bodyGot);

		// take body from socket
		if (contentLength - bodyGot > 0) {
			if (!myContentToStream(socekt, oss, contentLength - bodyGot)) {
				perror("Error: myContentToStream() during myRequestRecv()");
				return false;
			}
		}
	}

	return true;
}

bool parseAndValidateRequest(std::string const &request, std::string &method, std::string &url, std::string &httpVersion) {

	// parse first line
	size_t lastPosition = 0, newPosition = 0;
	newPosition = request.find(constants::HTTP_line_break);
	if (newPosition == std::string::npos) return false;
	std::string requestLine = request.substr(lastPosition, newPosition);

	// get request method
	newPosition = requestLine.find(constants::HTTP_inline_delimiter, lastPosition);
	if (newPosition == std::string::npos) return false;
	method = request.substr(lastPosition, newPosition - lastPosition);
	if (method.compare("GET") != 0) return false;
	lastPosition = newPosition + constants::HTTP_inline_delimiter.length();

	// cout << method << endl;

	// get request url
	newPosition = requestLine.find(constants::HTTP_inline_delimiter, lastPosition);
	if (newPosition == std::string::npos) return false;
	url = request.substr(lastPosition, newPosition - lastPosition);
	lastPosition = newPosition + constants::HTTP_inline_delimiter.length();

	// parse absolute path
	if (url.compare(0, absolutePathPrefix.length(), absolutePathPrefix) == 0) {
		newPosition = url.find(pathDelimiter, absolutePathPrefix.length());
		url = url.substr(newPosition);
	}

	// cout << url << endl;

	// get request httpVersion
	httpVersion = request.substr(lastPosition, requestLine.length() - lastPosition);
	if (httpVersion.compare("HTTP/1.0") != 0 && httpVersion.compare("HTTP/1.1") != 0) return false;

	// cout << httpVersion << endl;

	return true;
}

bool createAndSendGetResponse(int socket, std::string const &filePath, std::string const &httpVersion) {

	// file path
	std::string htmlFolderPath = "./html";
	std::string path = htmlFolderPath + filePath;

	// FK windows
	#ifdef WIN32
		std::replace(path.begin(), path.end(), '/', '\\');
	#endif

	// open file
	FILE *file;
	file = fopen(path.c_str(), "rb");
	size_t fileBufferSize = BUFFER_SIZE, readBytes;
	char fileBuffer[BUFFER_SIZE];

	// handle and send file
	std::string responseHeader = httpVersion;
	if (file == NULL) {
		// file not exist
		responseHeader += " 404 Not Found\r\n\r\n";
		myTcpSend(socket, responseHeader.c_str(), responseHeader.length());
	} else {
		// get file size
		fseek(file, 0L, SEEK_END);
		const long contentLength = ftell(file);
		fseek(file, 0L, SEEK_SET);

		// create and send header
		responseHeader += " 200 OK\r\n";
		if (hasEnding(path, string(".html")) || hasEnding(path, string(".htm"))) {
			responseHeader += "Content-Type: text/html\r\n";
		} else if (hasEnding(path, string(".jpg")) || hasEnding(path, string(".jpeg"))) {
			responseHeader += "Content-Type: image/jpeg\r\n";
		} else if (hasEnding(path, string(".png"))) {
			responseHeader += "Content-Type: image/png\r\n";
		} else {
			responseHeader += "Content-Type: application/octet-stream\r\n";
		}
		responseHeader += "Content-Length: " + to_string(contentLength) + "\r\n\r\n";
		myTcpSend(socket, responseHeader.c_str(), responseHeader.length());

		// read and send file content
		int sentResult;
		long sentTotal = 0;
		size_t bytesShouldSent;
		do {
			// read from file
			bytesShouldSent = std::min(contentLength - sentTotal, (long) fileBufferSize);
			readBytes = fread(fileBuffer, 1, bytesShouldSent, file);
			if (readBytes != bytesShouldSent) return false;

			// send to client
			sentResult = myTcpSend(socket, fileBuffer, readBytes);
			if (sentResult <= 0) return false;
			sentTotal += bytesShouldSent;
		} while (sentTotal < contentLength);

		fclose(file);
	}

	return true;
}

bool createAndSendGetResponse(int socket, std::string const &url, std::string const &httpVersion) {

	std::string header("GET");
	header += constants::HTTP_inline_delimiter + url;
	header += constants::HTTP_inline_delimiter + httpVersion + constants::HTTP_line_break;

	std::string connectionKeepAlive("Connection: Keep-Alive");
	header += connectionKeepAlive + constants::HTTP_line_break;

	// header ending
	header += constants::HTTP_line_break;

	// send header and content
	if (myTcpSend(socket, header.c_str(), header.length()) <= 0) return false;

	return true;
}

bool createAndSendPostResponse(int socket, std::string const &url, std::string const &httpVersion, const char *content, int contentSize) {

	std::string header("POST");
	header += constants::HTTP_inline_delimiter + url;
	header += constants::HTTP_inline_delimiter + httpVersion + constants::HTTP_line_break;

	std::string connectionKeepAlive("Connection: Keep-Alive");
	header += connectionKeepAlive + constants::HTTP_line_break;

	std::string contentType("Content-Type: application/x-www-form-urlencoded");
	header += contentType + constants::HTTP_line_break;

	std::string contentLength("Content-Length: ");
	header += contentLength + std::to_string(contentSize) + constants::HTTP_line_break;

	// header ending
	header += constants::HTTP_line_break;

	// send header and content
	if (myTcpSend(socket, header.c_str(), header.length()) <= 0) return false;
	if (myTcpSend(socket, content, contentSize) <= 0) return false;

	return true;
}
