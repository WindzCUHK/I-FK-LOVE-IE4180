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
// mutex g_display_mutex;
mutex statistics_display_m;

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

void clientHandler(int socket, struct sockaddr_in clientAddress) {

	char clientIP[INET_ADDRSTRLEN];
	int clientPort = ntohs(clientAddress.sin_port);
	if (inet_ntop(clientAddress.sin_family, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN) == NULL) {
		perror("Parse client IP failed with error code");
	}

	// server debug print
	// g_display_mutex.lock();
	cout << "--- thread start " << this_thread::get_id() << endl;
	cout << "client IP: " << clientIP << endl;
	cout << "client port: " << clientPort << endl;
	// g_display_mutex.unlock();

	// wait for hello package
	HelloPackage hello;
	if (myRecv(false, socket, NULL, (char *) &hello, sizeof(hello)) <= 0) {
		cout << "[" << this_thread::get_id() << "] " << "Hello is gg" << endl;
		cout << "--- thread end " << this_thread::get_id() << endl;
		return;
	}

	// server debug print
	printHello(&hello);

	// parse client parameters
	Mode mode;
	Protocol protocol;
	parseHello(&hello, &mode, &protocol);
	
	// if UDP, replace the tcp socket
	if (protocol == UDP) {
#ifdef WIN32
		if (closesocket(socket) == -1) {
#else
		if (close(socket) == -1) {
#endif
			perror("TCP socket close, exiting...");
			exit(1);
		}
		// open UDP socket
		// server recv on RECV & RESPONSE, send on SEND
		if (mode == RECV) {
			socket = getListenSocket(NULL, lPort, UDP, &clientAddress);
			if (socket == -1) {
				cout << "[" << this_thread::get_id() << "] " << "UDP already bind()" << endl;
				cout << "--- thread end " << this_thread::get_id() << endl;
				return;
			}
		} else if (mode == SEND) {
			socket = getConnectSocket(rhostname, hello.clientUdpListenPort, UDP, &clientAddress);
		} else if (mode == RESPONSE) {

			struct sockaddr_in responseAddress;
			socket = getListenSocket(NULL, lPort, UDP, &responseAddress);
			if (socket == -1) {
				cout << "[" << this_thread::get_id() << "] " << "UDP already bind()" << endl;
				cout << "--- thread end " << this_thread::get_id() << endl;
				return;
			}

			int responseSocket = getConnectSocket(rhostname, hello.clientUdpListenPort, UDP, &clientAddress);
#ifdef WIN32
			if (closesocket(responseSocket) == -1) {
#else
			if (close(responseSocket) == -1) {
#endif
				perror("close(), exiting...");
				exit(1);
			}

			clientPort = ntohs(clientAddress.sin_port);
			if (inet_ntop(clientAddress.sin_family, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN) == NULL) {
				perror("Parse client IP failed with error code");
			}
			cout << "rr client IP: " << clientIP << endl;
			cout << "rr client port: " << clientPort << endl;
		}
	}

	// set buffer size
	if (hello.bufferSize != 0) {
		if (mode == SEND) {
			// cout << "SEND buffer is set" << endl;
			if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char *) &(hello.bufferSize), sizeof(unsigned int)) == -1) {
				perror("setsockopt(SO_SNDBUF)");
				exit(1);
			}
		} else {
			// cout << "RECV buffer is set" << endl;
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
			cout << "[" << this_thread::get_id() << "] " << "server in RECV mode..." << endl;
			myRecvLoop(false, &stat, socket, (struct sockaddr *) &clientAddress, (protocol == UDP), hello.packageSize, hello.packageNummber);
			break;
		case SEND:
			cout << "[" << this_thread::get_id() << "] " << "server in SEND mode..." << endl;
			mySendLoop(false, &stat, socket, (struct sockaddr *) &clientAddress, (protocol == UDP), hello.packageSize, hello.txRate, hello.packageNummber);
			break;
		case RESPONSE:
			cout << "[" << this_thread::get_id() << "] " << "server in RESPONSE mode..." << endl;
			myServerRR(socket, (struct sockaddr *) &clientAddress, (protocol == UDP), hello.packageSize);
			break;
		default:
			perror("Unknown mode");
			exit(1);
	}

	// print overall statistics
	if (mode != RESPONSE) printStat(&stat, NULL, mode, hello.packageSize);

	// close socket
#ifdef WIN32
	if (closesocket(socket) == -1) {
#else
	if (close(socket) == -1) {
#endif
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
		cout << "Got new client XD" << endl;

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

#ifdef WIN32
	WSADATA wsa;
	initWinsock(&wsa);
#endif
	
	struct sockaddr_in listenAddress;
	int listenSocket = getListenSocket(NULL, lPort, TCP, &listenAddress);
	
	cout << "Waiting connection..." << endl;
	myAccept(listenSocket);
	cout << "KO" << endl;
}
