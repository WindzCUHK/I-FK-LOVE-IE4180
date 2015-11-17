#include "../myHead.h"

using namespace std;

int myRecv(bool isUDP, int socket, struct sockaddr *addr, char *package, int packageSize) {
	
	int result;
	int recv_size = 0;

	if (isUDP) {
		while (recv_size < packageSize) {
			result = recvfrom(socket, package + recv_size, packageSize - recv_size, 0, NULL, NULL);
			if (result == 0 || result == -1) return result;
			recv_size += result;
		}
	} else {
		while (recv_size < packageSize) {
			result = recv(socket, package + recv_size, packageSize - recv_size, 0);
			if (result == 0 || result == -1) return result;
			recv_size += result;
		}
	}

	if (recv_size != packageSize) {
		perror("Recv failed, exiting...");
		exit(1);
	}

	// cout << "[" << this_thread::get_id() << "] " << "recv()" << endl;
	// printBuffer(package, packageSize);
	return recv_size;
}

void myRecvLoop(bool isClient, Statistics *stat, int socket, struct sockaddr *addr, bool isUDP, int packageSize, unsigned int maxSequence) {

	char *package = createPackageBuffer(packageSize);
	int gotPackageBytes;
	unsigned long long gotBytes = 0;
	unsigned int previousSequence = 0, currentSequence = 0, lostCount = 0;

	// get first package
	gotBytes += myRecv(isUDP, socket, addr, package, packageSize);
	currentSequence = getSequence(package);
	// cout << "currentSequence: " << currentSequence << endl;
	// printBuffer(package, packageSize);
	// cout << "myRecvLoop 1" << endl;

	// init stat
	chrono::system_clock::time_point startTime = chrono::system_clock::now();
	if (isClient) {
		// cout << "lock 5" << endl;
		statistics_display_m.lock();
	}
	stat->isSessionStarted = true;
	stat->startTime = startTime;
	if (isClient) statistics_display_m.unlock();
	// cout << "myRecvLoop 2" << endl;

	// loop
	chrono::system_clock::duration timeDiff;
	unsigned long long previousTimeDiffInLong = 0, timeDiffInLong;
	double packageCount, avgTime, jitter = 0.0;
	do {
		// cout << "myRecvLoop 3" << endl;
		// skip out-of-sequence package
		if (currentSequence > previousSequence) {

			// check and update lost
			previousSequence++;
			if (currentSequence != previousSequence) {
				lostCount += currentSequence - previousSequence;
				previousSequence = currentSequence;
			}
			// cout << "myRecvLoop 4" << endl;

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
			// cout << "myRecvLoop 5" << endl;

			// update stat
			if (isClient) {
				// cout << "lock 6" << endl;
				statistics_display_m.lock();
			}
			stat->byteOnTraffic = gotBytes;
			stat->currentSequence = currentSequence;
			stat->lostCount = lostCount;
			stat->jitter = jitter;
			// debugStat(stat);
			if (isClient) statistics_display_m.unlock();
			// cout << "myRecvLoop 6" << endl;
			
			// session end for UDP
			if (maxSequence != 0 && currentSequence == maxSequence) {
				freePackageBuffer(package);
				return;
			}
		}

		// wait for next package
		gotPackageBytes = myRecv(isUDP, socket, addr, package, packageSize);
		if (gotPackageBytes <= 0) {
			// connection closed
			freePackageBuffer(package);
			return;
		} else {
			gotBytes += gotPackageBytes;
			currentSequence = getSequence(package);
			// cout << "currentSequence: " << currentSequence << endl;
			// printBuffer(package, packageSize);
		}
		// cout << "myRecvLoop 7" << endl;
	} while (true);
}
