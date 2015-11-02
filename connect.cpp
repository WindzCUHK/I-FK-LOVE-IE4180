#include "myHead.h"

using namespace std;


void myDied(char *str) {
	perror(str);
	exit(1);
}

int getConnectSocket(char *host, int port, Protocol protocol, struct sockaddr_in *serverAddress) {

	int connectSocket;

	// handle host for address
	inet_aton(host, &(serverAddress->sin_addr));
	serverAddress->sin_addr.s_addr = htonl(serverAddress->sin_addr.s_addr);

	// address info
	memset((char *) serverAddress, 0, sizeof(*serverAddress));
	serverAddress->sin_family = AF_INET;
	serverAddress->sin_port = htons(port);

	// create socket
	int type = ((protocol == UDP) ? SOCK_DGRAM : SOCK_STREAM);
	connectSocket = socket(AF_INET, type, 0);
	if (connectSocket == -1) myDied("socket()");

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

	// handle host for address
	if (host != NULL) {
		inet_aton(host, &(listenAddress->sin_addr));
		listenAddress->sin_addr.s_addr = htonl(listenAddress->sin_addr.s_addr);
	} else {
		listenAddress->sin_addr.s_addr = htonl(INADDR_ANY);
	}

	// create socket
	int type = (protocol == UDP) ? SOCK_DGRAM : SOCK_STREAM;
	listenSocket = socket(AF_INET, type, 0);
	if (listenSocket == -1) myDied("socket()");

	// set recv and send buffer size
	if (rBufferSize > 0) {
		if (setsockopt(listenSocket, SOL_SOCKET, SO_RCVBUF, (const char *) &rBufferSize, sizeof(int)) == -1) myDied("setsockopt(SO_RCVBUF)");
	}
	if (sBufferSize > 0) {
		if (setsockopt(listenSocket, SOL_SOCKET, SO_SNDBUF, (const char *) &sBufferSize, sizeof(int)) == -1) myDied("setsockopt(SO_SNDBUF)");
	}

	// bind
	if (bind(listenSocket, (struct sockaddr *) listenAddress, sizeof(*listenAddress)) == -1) myDied("bind()");

	// end for UDP
	if (protocol == UDP) return listenSocket;

	// start listen
	if (listen(listenSocket, SOMAXCONN) == -1) myDied("listen()");

	return listenSocket;
}

void myAccept(int listenSocket) {

	int clientSocket;
	struct sockaddr_in clientAddress;
	socklen_t addressSize = sizeof(clientAddress);

	while (true) {

		// accept new socket
		memset((char *) &clientAddress, 0, sizeof(clientAddress));
		clientSocket = accept(listenSocket, (struct sockaddr *) &clientAddress, &addressSize);
		if (clientSocket == -1) {
			perror("accept()");
			continue;
		}

		// start new thread
	}
}
