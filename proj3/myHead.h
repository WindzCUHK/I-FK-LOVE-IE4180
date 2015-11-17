#ifndef _MY_HEAD_H_
#define _MY_HEAD_H_

// common libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <map>
#include <queue>
#include <vector>
#include <regex>

// thread libraries
#include <condition_variable>
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

// data type
typedef enum {TCP, UDP} Protocol;
typedef struct _ConnectInfo {
	int socket;
	struct sockaddr_in address;
} ConnectInfo;

// connect.cpp
void myDied(char *str);
int getConnectSocket(char *host, int port, Protocol protocol, struct sockaddr_in *serverAddress);
int getListenSocket(char *host, int port, Protocol protocol, struct sockaddr_in *listenAddress);
#ifdef WIN32
	void initWinsock(WSADATA *ptr_wsa);
#endif

// tcpSendRecv.cpp
int myTcpSend(int socket, char *buffer, int bufferSize);
int myTcpRecv(int socket, char *buffer, int bufferSize);


#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
