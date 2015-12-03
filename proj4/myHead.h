#ifndef _MY_HEAD_H_
#define _MY_HEAD_H_

// common c libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>
#include <ctime>

// common cpp libraries
#include <iostream>
#include <iomanip>
#include <queue>
#include <vector>
#include <algorithm>
#include <string>

// thread libraries
#include <condition_variable>
#include <thread>
#include <mutex>
#include <chrono>

// socket libraries
#ifdef WIN32
	#define NOMINMAX
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

// file meta library
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
	#include "dirent.h"
#else
	#include <dirent.h>
#endif

// custom header
#include "model.h"

// connect.cpp
void myDied(char *str);
int getConnectSocket(char *host, int port, Protocol protocol, struct sockaddr_in *serverAddress);
int getListenSocket(char *host, int port, Protocol protocol, struct sockaddr_in *listenAddress);
#ifdef WIN32
	void initWinsock(WSADATA *ptr_wsa);
#endif

// myHTTP.cpp
int myTcpSend(int socket, const char *buffer, int bufferSize);
int myTcpRecv(int socket, const char *buffer, int bufferSize);
bool myRequestRecv(int socket, char *buffer, int bufferSize);
bool parseAndValidateRequest(std::string const &request, std::string &method, std::string &url, std::string &httpVersion);
bool createAndSendResponse(int socket, std::string const &filePath, std::string const &httpVersion);

#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
