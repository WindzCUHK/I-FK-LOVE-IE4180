#include "../myHead.h"

using namespace std;

void updateResponseStat(ResponseStat *rStat, unsigned int responseSequenceNumber, unsigned long long *previousTimeDiffInLong, chrono::system_clock::time_point startTime) {

	chrono::system_clock::time_point now = chrono::system_clock::now();
	chrono::system_clock::time_point sendTime = rStat->timeStore[responseSequenceNumber];
	chrono::system_clock::duration timeDiff = now - sendTime;
	unsigned long long timeDiffInLong = (chrono::duration_cast<std::chrono::milliseconds> (timeDiff)).count();

	// time
	if (timeDiffInLong > rStat->maxTime) rStat->maxTime = timeDiffInLong;
	if (timeDiffInLong < rStat->minTime) {
		// cout << "previous min: " << rStat->minTime << endl;
		rStat->minTime = timeDiffInLong;
	}
	rStat->meanTime = ((rStat->meanTime * ((double)rStat->packageGot)) + timeDiffInLong) / ((double)(rStat->packageGot+1));

	rStat->packageGot = rStat->packageGot + 1;


	// compute jitter
	// 1. compute time difference
	timeDiff = now - startTime;
	timeDiffInLong = (chrono::duration_cast<std::chrono::milliseconds> (timeDiff)).count();
	// 2. compute total package & avg. time
	double packageCount = (double) rStat->packageGot;
	double avgTime = ((double)timeDiffInLong) / packageCount;
	// 3. iterate
	double jitter = rStat->jitter;
	jitter = ((jitter * (packageCount - 1.0)) + (((double)timeDiffInLong) - ((double)*previousTimeDiffInLong) - avgTime)) / packageCount;
	*previousTimeDiffInLong = timeDiffInLong;
	rStat->jitter = jitter;
}

void myClientRR(Statistics *stat, ResponseStat *rStat, int socket, struct sockaddr *addr, bool isUDP, int packageSize, unsigned int maxPackageOnTraffic) {

	char *package = createPackageBuffer(packageSize);
	unsigned int serverResponse;
	int sentPackageBytes, gotPackageBytes;
	unsigned int currentSequence = 0;
	chrono::system_clock::time_point startTime = chrono::system_clock::now();
	unsigned long long previousTimeDiffInLong = 0;

	// init stat
	// cout << "lock r1" << endl;
	statistics_display_m.lock();
	stat->isSessionStarted = true;
	stat->startTime = startTime;
	statistics_display_m.unlock();

	// 1st send
	while (currentSequence < maxPackageOnTraffic) {
		setAndIncreaseSequence(package, &currentSequence);
		// cout << "send() " << currentSequence << endl;
		sentPackageBytes = mySend(isUDP, socket, addr, package, packageSize);
		if (sentPackageBytes == 0 || sentPackageBytes == -1) {
			cout << "Cannot send..." << endl;
			freePackageBuffer(package);
			return;
		}

		// rStat->timeStore should be not used in other place
		rStat->timeStore[currentSequence] = chrono::system_clock::now();
	}

	// recv & send loop
	do {
		// blocking recv()
		gotPackageBytes = myRecv(isUDP, socket, NULL, (char *) &serverResponse, sizeof(serverResponse));
		if (gotPackageBytes <= 0) {
			cout << "Server connection closed" << endl;
			freePackageBuffer(package);
			return;
		}

		// process server response
		serverResponse = getSequence((char *) &serverResponse);
		// cout << "serverResponse: " << serverResponse << endl;

		// update response statistics
		statistics_display_m.lock();
		updateResponseStat(rStat, serverResponse, &previousTimeDiffInLong, startTime);
		statistics_display_m.unlock();

		// erase corresponding element in timeStore
		rStat->timeStore.erase(serverResponse);

		// send next
		setAndIncreaseSequence(package, &currentSequence);
		sentPackageBytes = mySend(isUDP, socket, addr, package, packageSize);
		if (sentPackageBytes == 0 || sentPackageBytes == -1) {
			cout << "Cannot send..." << endl;
			freePackageBuffer(package);
			return;
		}

		// insert the sequence number in the timeStore
		rStat->timeStore[currentSequence] = chrono::system_clock::now();

		std::this_thread::sleep_for(chrono::milliseconds(500));

	} while (true);

	freePackageBuffer(package);
}

void myServerRR(int socket, struct sockaddr *addr, bool isUDP, int packageSize) {

	char *package = createPackageBuffer(packageSize);
	unsigned int serverResponse;
	int sentPackageBytes, gotPackageBytes;

	do {
		// blocking recv()
		// cout << "waiting recv()..." << endl;
		gotPackageBytes = myRecv(isUDP, socket, addr, package, packageSize);
		if (gotPackageBytes <= 0) {
			cout << "[" << this_thread::get_id() << "] " << "Client connection closed" << endl;
			freePackageBuffer(package);
			return;
		}

		// send back the sequence number
		serverResponse = getSequence(package);
		serverResponse = htonl(serverResponse);
		sentPackageBytes = mySend(isUDP, socket, addr, (char *) &serverResponse, sizeof(serverResponse));
		if (sentPackageBytes == 0 || sentPackageBytes == -1) {
			cout << "Cannot send..." << endl;
			freePackageBuffer(package);
			return;
		}

		// cout << "[" << this_thread::get_id() << "] " << "serverResponse: " << ntohl(serverResponse) << endl;

	} while (true);

	freePackageBuffer(package);

}
