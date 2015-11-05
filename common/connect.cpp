#include "../myHead.h"

using namespace std;


void myDied(char *str) {
	perror(str);
	exit(1);
}

int getConnectSocket(char *host, int port, Protocol protocol, struct sockaddr_in *serverAddress) {

	int connectSocket;
	memset((char *) serverAddress, 0, sizeof(*serverAddress));

	// handle host for address
	if (inet_pton(AF_INET, host, &(serverAddress->sin_addr)) != 1) myDied("inet_pton()");

	// address info
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
		if (inet_pton(AF_INET, host, &(listenAddress->sin_addr)) != 1) myDied("inet_pton()");
	}

	// create socket
	int type = (protocol == UDP) ? SOCK_DGRAM : SOCK_STREAM;
	listenSocket = socket(AF_INET, type, 0);
	if (listenSocket == -1) myDied("socket()");

	// set recv and send buffer size
	if (mode == RECV && rBufferSize > 0) {
		if (setsockopt(listenSocket, SOL_SOCKET, SO_RCVBUF, (const char *) &rBufferSize, sizeof(int)) == -1) myDied("setsockopt(SO_RCVBUF)");
	}
	if (mode == SEND && sBufferSize > 0) {
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
