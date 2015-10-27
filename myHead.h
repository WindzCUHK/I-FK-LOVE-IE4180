#ifndef _MY_HEAD_H_
#define _MY_HEAD_H_

// common libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <iostream>

// thread libraries
#include <thread>
#include <mutex>
#include <chrono>

// socket libraries
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

// data types
typedef enum {SEND, RECV, HOST, RESPONSE} Mode;
typedef enum {TCP, UDP} Protocol;



// common
extern Mode mode;			// -send, -recv, -host, -response
extern Protocol protocol;	// -proto udp
extern int displayInterval;	// -stat 500 ms
extern int packageSize;		// -pktsize 1000 bytes
// send
extern char* rhostname;		// -host, -rhost localhost
extern int rPort;			// -rport 4180
extern int sBufferSize;		// -sbufsize -1
extern double txRate;		// -pktrate 1000 bytes/second
extern int packageNummber;	// -pktnum 0
// recv
extern char* lhostname;		// -lhost IN_ADDR_ANY
extern int lPort;			// -lport 4180
extern int rBufferSize;		// -rbufsize -1


// util functions
char getArguments(int argc, char *argv[]);
void printBuffer(char *buf, int bSize);

// connect
int getConnectSocket(char *host, int port, Protocol protocol, struct sockaddr_in *serverAddress);
int getListenSocket(char *host, int port, Protocol protocol, struct sockaddr_in *listenAddress);
void myAccept(int listenSocket);

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

#endif