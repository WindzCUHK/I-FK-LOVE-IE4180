#include "myHead.h"

using namespace std;

Mode mode;
Protocol protocol;
int displayInterval;
int packageSize;
char* rhostname;
int rPort;
int sBufferSize;
double txRate;
int packageNummber;
char* lhostname;
int lPort;
int rBufferSize;

int main(int argc, char *argv[]) {

	getArguments(argc, argv);
	
	struct sockaddr_in serverAddress;
	int serverSocket = getConnectSocket(rhostname, rPort, protocol, &serverAddress);
	close(serverSocket);
	cout << serverSocket << endl;
	

	/*
	if (sendto(s, my_message, strlen(my_message), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("sendto failed");
		return 0;
	}

	struct sockaddr_in senderAddress;
	socklen_t addrlen = sizeof(senderAddress);
	recvlen = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&senderAddress, &addrlen);
	if (recvlen > 0) {
	    buf[recvlen] = 0;
	    printf("received message: \"%s\"\n", buf);
	}
	*/
}
