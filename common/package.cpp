#include "../myHead.h"

using namespace std;

void setSequence(char *packageBuffer, unsigned int *currentSequence) {

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
	return (char *) malloc(bufferSize);
}
void freePackageBuffer(char *packageBuffer) {
	free(packageBuffer);
}

void initHello(HelloPackage *hello) {
	hello->packageSize = htonl((unsigned int) packageSize);
	hello->packageNummber = htonl((unsigned int) packageNummber);
	hello->txRate = htonl((unsigned int) txRate);
	hello->bufferSize = htonl((unsigned int) bufferSize);
	hello->protocol = ((protocol == UDP) ? 'U' : 'T');
}