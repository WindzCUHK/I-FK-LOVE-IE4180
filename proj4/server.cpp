#include "myHead.h"

std::mutex g_display_mutex;


bool createAndSendFileList(bool isRestore, int socket, const std::string &httpVersion, const std::string &monitorPath, std::ostringstream &oss) {

	// generate file list
	std::vector<FileMeta> fileMetas;
	if (listAllFilesInDir(fileMetas, monitorPath, isRestore) == EXIT_FAILURE) {
		oss << "Error: Cannot monitor directory!!!\n";
		return false;
	}

	// send file list
	std::string contentString = cgicc::form_urlencode(encodeFileMetas(fileMetas));
	if (!createAndSendResponse(socket, constants::EMPTY_STRING, httpVersion, contentString, contentString.length())) {
		oss << "Error: file IO OR send()\n";
		return false;
	}

	return true;
}

void httpHandler(int socket, std::string &monitorPath) {

	// thread output stream
	std::ostringstream oss, bodyss;
	oss << "--- httpHandler thread start\n";

	// request buffer
	char buffer[BUFFER_SIZE];
	int bufferSize = BUFFER_SIZE;

	// take out the whole request header
	if (!myRequestRecv(socket, buffer, bufferSize, bodyss)) {
		oss << "Error: recv() error OR request >= 4096 bytes\n";
		threadPrint(oss.str().c_str(), "\n");
		mySocketClose(socket);
		return;
	}
	oss << "HTTP request:\n" << buffer << '\n';

	// process request
	const std::string requestString = buffer;
	std::string method, url, httpVersion;
	if (!parseAndValidateRequest(requestString, method, url, httpVersion)) {
		oss << "Error: Unknown request header\n";
		threadPrint(oss.str().c_str(), "\n");
		mySocketClose(socket);
		return;
	}

	// create response
	oss << "Request file path: " << url << '\n';
	if (method == constants::HTTP_POST) {
		// POST /delete
		if (url.compare(0, constants::SERVER_delete_path.length(), constants::SERVER_delete_path) == 0) {
		}
		// POST /create
		if (url.compare(0, constants::SERVER_create_path.length(), constants::SERVER_create_path) == 0) {
		}
		// POST /update
		if (url.compare(0, constants::SERVER_update_path.length(), constants::SERVER_update_path) == 0) {
		}

	} else if (method == constants::HTTP_GET) {

		// GET /list
		if (url.compare(0, constants::SERVER_list_path.length(), constants::SERVER_list_path) == 0) {
			if (!createAndSendFileList(false, socket, httpVersion, monitorPath, oss)) {
				threadPrint(oss.str().c_str(), "\n");
				mySocketClose(socket);
				return;
			}
		}
		// GET /restoreList
		if (url.compare(0, constants::SERVER_restore_list_path.length(), constants::SERVER_restore_list_path) == 0) {
			if (!createAndSendFileList(true, socket, httpVersion, monitorPath, oss)) {
				threadPrint(oss.str().c_str(), "\n");
				mySocketClose(socket);
				return;
			}
		}
		// GET /restore
		if (url.compare(0, constants::SERVER_restore_path.length(), constants::SERVER_restore_path) == 0) {
			std::string filePath = url.substr(constants::SERVER_restore_path.length());
			if (!createAndSendResponse(socket, monitorPath + filePath, httpVersion, constants::EMPTY_STRING, 0L)) {
				oss << "Error: file IO OR send()\n";
				threadPrint(oss.str().c_str(), "\n");
				mySocketClose(socket);
				return;
			}
		}

	} else {
		oss << "Error: Unknown request method\n";
		threadPrint(oss.str().c_str(), "\n");
		mySocketClose(socket);
		return;
	}
}

void acceptLoop(int socket, std::string &monitorPath, int port) {
	int clientSocket;
	struct sockaddr_in clientAddress;
	socklen_t addressSize = sizeof(clientAddress);

	threadPrint("Waiting connection... on port ", std::to_string(port).c_str());
	while (true) {

		// accept new socket
		memset((char *) &clientAddress, 0, addressSize);
		clientSocket = accept(socket, (struct sockaddr *) &clientAddress, &addressSize);
		if (clientSocket == -1) {
			perror("accept()");
			continue;
		}

		// new thread to handle the POST request
		std::thread httpHandlerThread(httpHandler, clientSocket, std::cref(monitorPath));
		httpHandlerThread.detach();
	}
}

int main(int argc, char *argv[]) {

	// check argument
	if (argc < 3) {
		std::cout << "Usage: server.exe [sync folder path] [server listen port]" << std::endl;
		exit(EXIT_FAILURE);
	}

	// get argument
	std::string monitorPath(argv[1]);
	int listenPort = atoi(argv[2]);

	// constants
	const Protocol protocol = TCP;

	/*|=======================================================|*/
	/*|            Connect to server and monitor              |*/
	/*|=======================================================|*/
	// struct sockaddr_in serverAddress;
	// int serverSocket = getConnectSocket(serverIP, serverPort, protocol, &serverAddress);
	// // file monitoring thread
	// std::thread monitorThread(monitorFile, serverSocket, std::cref(monitorPath), refreshInterval);

	/*|=======================================================|*/
	/*|             Listen to request from client             |*/
	/*|=======================================================|*/
	#ifdef WIN32
		WSADATA wsa;
		initWinsock(&wsa);
	#endif
	struct sockaddr_in listenAddress;
	int listenSocket = getListenSocket(NULL, listenPort, protocol, &listenAddress);
	// accept loop
	acceptLoop(listenSocket, monitorPath, listenPort);

	threadPrint("server is KO ed...", "");
	return 0;
}
