#ifndef _MY_CONNECT_H_
#define _MY_CONNECT_H_

// socket libraries
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

#define HTTP_REQUEST_ENDING "\r\n\r\n"
#define HTTP_CONTENT_HEADER "Content-Length:"
namespace constants {
	const std::string HTTP_GET("GET");
	const std::string HTTP_POST("POST");
	const std::string HTTP_inline_delimiter(" ");
	const std::string HTTP_line_break("\r\n");
	const std::string HTTP_path_delimiter("/");
	const std::string HTTP_absolute_path_prefix("http://");
	const std::string HTTP_connection_keep_alive("Connection: Keep-Alive");

	// application constant
	// GET path
	const std::string SERVER_list_path("/list");
	const std::string SERVER_restore_path("/restore");
	const std::string SERVER_restore_list_path("/restoreList");
	// POST path
	const std::string SERVER_delete_path("/delete");
	const std::string SERVER_create_path("/create");
	const std::string SERVER_update_path("/update");

	const std::string REQUEST_default_http_version("HTTP/1.1");
	const std::string POST_content_type("Content-Type: application/octet-stream");
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
int getConnectSocketByAddress(Protocol protocol, struct sockaddr_in *address);
int getConnectSocket(char *host, int port, Protocol protocol, struct sockaddr_in *serverAddress);
int getListenSocket(char *host, int port, Protocol protocol, struct sockaddr_in *listenAddress);
bool mySocketClose(int socket);
#ifdef WIN32
	void initWinsock(WSADATA *ptr_wsa);
#endif

// myHTTP.cpp
int myTcpSend(int socket, const char *buffer, int bufferSize);
int myTcpRecv(int socket, const char *buffer, int bufferSize);
bool myResponseRecv(int socket, std::ostream &os);
bool myRequestRecv(int socket, char *buffer, int bufferSize, std::ostringstream &oss);
bool parseAndValidateRequest(const std::string &request, std::string &method, std::string &url, std::string &httpVersion);
bool createAndSendResponse(int socket, const std::string &url, const std::string &httpVersion, const std::string &content, long contentLength);
bool createAndSendOK(int socket, const std::string &httpVersion);
bool createAndSendRequest(int socket, bool isGet, const std::string &url, const std::string &httpVersion, bool isKeepAlive, const char *content, int contentSize);

#endif
