#include "myHead.h"


#ifdef WIN32
void initWinsock(WSADATA *ptr_wsa) {
	std::cout << "\nInitialising Winsock...\n";
	if (WSAStartup(MAKEWORD(2, 2), ptr_wsa) != 0) {
		printf("Init Winsock failed. Error Code : %d", WSAGetLastError());
		exit(1);
	}
	std::cout << "Winsock Initialised.\n";
}
#endif


void myDied(char *str) {
	perror(str);
	exit(EXIT_FAILURE);
}

int getConnectSocket(char *host, int port, Protocol protocol, struct sockaddr_in *serverAddress) {

	int connectSocket;
	memset((char *) serverAddress, 0, sizeof(*serverAddress));

	// convert string IP to long
	if (inet_pton(AF_INET, host, &(serverAddress->sin_addr)) != 1) myDied("inet_pton()");

	// address info
	serverAddress->sin_family = AF_INET;
	serverAddress->sin_port = htons(port);

	// create socket
	int type = ((protocol == UDP) ? SOCK_DGRAM : SOCK_STREAM);
	connectSocket = socket(AF_INET, type, 0);
	if (connectSocket == -1) myDied("socket()");

	/*
	// set recv and send buffer size
	if (mode != SEND && rBufferSize > 0) {
		// std::cout << "RECV buffer is set" << endl;
		if (setsockopt(connectSocket, SOL_SOCKET, SO_RCVBUF, (const char *) &rBufferSize, sizeof(int)) == -1) myDied("setsockopt(SO_RCVBUF)");
	}
	if (mode != RECV && sBufferSize > 0) {
		// std::cout << "SEND buffer is set" << endl;
		if (setsockopt(connectSocket, SOL_SOCKET, SO_SNDBUF, (const char *) &sBufferSize, sizeof(int)) == -1) myDied("setsockopt(SO_SNDBUF)");
	}
	*/

	// end for UDP
	if (protocol == UDP) return connectSocket;

	// connect
	if (connect(connectSocket, (struct sockaddr *) serverAddress , sizeof(*serverAddress)) == -1) myDied("connect()");

	return connectSocket;
}

int getListenSocket(char *host, int port, Protocol protocol, struct sockaddr_in *listenAddress) {

	int listenSocket;

	// address info
	memset((char *) listenAddress, 0, sizeof(*listenAddress));
	listenAddress->sin_family = AF_INET;
	listenAddress->sin_port = htons(port);

	// convert string IP to long
	if (host != NULL) {
		if (inet_pton(AF_INET, host, &(listenAddress->sin_addr)) != 1) myDied("inet_pton()");
	} else {
		listenAddress->sin_addr.s_addr = htonl(INADDR_ANY);
	}

	// create socket
	int type = (protocol == UDP) ? SOCK_DGRAM : SOCK_STREAM;
	listenSocket = socket(AF_INET, type, 0);
	if (listenSocket == -1) myDied("socket()");

	/*
	// set recv and send buffer size
	if (mode != SEND && rBufferSize > 0) {
		// std::cout << "RECV buffer is set" << endl;
		if (setsockopt(listenSocket, SOL_SOCKET, SO_RCVBUF, (const char *) &rBufferSize, sizeof(int)) == -1) myDied("setsockopt(SO_RCVBUF)");
	}
	if (mode != RECV && sBufferSize > 0) {
		// std::cout << "SEND buffer is set" << endl;
		if (setsockopt(listenSocket, SOL_SOCKET, SO_SNDBUF, (const char *) &sBufferSize, sizeof(int)) == -1) myDied("setsockopt(SO_SNDBUF)");
	}
	*/

	// bind
	if (bind(listenSocket, (struct sockaddr *) listenAddress, sizeof(*listenAddress)) == -1) {
		if (protocol != UDP) myDied("bind()");
		else {
			// there is other thread to handle it
			return -1;
		}
	}

	// end for UDP
	if (protocol == UDP) return listenSocket;

	// start listen
	if (listen(listenSocket, SOMAXCONN) == -1) myDied("listen()");

	return listenSocket;
}

bool mySocketClose(int socket) {
	#ifdef WIN32
		if (closesocket(socket) == -1) {
	#else
		if (close(socket) == -1) {
	#endif
		perror("close(), exiting...");
		return false;
	}

	return true;
}
