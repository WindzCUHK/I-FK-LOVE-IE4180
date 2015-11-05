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

mutex m;
unique_lock<mutex> statistics_display_lock(m, defer_lock);

void displayStatistics(Statistics *stat, Mode mode, int refresh_interval, unsigned int packageSize) {

	chrono::milliseconds refresh_interval_as_ms = chrono::milliseconds(refresh_interval);

	while (true) {

		// sleep
		std::this_thread::sleep_for(refresh_interval_as_ms);
		
		// lock
		statistics_display_lock.lock();

		if (stat->isEnded == true) {
			statistics_display_lock.unlock();
			return;
		}
		if (stat->isSessionStarted == false) {
			puts("Nothing to play now. Waiting something fun...");
			statistics_display_lock.unlock();
			continue;
		}

		printStat(stat, mode, packageSize);

		statistics_display_lock.unlock();
	}
}

int main(int argc, char *argv[]) {

	getArguments(argc, argv);

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
		if (close(serverSocket) == -1) {
			perror("TCP socket close, exiting...");
			exit(1);
		}
		// open UDP socket
		if (mode == RECV) {
			serverSocket = getListenSocket(NULL, lPort, UDP, &serverAddress);
		} else {
			serverSocket = getConnectSocket(rhostname, rPort, UDP, &serverAddress);
		}
	}

	// open display thread
	Statistics stat;
	thread th(displayStatistics, &stat, mode, displayInterval, (unsigned int) packageSize);

	switch (mode) {
		case SEND:
			mySendLoop(true, &stat, serverSocket, (struct sockaddr *) &serverAddress, (protocol == UDP), packageSize, txRate, (unsigned int) packageNummber);
			break;
		case RECV:
			myRecvLoop(true, &stat, serverSocket, (struct sockaddr *) &serverAddress, (protocol == UDP), packageSize);
			break;
		case RESPONSE:
			// myRR();
			break;
		default:
			puts("You are in on99 mode...");
			exit(1);
	}

	// signal and wait for thread terminate
	statistics_display_lock.lock();
	stat.isEnded = true;
	statistics_display_lock.unlock();
	th.join();

	cout << "Client done! Tutor, I love u~~" << endl;
	if (close(serverSocket) == -1) {
		perror("close(), exiting...");
		exit(1);
	}
}
