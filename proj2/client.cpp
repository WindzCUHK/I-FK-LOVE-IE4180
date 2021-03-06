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

void displayStatistics(Statistics *stat, ResponseStat *rStat, Mode mode, int refresh_interval, unsigned int packageSize) {

	chrono::milliseconds refresh_interval_as_ms = chrono::milliseconds(refresh_interval);

	while (true) {

		// sleep
		std::this_thread::sleep_for(refresh_interval_as_ms);
		
		// lock
		// cout << "lock 1" << endl;
		statistics_display_m.lock();

		if (stat->isEnded == true) {
			statistics_display_m.unlock();
			return;
		}
		if (stat->isSessionStarted == false) {
			puts("Nothing to play now. Waiting something fun...");
			statistics_display_m.unlock();
			continue;
		}

		printStat(stat, rStat, mode, packageSize);

		statistics_display_m.unlock();
	}
}

int main(int argc, char *argv[]) {

	getArguments(argc, argv);

#ifdef WIN32
	WSADATA wsa;
	initWinsock(&wsa);
#endif

	// handle -host mode
	if (mode == HOST) {
		return 0;
	}
	
	// connect server by TCP first
	struct sockaddr_in serverAddress;
	int serverSocket = getConnectSocket(rhostname, rPort, TCP, &serverAddress);

	// create and send hello
	HelloPackage hello;
	initHello(&hello);
	printHello(&hello);
	mySend(false, serverSocket, (struct sockaddr *) &serverAddress, (char *) &hello, sizeof(hello));

	// if UDP, replace the tcp socket
	if (protocol == UDP) {
		// close TCP
#ifdef WIN32
		if (closesocket(serverSocket) == -1) {
#else
		if (close(serverSocket) == -1) {
#endif
			perror("TCP socket close, exiting...");
			exit(1);
		}
		// open UDP socket
		// client recv on RECV, send on SEND & RESPONSE
		if (mode == RECV) {
			serverSocket = getListenSocket(NULL, lPort, UDP, &serverAddress);
			if (serverSocket == -1) {
				cout << "UDP already bind()" << endl;
				cout << "exiting client..." << endl;
				exit(1);
			}
		} else if (mode == SEND) {
			serverSocket = getConnectSocket(rhostname, rPort, UDP, &serverAddress);
		} else if (mode == RESPONSE) {
			struct sockaddr_in responseAddress;
			serverSocket = getListenSocket(NULL, lPort, UDP, &responseAddress);
			if (serverSocket == -1) {
				cout << "UDP already bind()" << endl;
				cout << "exiting client..." << endl;
				exit(1);
			}
			// the server address struct has no changes
		}
	}

	// open display thread
	Statistics stat;
	initStat(&stat);
	ResponseStat rStat;
	initResponseStat(&rStat);
	thread th(displayStatistics, &stat, &rStat, mode, displayInterval, (unsigned int) packageSize);

	switch (mode) {
		case SEND:
			mySendLoop(true, &stat, serverSocket, (struct sockaddr *) &serverAddress, (protocol == UDP), packageSize, txRate, (unsigned int) packageNummber);
			break;
		case RECV:
			myRecvLoop(true, &stat, serverSocket, (struct sockaddr *) &serverAddress, (protocol == UDP), packageSize, (unsigned int) packageNummber);
			break;
		case RESPONSE:
			myClientRR(&stat, &rStat, serverSocket, (struct sockaddr *) &serverAddress, (protocol == UDP), packageSize, (unsigned int) packageNummber);
			break;
		default:
			puts("You are in on99 mode...");
			exit(1);
	}

	// signal and wait for thread terminate
	
	// cout << "lock 2" << endl;
	statistics_display_m.lock();
	stat.isEnded = true;
	statistics_display_m.unlock();
	th.join();

	cout << "Client done! Tutor, I love u~~" << endl;
#ifdef WIN32
	if (closesocket(serverSocket) == -1) {
#else
	if (close(serverSocket) == -1) {
#endif
		perror("close(), exiting...");
		exit(1);
	}
}
