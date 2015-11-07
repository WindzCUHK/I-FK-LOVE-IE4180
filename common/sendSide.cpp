#include "../myHead.h"

using namespace std;

int mySend(bool isUDP, int socket, struct sockaddr *addr, char *package, int packageSize) {

	int result;
	int send_size = 0;

	if (isUDP) {
		while (send_size < packageSize) {
			result = sendto(socket, package + send_size, packageSize - send_size, MSG_NOSIGNAL, addr, sizeof(*addr));
			if (result == 0 || result == -1) return result;
			send_size += result;
		}
	} else {
		while (send_size < packageSize) {
			result = send(socket, package + send_size, packageSize - send_size, MSG_NOSIGNAL);
			if (result == 0 || result == -1) return result;
			send_size += result;
		}
	}

	// if (send_size == -1 || send_size != packageSize) {
	if (send_size != packageSize) {
		perror("Send failed, exiting...");
		exit(1);
	}

	// cout << "[" << this_thread::get_id() << "] " << "send()" << endl;
	// printBuffer(package, packageSize);
	return send_size;
}

void mySendLoop(bool isClient, Statistics *stat, int socket, struct sockaddr *addr, bool isUDP, int packageSize, int rate, unsigned int maxSequence) {

	// printAddress(addr);

	char *package = createPackageBuffer(packageSize);
	int sentPackageBytes;
	unsigned long long sentBytes = 0;
	unsigned int currentSequence = 0;
	chrono::system_clock::time_point startTime = chrono::system_clock::now();

	// convert target rate to byte per ms
	double targetRate = ((double) rate) / 1000.0;

	// init stat
	if (isClient) {
		// cout << "lock 3" << endl;
		statistics_display_m.lock();
	}
	stat->isSessionStarted = true;
	stat->startTime = startTime;
	if (isClient) statistics_display_m.unlock();

	// loop
	chrono::system_clock::duration timeDiff;
	unsigned long long timeDiffInLong;
	while (maxSequence == 0 || currentSequence < maxSequence) {

		// compute time difference
		timeDiff = chrono::system_clock::now() - startTime;
		timeDiffInLong = (chrono::duration_cast<std::chrono::milliseconds> (timeDiff)).count();

		// compare rate
		if (rate == 0 || (timeDiffInLong > 0 && targetRate > (((double) sentBytes) / ((double) timeDiffInLong)))) {

			// sent package
			setAndIncreaseSequence(package, &currentSequence);
			sentPackageBytes = mySend(isUDP, socket, addr, package, packageSize);
			if (sentPackageBytes != 0 && sentPackageBytes != -1) {
				sentBytes += sentPackageBytes;
			} else {
				cout << "Cannot send..." << endl;
				freePackageBuffer(package);
				return;
			}

			// update statistics
			if (isClient) {
				// cout << "lock 4" << endl;
				statistics_display_m.lock();
			}
			stat->byteOnTraffic = sentBytes;
			stat->currentSequence = currentSequence;
			debugStat(stat);
			if (isClient) statistics_display_m.unlock();

		} else {
			// sleep ?
			std::this_thread::sleep_for(chrono::milliseconds((long)(((double)sentBytes) / targetRate - timeDiffInLong)));
		}
	}

	// clean up
	freePackageBuffer(package);
}
