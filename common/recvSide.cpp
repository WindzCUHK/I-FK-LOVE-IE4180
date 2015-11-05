#include "../myHead.h"

using namespace std;

int myRecv(bool isUDP, int socket, struct sockaddr *addr, char *package, int packageSize) {

	int recv_size = 0;

	if (isUDP) {
		while (recv_size != packageSize) {
			recv_size = recvfrom(socket, package + recv_size, packageSize - recv_size, 0, NULL, NULL);
			if (recv_size == 0) return 0;
		}
	} else {
		while (recv_size != packageSize) {
			recv_size = recv(socket, package + recv_size, packageSize - recv_size, 0);
			if (recv_size == 0) return 0;
		}
	}

	if (recv_size == -1 || (recv_size != 0 && recv_size != packageSize)) {
		perror("Recv failed, exiting...");
		exit(1);
	}

	return recv_size;
}

void myRecvLoop(bool isClient, Statistics *stat, int socket, struct sockaddr *addr, bool isUDP, int packageSize) {

	char *package = createPackageBuffer(packageSize);
	int gotPackageBytes;
	unsigned long long gotBytes = 0;
	unsigned int previousSequence = 0, currentSequence = 0, lostCount = 0;

	// get first package
	gotBytes += myRecv(isUDP, socket, addr, package, packageSize);
	currentSequence = getSequence(package);
	cout << "currentSequence: " << currentSequence << endl;
	printBuffer(package, packageSize);

	// init stat
	chrono::system_clock::time_point startTime = chrono::system_clock::now();
	if (isClient) statistics_display_lock.lock();
	stat->isSessionStarted = true;
	stat->startTime = startTime;
	if (isClient) statistics_display_lock.unlock();

	// loop
	chrono::system_clock::duration timeDiff;
	unsigned long long previousTimeDiffInLong = 0, timeDiffInLong;
	double packageCount, avgTime, jitter = 0.0;
	do {
		// skip out-of-sequence package
		if (currentSequence > previousSequence) {

			// check and update lost
			previousSequence++;
			if (currentSequence != previousSequence) {
				lostCount += currentSequence - previousSequence;
				previousSequence = currentSequence;
			}

			// compute jitter
			// 1. compute time difference
			timeDiff = chrono::system_clock::now() - startTime;
			timeDiffInLong = (chrono::duration_cast<std::chrono::milliseconds> (timeDiff)).count();
			// 2. compute total package & avg. time
			packageCount = (double)(currentSequence - lostCount);
			avgTime = ((double)timeDiffInLong) / packageCount;
			// 3. iterate
			jitter = ((jitter * (packageCount - 1.0)) + (((double)timeDiffInLong) - ((double)previousTimeDiffInLong) - avgTime)) / packageCount;
			previousTimeDiffInLong = timeDiffInLong;

			// update stat
			if (isClient) statistics_display_lock.lock();
			stat->byteOnTraffic = gotBytes;
			stat->currentSequence = currentSequence;
			stat->lostCount = lostCount;
			stat->jitter = jitter;
			if (isClient) statistics_display_lock.unlock();

			// wait for next package
			gotPackageBytes = myRecv(isUDP, socket, addr, package, packageSize);
			if (gotPackageBytes == 0) {
				// connection closed
				return;
			} else {
				gotBytes += gotPackageBytes;
				currentSequence = getSequence(package);
				cout << "currentSequence: " << currentSequence << endl;
				printBuffer(package, packageSize);
			}
		}
	} while (true);
}
