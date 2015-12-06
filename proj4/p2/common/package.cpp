#include "../myHead.h"

using namespace std;

void setAndIncreaseSequence(char *packageBuffer, unsigned int *currentSequence) {

	*currentSequence = *currentSequence + 1;

	unsigned int outSequence = htonl(*currentSequence);
	memcpy(packageBuffer, &outSequence, sizeof(unsigned int));
}
unsigned int getSequence(char *packageBuffer) {

	unsigned int sequence;
	memcpy(&sequence, packageBuffer, sizeof(unsigned int));

	return ntohl(sequence);
}

char * createPackageBuffer(int bufferSize) {
	char *packageBuffer = (char *) malloc(bufferSize);
	memset(packageBuffer, '-' , bufferSize);
	return packageBuffer;
}
void freePackageBuffer(char *packageBuffer) {
	free(packageBuffer);
}

void initHello(HelloPackage *hello) {

	hello->packageSize = htonl((unsigned int) packageSize);
	hello->packageNummber = htonl((unsigned int) packageNummber);
	hello->txRate = htonl((unsigned int) txRate);
	hello->bufferSize = htonl((unsigned int) ((mode != RECV) ? rBufferSize : sBufferSize));

	hello->protocol = ((protocol == UDP) ? 'U' : 'T');
	switch (mode) {
		case SEND:
			hello->mode = '0';
			break;
		case RECV:
			hello->mode = '1';
			break;
		case HOST:
			hello->mode = '2';
			break;
		case RESPONSE:
			hello->mode = '3';
			break;
	}
	hello->clientUdpListenPort = htons((unsigned short) lPort);
}
void parseHello(HelloPackage *hello, Mode *mode, Protocol *protocol) {
	hello->packageSize = ntohl(hello->packageSize);
	hello->packageNummber = ntohl(hello->packageNummber);
	hello->txRate = ntohl(hello->txRate);
	hello->bufferSize = ntohl(hello->bufferSize);

	*protocol = ((hello->protocol == 'U') ? UDP : TCP);
	switch (hello->mode) {
		case '0':
			*mode = RECV;	// client is send, server = recv
			break;
		case '1':
			*mode = SEND;	// client is recv, server = send
			break;
		case '2':
			*mode = HOST;	// should not happen
			break;
		case '3':
			*mode = RESPONSE;	// response
			break;
	}

	hello->clientUdpListenPort = ntohs(hello->clientUdpListenPort);
}
void printHello(HelloPackage *hello) {
	cout << "HelloPackage {" << endl;
	cout << "\tpackageSize: " << ntohl(hello->packageSize) << endl;
	cout << "\tpackageNummber: " << ntohl(hello->packageNummber) << endl;
	cout << "\ttxRate: " << ntohl(hello->txRate) << endl;
	cout << "\tbufferSize: " << ntohl(hello->bufferSize) << endl;
	cout << "\tprotocol: " << hello->protocol << endl;
	cout << "\tmode: " << hello->mode << endl;
	cout << "\tclientUdpListenPort: " << ntohs(hello->clientUdpListenPort) << endl;
	cout << "}" << endl;
}
