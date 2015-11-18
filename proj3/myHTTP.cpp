#include "myHead.h"

using namespace std;

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

bool myRequestRecv(int socket, char *buffer, int bufferSize) {

	int gotBytes = 0, result;

	// loop until empty line reached
	do {
		result = recv(socket, buffer + gotBytes, bufferSize - gotBytes, 0);
		if (result == 0) break;
		if (result == -1) {
			perror("recv()");
			return false;
		}
		gotBytes += result;
		if (gotBytes >= bufferSize - 1) return false;

	} while (buffer[gotBytes - 4] != '\r'|| buffer[gotBytes - 3] != '\n'|| buffer[gotBytes - 2] != '\r'|| buffer[gotBytes - 1] != '\n');

	// append NULL at the end
	buffer[gotBytes] = '\0';

	return true;
}

bool parseAndValidateRequest(std::string const &request, std::string &method, std::string &url, std::string &httpVersion) {

	const std::string requestLineDelimiter = " ";
	const std::string httpDelimiter = "\r\n";
	const std::string pathDelimiter = "/";
	const std::string absolutePathPrefix = "http://";

	size_t lastPosition = 0, newPosition = 0;
	newPosition = request.find(httpDelimiter);
	if (newPosition == std::string::npos) return false;
	std::string requestLine = request.substr(lastPosition, newPosition);

	// get request method
	newPosition = requestLine.find(requestLineDelimiter, lastPosition);
	if (newPosition == std::string::npos) return false;
	method = request.substr(lastPosition, newPosition - lastPosition);
	if (method.compare("GET") != 0) return false;
	lastPosition = newPosition + requestLineDelimiter.length();

	// cout << method << endl;

	// get request url
	newPosition = requestLine.find(requestLineDelimiter, lastPosition);
	if (newPosition == std::string::npos) return false;
	url = request.substr(lastPosition, newPosition - lastPosition);
	lastPosition = newPosition + requestLineDelimiter.length();

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

bool createAndSendResponse(int socket, std::string const &filePath, std::string const &httpVersion) {

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
	size_t fileBufferSize = 4096, readBytes;
	char fileBuffer[fileBufferSize];

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
		responseHeader += " 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + to_string(contentLength) + "\r\n\r\n";
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
