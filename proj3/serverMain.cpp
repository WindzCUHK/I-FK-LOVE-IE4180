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

bool mySocketClose(int socket) {
	#ifdef WIN32
		if (closesocket(socket) == -1) {
	#else
		if (close(socket) == -1) {
	#endif
		perror("close(), exiting...");
		return false;
	}

	return true;
}

void httpHandler(int socket, struct sockaddr_in address) {

	// print client IP:port
	char clientIP[INET_ADDRSTRLEN];
	int clientPort = ntohs(address.sin_port);
	if (inet_ntop(address.sin_family, &(address.sin_addr), clientIP, INET_ADDRSTRLEN) == NULL) {
		perror("Parse client IP failed with error code");
		mySocketClose(socket);
		return;
	}
	threadPrint("Got client: ", (char *) ((std::string((char *) clientIP) + ":" + to_string(clientPort)).c_str()));

	// recv init
	int bufferSize = BUFFER_SIZE;
	char buffer[BUFFER_SIZE];
	memset(buffer, '\0', bufferSize);

	// take out the whole request header
	if (!myRequestRecv(socket, buffer, bufferSize)) {
		threadPrint("Error: ", "recv() error OR request >= 4096 bytes");
		mySocketClose(socket);
		return;
	}
	threadPrint("HTTP request:\n", buffer);

	// process request
	const std::string requestString = buffer;
	std::string method, url, httpVersion;
	if (!parseAndValidateRequest(requestString, method, url, httpVersion)) {
		threadPrint("Error: ", "Unknown request header");
		mySocketClose(socket);
		return;
	}

	// create response
	threadPrint("Request file path: ", (char*) url.c_str());
	if (!createAndSendResponse(socket, url, httpVersion)) {
		threadPrint("Error: ", "file IO OR send()");
		mySocketClose(socket);
		return;
	}

	// close socket
	if (!mySocketClose(socket)) {
		threadPrint("Error: ", "socket close()");
	}
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
