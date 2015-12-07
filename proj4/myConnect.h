#ifndef _MY_CONNECT_H_
#define _MY_CONNECT_H_

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

namespace constants {
	cost std::string HTTP_inline_delimiter(" ");
	cost std::string HTTP_line_break("\r\n");
}

enum Protocol {
	TCP = 0,
	UDP = 1
};
// typedef struct _ConnectInfo {
// 	int socket;
// 	struct sockaddr_in address;
// } ConnectInfo;

// connect.cpp
void myDied(char *str);
int getConnectSocket(char *host, int port, Protocol protocol, struct sockaddr_in *serverAddress);
int getListenSocket(char *host, int port, Protocol protocol, struct sockaddr_in *listenAddress);
bool mySocketClose(int socket);
#ifdef WIN32
	void initWinsock(WSADATA *ptr_wsa);
#endif

// myHTTP.cpp
int myTcpSend(int socket, const char *buffer, int bufferSize);
int myTcpRecv(int socket, const char *buffer, int bufferSize);
bool myRequestRecv(int socket, char *buffer, int bufferSize);
bool parseAndValidateGetRequest(std::string const &request, std::string &method, std::string &url, std::string &httpVersion);
bool createAndSendGetResponse(int socket, std::string const &filePath, std::string const &httpVersion);

#endif
