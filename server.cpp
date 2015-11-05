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

// mutex
mutex g_display_mutex;

mutex m;
unique_lock<mutex> statistics_display_lock(m, defer_lock);

void clientHandler(int socket, struct sockaddr_in clientAddress) {

	char clientIP[INET_ADDRSTRLEN];
	int clientPort = clientAddress.sin_port;
	if (inet_ntop(clientAddress.sin_family, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN) == NULL) {
		perror("Parse client IP failed with error code");
	}

	// server debug print
	g_display_mutex.lock();
	cout << "--- thread start " << this_thread::get_id() << endl;
	cout << "client IP: " << clientIP << endl;
	cout << "client port: " << clientPort << endl;
	g_display_mutex.unlock();

	// wait for hello package
	HelloPackage hello;
	myRecv(false, socket, NULL, (char *) &hello, sizeof(hello));

	// server debug print
	printHello(&hello);

	// parse client parameters
	Mode mode;
	Protocol protocol;
	parseHello(&hello, &mode, &protocol);

	// if UDP, replace the tcp socket
	if (protocol == UDP) {
		if (close(socket) == -1) {
			perror("TCP socket close, exiting...");
			exit(1);
		}
		// open UDP socket
		if (mode == RECV) {
			socket = getListenSocket(NULL, lPort, UDP, &clientAddress);
		} else {
			socket = getConnectSocket(rhostname, hello.clientUdpListenPort, UDP, &clientAddress);
		}
		// set buffer size
	}

	// react to client parameters
	Statistics stat;
	switch (mode) {
		case RECV:
			cout << "server in RECV mode" << endl;
			myRecvLoop(false, &stat, socket, (struct sockaddr *) &clientAddress, (protocol == UDP), hello.packageSize);
			break;
		case SEND:
			cout << "server in SEND mode" << endl;
			mySendLoop(false, &stat, socket, (struct sockaddr *) &clientAddress, (protocol == UDP), hello.packageSize, hello.txRate, hello.packageNummber);
			break;
		case RESPONSE:
			break;
		default:
			perror("Unknown mode");
			exit(1);
	}

	// print overall statistics
	printStat(&stat, mode, hello.packageSize);

	// close socket
	if (close(socket) == -1) {
		perror("close(), exiting...");
		exit(1);
	}

	// server debug print
	cout << "--- thread end " << this_thread::get_id() << endl;
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
		thread t1(clientHandler, clientSocket, clientAddress);
		t1.detach();
	}
}

int main(int argc, char *argv[]) {

	getArguments(argc, argv);
	
	struct sockaddr_in listenAddress;
	int listenSocket = getListenSocket(NULL, lPort, TCP, &listenAddress);
	
	cout << "Waiting connection..." << endl;
	myAccept(listenSocket);
	cout << "KO" << endl;
}
