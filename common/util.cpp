#include "../myHead.h"

using namespace std;



void printBuffer(char *buf, int bSize) {
	for (int i = 0; i < bSize; i++) {
		if (i > 0) printf(":");
		printf("%02X", buf[i]);
	}
	cout << "\n\n";
}
void printAddress(struct sockaddr *address) {

	struct sockaddr_in *addr = (struct sockaddr_in *) address;
	char ip[INET_ADDRSTRLEN];
	int port = ntohs(addr->sin_port);

	if (inet_ntop(addr->sin_family, &(addr->sin_addr), ip, INET_ADDRSTRLEN) == NULL) {
		perror("Parse IP failed with error code, printAddress()");
	}

	cout << ip << ":" << port << endl;
}
void debugStat(Statistics *stat) {
	cout << "=== debugStat ===" << endl;
	cout << "byteOnTraffic" << ": " << stat->byteOnTraffic << endl;
	cout << "currentSequence" << ": " << stat->currentSequence << endl;
	cout << "lostCount" << ": " << stat->lostCount << endl;
	cout << "=== ending ===" << endl;
}

void strToLower(char *str) {
	for (unsigned int i = 0; i < strlen(str); i++) {
		str[i] = tolower(str[i]);
	}
}
bool isInteger(char *str) {
	bool isDigit = true;

	for (unsigned int i = 0; i < strlen(str); i++) {
		if (!isdigit(str[i])) isDigit = false;
	}

	return isDigit;
}
void errorExist(char *msg) {
	cout << "Invalid value for " << msg << endl;
	exit(1);
}
void getArguments(int argc, char *argv[]) {

	for (int idx = 1; idx < argc; idx++) {

		// mode
		if (strcmp(argv[idx], "-send") == 0) {
			mode = SEND;
			continue;
		} else if (strcmp(argv[idx], "-recv") == 0) {
			mode = RECV;
			continue;
		} else if (strcmp(argv[idx], "-host") == 0) {
			mode = HOST;
			continue;
		} else if (strcmp(argv[idx], "-response") == 0) {
			mode = RESPONSE;
			continue;
		}

		// statistic
		else if (strcmp(argv[idx], "-stat") == 0) {
			idx++;
			if (isInteger(argv[idx])) {
				displayInterval = atoi(argv[idx]);
				continue;
			} else errorExist(argv[idx - 1]);
		}

		// protocol
		else if (strcmp(argv[idx], "-proto") == 0) {
			idx++;
			strToLower(argv[idx]);
			if (strcmp(argv[idx], "udp") == 0) {
				protocol = UDP;
				continue;
			} else if (strcmp(argv[idx], "tcp") == 0) {
				protocol = TCP;
				continue;
			} else errorExist(argv[idx - 1]);
		}

		// port
		else if (strcmp(argv[idx], "-lport") == 0) {
			idx++;
			if (isInteger(argv[idx])) {
				lPort = atoi(argv[idx]);
				continue;
			} else errorExist(argv[idx - 1]);
		} else if (strcmp(argv[idx], "-rport") == 0) {
			idx++;
			if (isInteger(argv[idx])) {
				rPort = atoi(argv[idx]);
				continue;
			} else errorExist(argv[idx - 1]);
		}

		// host
		else if (strcmp(argv[idx], "-host") == 0) {
			rhostname = argv[++idx];
			continue;
		} else if (strcmp(argv[idx], "-rhost") == 0) {
			rhostname = argv[++idx];
			continue;
		} else if (strcmp(argv[idx], "-lhost") == 0) {
			lhostname = argv[++idx];
			continue;
		}

		// package
		else if (strcmp(argv[idx], "-pktsize") == 0) {
			idx++;
			if (isInteger(argv[idx])) {
				packageSize = atoi(argv[idx]);
				continue;
			} else errorExist(argv[idx - 1]);
		} else if (strcmp(argv[idx], "-pktnum") == 0) {
			idx++;
			if (isInteger(argv[idx])) {
				packageNummber = atoi(argv[idx]);
				continue;
			} else errorExist(argv[idx - 1]);
		}

		// buffer
		else if (strcmp(argv[idx], "-rbufsize") == 0) {
			idx++;
			if (isInteger(argv[idx])) {
				rBufferSize = atoi(argv[idx]);
				continue;
			} else errorExist(argv[idx - 1]);
		} else if (strcmp(argv[idx], "-sbufsize") == 0) {
			idx++;
			if (isInteger(argv[idx])) {
				sBufferSize = atoi(argv[idx]);
				continue;
			} else errorExist(argv[idx - 1]);
		}

		// rate
		else if (strcmp(argv[idx], "-pktrate") == 0) {
			idx++;
			if (isInteger(argv[idx])) {
				txRate = atoi(argv[idx]);
				continue;
			} else errorExist(argv[idx - 1]);
		}

		// unknown option
		else errorExist(argv[idx]);
	}
}

