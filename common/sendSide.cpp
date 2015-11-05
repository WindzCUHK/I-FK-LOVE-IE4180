#include "../myHead.h"

using namespace std;

int mySend(bool isUDP, int socket, struct sockaddr *addr, char *package, int packageSize) {

	int send_size = 0;

	if (isUDP) {
		while (send_size != packageSize) {
			send_size += sendto(socket, package + send_size, packageSize - send_size, 0, addr, sizeof(*addr));
			if (send_size = 0) return 0;
		}
	} else {
		while (send_size != packageSize) {
			send_size += send(socket, package + send_size, packageSize - send_size, 0);
			if (send_size = 0) return 0;
		}
	}

	if (send_size == -1 || send_size != packageSize) {
		perror("Send failed, exiting...");
		exit(1);
	}

	return send_size;
}

void mySendLoop(bool isClient, Statistics *stat, int socket, struct sockaddr *addr, bool isUDP, int packageSize, int rate, unsigned int maxSequence) {

	printAddress(addr);

	char *package = createPackageBuffer(packageSize);
	unsigned long long sentBytes = 0;
	unsigned int currentSequence = 0;
	chrono::system_clock::time_point startTime = chrono::system_clock::now();

	// convert target rate to byte per ms
	double targetRate = ((double) rate) / 1000.0;

	// init stat
	if (isClient) statistics_display_lock.lock();
	stat->isSessionStarted = true;
	stat->startTime = startTime;
	if (isClient) statistics_display_lock.unlock();

	// loop
	chrono::system_clock::duration timeDiff;
	unsigned long long timeDiffInLong;
	while (currentSequence < maxSequence) {

		// compute time difference
		timeDiff = chrono::system_clock::now() - startTime;
		timeDiffInLong = (chrono::duration_cast<std::chrono::milliseconds> (timeDiff)).count();

		// compare rate
		if (timeDiffInLong > 0 && targetRate > (((double) sentBytes) / ((double) timeDiffInLong))) {

			// sent package
			setAndIncreaseSequence(package, &currentSequence);
			cout << "before sent: " << sentBytes << endl;
			sentBytes += mySend(isUDP, socket, addr, package, packageSize);
			cout << "after sent: " << sentBytes << endl;

			// update statistics
			if (isClient) statistics_display_lock.lock();
			stat->byteOnTraffic = sentBytes;
			stat->currentSequence = currentSequence;
			if (isClient) statistics_display_lock.unlock();

		} else {
			// sleep ?
			std::this_thread::sleep_for(chrono::milliseconds((long)(((double)sentBytes) / targetRate - timeDiffInLong)));
		}
	}

	// clean up
	freePackageBuffer(package);
}
