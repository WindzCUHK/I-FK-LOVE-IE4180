#include "myHead.h"

using namespace std;




void printBuffer(char *buf, int bSize) {
	for (int i = 0; i < bSize; i++) {
		if (i > 0) printf(":");
		printf("%02X", buf[i]);
	}
	printf("\n\n");
}


void strToLower(char *str) {
	for (int i = 0; i < strlen(str); i++) {
		str[i] = tolower(str[i]);
	}
}
bool isInteger(char *str) {
	bool isDigit = true;

	for (int i = 0; i < strlen(str); i++) {
		if (!isdigit(str[i])) isDigit = false;
	}

	return isDigit;
}
void errorExist(char *msg) {
	cout << "Invalid value for " << msg << endl;
	exit(1);
}
char getArguments(int argc, char *argv[]) {

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
				txRate = (double) atoi(argv[idx]);
				continue;
			} else errorExist(argv[idx - 1]);
		}

		// unknown option
		else errorExist(argv[idx]);
	}
}