void initStat(Statistics *stat) {
	stat->isEnded = false;
	stat->isSessionStarted = false;

	stat->byteOnTraffic = 0L;
	stat->currentSequence = 0;
	stat->lostCount = 0;
	stat->jitter = 0.0;
}

void initResponseStat(ResponseStat *stat) {
	stat->maxTime = 0;
	stat->minTime = (unsigned long long) -1L;

	stat->meanTime = 0.0;
	stat->packageGot = 0;

	stat->jitter = 0.0;
}

void printStat(Statistics *stat, ResponseStat *rStat, Mode mode, unsigned int packageSize) {

	static double rateUnitConstant = 1000.0 / (1024.0 * 1024.0); // byte per ms => Mb per s
	// static char *sendOutputFormat = "Elapsed [%lu ms] Rate [%.9f Mbps]";

	// static char *recvOutputFormat1 = "Elapsed [%lu ms] Pkts [%u] Lost [%u, %.2f%%] ";
	// static char *recvOutputFormat2 = "Rate [%.9f Mbps] Jitter [%.2f ms]";

	// static char *responseOutputFormat = "Pkts [%u] Max [%lu ms] Min [%lu ms] Mean [%.2f ms] Jitter [%.2f ms]";
	// why separate? because it crash in windows

	// variables
	chrono::system_clock::duration elapsedTime;
	unsigned long long elapsedTimeInLong;
	double lostPercentage, rate;
	unsigned int packageArrived;

	if (mode != RESPONSE) {
		// some calculation
		elapsedTime = chrono::system_clock::now() - stat->startTime;
		elapsedTimeInLong = (chrono::duration_cast<std::chrono::milliseconds> (elapsedTime)).count();
		if (elapsedTimeInLong == 0L) elapsedTimeInLong = 1;
		packageArrived = stat->currentSequence - stat->lostCount;
		rate = (((double)stat->byteOnTraffic) / ((double)elapsedTimeInLong)) * rateUnitConstant;
	}

	// std::cout << ((double)stat->byteOnTraffic) / ((double)elapsedTimeInLong) << ", " << rate << std::endl;
	// std::cout << stat->byteOnTraffic << ", " << elapsedTimeInLong << std::endl;

	// calculation + display
	switch (mode) {
		case RECV:
			lostPercentage = 100.0 * ((double)stat->lostCount) / ((double)(stat->currentSequence));
			cout << "Elapsed [" << elapsedTimeInLong << " ms]";
			cout << " Pkts [" << packageArrived << "]";
			cout << " Lost [" << stat->lostCount << "," << lostPercentage << "%%]";
			cout << " Rate [" << rate << " Mbps ";
			cout << " Jitter [" << stat->jitter << " ms]";
			break;
		case SEND:
			cout << "Elapsed [" << elapsedTimeInLong << " ms]";
			cout << " Rate [" << rate << " Mbps]";
			break;
		case RESPONSE:
			cout << "Pkts" << " [" << rStat->packageGot << "]";
			cout << " Max" << " [" << rStat->maxTime << " ms]";
			cout << " Min" << " [" << rStat->minTime << " ms]";
			cout << " Mean" << " [" << rStat->meanTime << " ms]";
			cout << " Jitter" << " [" << rStat->jitter << " ms]";
			break;
		default:
			puts("on9 mode: no stat");
			break;
	}
		
	std::cout << std::endl;
}
