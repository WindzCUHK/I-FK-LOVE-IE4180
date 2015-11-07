#ifndef _MY_HEAD_H_
#define _MY_HEAD_H_

// common libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <iostream>
#include <map>

// thread libraries
#include <thread>
#include <mutex>
#include <chrono>

// socket libraries
#include <errno.h>
#include <sys/types.h>
#ifdef WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib,"ws2_32.lib") 
#else
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <unistd.h>
#endif

// data types
typedef enum {SEND, RECV, HOST, RESPONSE} Mode;
typedef enum {TCP, UDP} Protocol;

typedef struct _Statistics {
	bool isEnded;

	bool isSessionStarted;
	std::chrono::system_clock::time_point startTime;

	unsigned long long byteOnTraffic;
	unsigned int currentSequence;
	unsigned int lostCount;
	double jitter;
} Statistics;

typedef struct _HelloPackage {
	unsigned int packageSize;
	unsigned int packageNummber;
	unsigned int txRate;
	unsigned int bufferSize;
	unsigned char protocol;
	unsigned char mode;
	unsigned short clientUdpListenPort;
} HelloPackage;

typedef struct _ResponseStat {

	unsigned long long maxTime;
	unsigned long long minTime;
	
	double meanTime;
	unsigned int packageGot;

	std::map<unsigned int, std::chrono::system_clock::time_point> timeStore;

	double jitter;

} ResponseStat;

// common
extern Mode mode;			// -send, -recv, -host, -response
extern Protocol protocol;	// -proto udp
extern int displayInterval;	// -stat 500 ms
extern int packageSize;		// -pktsize 1000 bytes
// send
extern char* rhostname;		// -host, -rhost localhost
extern int rPort;			// -rport 4180
extern int sBufferSize;		// -sbufsize -1
extern int txRate;		// -pktrate 1000 bytes/second
extern int packageNummber;	// -pktnum 0
// recv
extern char* lhostname;		// -lhost IN_ADDR_ANY
extern int lPort;			// -lport 4180
extern int rBufferSize;		// -rbufsize -1

// lock
extern std::mutex statistics_display_m;

// util functions
void debugStat(Statistics *stat);
void getArguments(int argc, char *argv[]);
void printBuffer(char *buf, int bSize);
void printAddress(struct sockaddr *address);

void initStat(Statistics *stat);
void initResponseStat(ResponseStat *stat);
void printStat(Statistics *stat, ResponseStat *rStat, Mode mode, unsigned int packageSize);

// connect
int getConnectSocket(char *host, int port, Protocol protocol, struct sockaddr_in *serverAddress);
int getListenSocket(char *host, int port, Protocol protocol, struct sockaddr_in *listenAddress);

// package
void setAndIncreaseSequence(char *packageBuffer, unsigned int *currentSequence);
unsigned int getSequence(char *packageBuffer);
char * createPackageBuffer(int bufferSize);
void freePackageBuffer(char *packageBuffer);
void initHello(HelloPackage *hello);
void parseHello(HelloPackage *package, Mode *mode, Protocol *protocol);
void printHello(HelloPackage *hello);

// send and recv
int mySend(bool isUDP, int socket, struct sockaddr *addr, char *package, int packageSize);
void mySendLoop(bool isClient, Statistics *stat, int socket, struct sockaddr *addr, bool isUDP, int packageSize, int rate, unsigned int maxSequence);
int myRecv(bool isUDP, int socket, struct sockaddr *addr, char *package, int packageSize);
void myRecvLoop(bool isClient, Statistics *stat, int socket, struct sockaddr *addr, bool isUDP, int packageSize, unsigned int maxSequence);

// response
void myServerRR(int socket, struct sockaddr *addr, bool isUDP, int packageSize);
void myClientRR(Statistics *stat, ResponseStat *rStat, int socket, struct sockaddr *addr, bool isUDP, int packageSize, unsigned int maxPackageOnTraffic);

#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

