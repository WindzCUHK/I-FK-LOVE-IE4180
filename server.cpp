#include "myHead.h"

using namespace std;

Mode mode;
Protocol protocol;
int displayInterval;
int packageSize;
char* rhostname;
int rPort;
int sBufferSize;
int txRate;
int packageNummber;
char* lhostname;
int lPort;
int rBufferSize;



void printClientInfo(struct sockaddr_in clientAddress) {
	cout << "thread start " << endl;
	cout << clientAddress.sin_port << endl;
	cout << "thread end " << endl;
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
		cout << "Got new client" << endl;

		// start new thread
		thread t1(printClientInfo, clientAddress);
		t1.join();
	}
}

int main(int argc, char *argv[]) {

	getArguments(argc, argv);
	
	struct sockaddr_in listenAddress;
	int listenSocket = getListenSocket(NULL, lPort, TCP, &listenAddress);
	
	cout << "connect" << endl;
	myAccept(listenSocket);
	cout << "KO" << endl;

	/*
	if (sendto(s, my_message, strlen(my_message), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("sendto failed");
		return 0;
	}

	struct sockaddr_in senderAddress;
	socklen_t addrlen = sizeof(senderAddress);
	recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&senderAddress, &addrlen);
	if (recvlen > 0) {
	    buf[recvlen] = 0;
	    printf("received message: \"%s\"\n", buf);
	}
	*/
}
