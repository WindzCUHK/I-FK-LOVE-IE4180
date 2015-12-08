#include "myHead.h"

mutex g_display_mutex;

void monitorFile(int socket, const std::string &path, int refreshInterval) {

}

void httpGetHandler(int socket) {

	// thread output stream
	std::ostringstream oss;
	oss << '--- httpGetHandler thread start\n';

	// request buffer
	int bufferSize = BUFFER_SIZE;
	char buffer[BUFFER_SIZE];
	memset(buffer, '\0', bufferSize);

	// take out the whole request header
	if (!myRequestRecv(socket, buffer, bufferSize)) {
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
	if (!createAndSendResponse(socket, url, httpVersion)) {
		oss << "Error: file IO OR send()\n";
		threadPrint(oss.str().c_str(), "\n");
		mySocketClose(socket);
		return;
	}

	// close socket
	if (!mySocketClose(socket)) {
		oss << "Error: socket close()\n";
	}
	
	threadPrint(oss.str().c_str(), "--- httpGetHandler thread end\n\n");
}

void acceptLoop(int socket) {
	int clientSocket;
	struct sockaddr_in clientAddress;
	socklen_t addressSize = sizeof(clientAddress);

	threadPrint("Waiting connection... on port ", to_string(port).c_str());
	while (true) {

		// accept new socket
		memset((char *) &clientAddress, 0, addressSize);
		clientSocket = accept(socket, (struct sockaddr *) &clientAddress, &addressSize);
		if (clientSocket == -1) {
			perror("accept()");
			continue;
		}

		// new thread to handle the GET request
		std::thread httpGetHandlerThread(httpGetHandler, clientSocket);
	}

	httpGetHandlerThread.join();
}

int main(int argc, char *argv[]) {

	// check argument
	if (argc < 3) {
		cout << "Usage: monitorClient [sync folder path] [refresh interval] [server IP] [server port] [client listen port]" << endl;
		exit(EXIT_FAILURE);
	}

	// get argument
	std::string monitorPath(argv[1]);
	int refreshInterval = atoi(argv[2]);
	char *serverIP = "127.0.0.1";
	int serverPort = 4180;
	int listenPort = 4181;

	// constants
	const Protocol protocol = TCP;

	/*|=======================================================|*/
	/*|            Connect to server and monitor              |*/
	/*|=======================================================|*/
	struct sockaddr_in serverAddress;
	int serverSocket = getConnectSocket(serverIP, serverPort, protocol, &serverAddress);
	// file monitoring thread
	std::thread monitorThread(monitorFile, serverSocket, std::cref(monitorPath), refreshInterval);

	/*|=======================================================|*/
	/*|          Listen to GET request from server            |*/
	/*|=======================================================|*/
	#ifdef WIN32
		WSADATA wsa;
		initWinsock(&wsa);
	#endif
	struct sockaddr_in listenAddress;
	int listenSocket = getListenSocket(NULL, listenPort, protocol, &listenAddress);
	// accept loop
	acceptLoop(listenSocket);

	threadPrint("Client is KO ed...", "");
	return 0;
}
