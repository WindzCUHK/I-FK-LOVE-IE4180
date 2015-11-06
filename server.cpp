#include "myHead.h"

using namespace std;

Mode mode;
Protocol protocol = UDP;
int displayInterval = 500;
int packageSize = 1000;
char* rhostname = "127.0.0.1";
int rPort = 4180;
int sBufferSize = 0;
int txRate = 1000;
int packageNummber = 0;
char* lhostname = NULL;
int lPort = 4180;
int rBufferSize = 0;

// mutex
mutex g_display_mutex;

mutex statistics_display_m;

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
		// server recv on RECV & RESPONSE, send on SEND
		if (mode != SEND) {
			socket = getListenSocket(NULL, lPort, UDP, &clientAddress);
			if (socket == -1) {
				cout << "UDP already bind()" << endl;
				cout << "--- thread end " << this_thread::get_id() << endl;
				return;
			}
		} else {
			socket = getConnectSocket(rhostname, hello.clientUdpListenPort, UDP, &clientAddress);
		}
	}

	// set buffer size
	if (hello.bufferSize != 0) {
		if (mode == SEND) {
			cout << "SEND buffer is set" << endl;
			if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char *) &(hello.bufferSize), sizeof(unsigned int)) == -1) {
				perror("setsockopt(SO_SNDBUF)");
				exit(1);
			}
		} else {
			cout << "RECV buffer is set" << endl;
			if (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (const char *) &(hello.bufferSize), sizeof(unsigned int)) == -1) {
				perror("setsockopt(SO_RCVBUF)");
				exit(1);
			}
		}
	}


	// react to client parameters
	Statistics stat;
	initStat(&stat);
	switch (mode) {
		case RECV:
			cout << "server in RECV mode" << endl;
			myRecvLoop(false, &stat, socket, (struct sockaddr *) &clientAddress, (protocol == UDP), hello.packageSize, hello.packageNummber);
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

	#ifdef WIN32
		puts("hi9 window here");
	#else
		puts("hello linux");
	#endif

	getArguments(argc, argv);
	
	struct sockaddr_in listenAddress;
	int listenSocket = getListenSocket(NULL, lPort, TCP, &listenAddress);
	
	cout << "Waiting connection..." << endl;
	myAccept(listenSocket);
	cout << "KO" << endl;
}
