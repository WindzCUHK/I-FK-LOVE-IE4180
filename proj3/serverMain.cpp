#include "myHead.h"

using namespace std;

// thread pool
vector<thread> pool;

// job queue and its mutex
queue<ConnectInfo> jobQueue;
mutex job_q_mutex;
condition_variable hasNewJob;
// display mutex
mutex g_display_mutex;

void threadPrint(char* str1, char *str2) {
	lock_guard<mutex> jqLock(g_display_mutex);
	cout << '[' << this_thread::get_id() << "] " << str1 << str2 << endl;
}

void createResponse(int socket, char *filePath, int major, int minor) {

	threadPrint("file path: ", filePath);

	// file path
	int pathLength = strlen(filePath);
	char relativePath[pathLength];
	relativePath[0] = '.';
	memcpy(relativePath + 1, filePath, pathLength + 1);

	// open file
	int responseHeaderSize = 255;
	char responseHeader[responseHeaderSize + 1];
	int fileBufferSize = 4096, readBytes
;	char fileBuffer[fileBufferSize];
	FILE *file;
	file = fopen(relativePath, "rb");

	// handle and send file
	if (file == NULL) {
		snprintf(responseHeader, responseHeaderSize, "HTTP/%d.%d 404 Not Found\r\n\r\n", major, minor);
		myTcpSend(socket, responseHeader, strlen(responseHeader));
		return;
	} else {
		fseek(file, 0L, SEEK_END);
		long contentLength = ftell(file);
		fseek(file, 0L, SEEK_SET);

		snprintf(responseHeader, responseHeaderSize, "HTTP/%d.%d 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", major, minor, contentLength);
		myTcpSend(socket, responseHeader, strlen(responseHeader));

		// read and send file content
		do {
			readBytes = fread(fileBuffer, 1, fileBufferSize, file);
			myTcpSend(socket, fileBuffer, readBytes);
		} while (readBytes == fileBufferSize);

		fclose(file);
	}
}

void httpHandler(int socket, struct sockaddr_in address) {

	// debug print
	int printBufferSize = 255;
	char printBuffer[printBufferSize + 1];
	memset(printBuffer, '\0', printBufferSize + 1);

	// print client IP:port
	char clientIP[INET_ADDRSTRLEN];
	int clientPort = ntohs(address.sin_port);
	if (inet_ntop(address.sin_family, &(address.sin_addr), clientIP, INET_ADDRSTRLEN) == NULL) {
		perror("Parse client IP failed with error code");
		return;
	}
	snprintf(printBuffer, printBufferSize, "%s:%d", clientIP, clientPort);
	threadPrint("Got client: ", printBuffer);

	// recv init
	int bufferSize = 4096, gotBytes = 0, result;
	char buffer[bufferSize];
	memset(buffer, '\0', bufferSize);
	"GET /index.html HTTP/1.1\r\n\r\n";

	// take out the whole request header
	do {
		const int result = recv(socket, buffer + gotBytes, bufferSize - gotBytes, 0);
		if (result == 0) break;
		if (result == -1) {
			perror("recv()");
			return;
		}
	} while (buffer[gotBytes - 4] != '\r'|| buffer[gotBytes - 3] != '\n'|| buffer[gotBytes - 2] != '\r'|| buffer[gotBytes - 1] != '\n');
	buffer[gotBytes] = '\0'
	threadPrint("HTTP request: ", buffer);

	// process request
	const std::string s = buffer;
	std::regex rgx("^GET\\s+(.+)\\s+HTTP\\/(\\d)\\.(\\d)\r\n(.+\r\n)*\r\n$");
	std::smatch match;

	if (std::regex_search(s.begin(), s.end(), match, rgx)) {
		threadPrint("path: ", match[1]);
		threadPrint("major: ", match[2]);
		threadPrint("minor: ", match[3]);
	} else {

	}
	// const char *filePath = request.url().c_str();

	// create response
	createAndSendResponse(socket, filePath, request.majorversion(), request.minorversion());

	// close socket
	#ifdef WIN32
		if (closesocket(socket) == -1) {
	#else
		if (close(socket) == -1) {
	#endif
		perror("close(), exiting...");
		exit(1);
	}

	// cleanup
	request.clear();
}

void pooledClientHandler() {

	threadPrint("--- thread ", "start");

	ConnectInfo job;
	unique_lock<mutex> jqLock(job_q_mutex, defer_lock);
	while (true) {

		jqLock.lock();

		// wait for job
		while (jobQueue.empty()) {
			hasNewJob.wait(jqLock);
		}

		// take out the job
		job = jobQueue.front();
		jobQueue.pop();

		jqLock.unlock();

		httpHandler(job.socket, job.address);
	}

	threadPrint("--- thread ", "end");
	cout << endl;
}

void clientHandler(int clientSocket, struct sockaddr_in clientAddress) {

	threadPrint("--- thread ", "start");

	httpHandler(clientSocket, clientAddress);

	threadPrint("--- thread ", "end");
	cout << endl;
}

void myAccept(bool isThreadPool, int threadPoolSize, int listenSocket) {

	int clientSocket;
	struct sockaddr_in clientAddress;
	socklen_t addressSize = sizeof(clientAddress);

	// init thread pool
	if (isThreadPool) {
		for (int i = 0; i < threadPoolSize; i++) {
			pool.emplace_back(pooledClientHandler);
		}
	}

	while (true) {
		// accept new socket
		memset((char *) &clientAddress, 0, addressSize);
		clientSocket = accept(listenSocket, (struct sockaddr *) &clientAddress, &addressSize);
		if (clientSocket == -1) {
			perror("accept()");
			continue;
		}
		cout << "[Main thread] Got new client XD" << endl;

		if (isThreadPool) {

			// create job
			ConnectInfo job;
			job.socket = clientSocket;
			job.address = clientAddress;

			// push job, auto release lock
			{
				lock_guard<mutex> jqLock(job_q_mutex);
				jobQueue.push(job);
			}

			// notify any thread in the pool
			hasNewJob.notify_one();

		} else {
			// start new thread
			pool.emplace_back(clientHandler, clientSocket, clientAddress);
		}
	}

	// wait for threads join
	for (auto &thread: pool) thread.join();
}	

int main(int argc, char* argv[]) {

	// constants
	const Protocol protocol = TCP;

	/* Alvin's part */
	// get argument from command line (need checking?)
	// get from argument
	int port = atoi(argv[2]);
	bool isThreadPool = false;
	int threadPoolSize = 0;
	if (argv[3][0] == 'o') {
		isThreadPool = false;
	} else if (argv[3][0] == 'p') {
		isThreadPool = true;
		threadPoolSize = atoi(argv[4]);
	} else {
		cout << "On9 mode, cannot handle" << endl;
		exit(1);
	}

	// listent to connection
	#ifdef WIN32
		WSADATA wsa;
		initWinsock(&wsa);
	#endif
	struct sockaddr_in listenAddress;
	int listenSocket = getListenSocket(NULL, port, protocol, &listenAddress);

	// accept loop
	cout << "Waiting connection... on port " << port << endl;
	myAccept(isThreadPool, threadPoolSize, listenSocket);
	cout << "KO" << endl;

	return 0;
}
