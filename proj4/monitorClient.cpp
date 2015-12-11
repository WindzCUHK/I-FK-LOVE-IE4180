#include "myHead.h"

std::mutex g_display_mutex;

void filesDifference(const std::vector<FileMeta> &oldFileMetas, const std::vector<FileMeta> &fileMetas, std::vector<FileMeta> &update_list, std::vector<FileMeta> &delete_list, std::vector<FileMeta> &create_list) {

	std::vector<FileMeta> file_intersection, old_difference, current_difference;

	// compare file hierarchy change
	std::set_intersection(
		oldFileMetas.begin(),
		oldFileMetas.end(),
		fileMetas.begin(),
		fileMetas.end(),
		std::back_inserter(file_intersection),
		cmpFileMeta
	);

	// get changed files in the old list
	std::set_difference(
		oldFileMetas.begin(),
		oldFileMetas.end(),
		file_intersection.begin(),
		file_intersection.end(),
		std::back_inserter(old_difference),
		cmpFileMeta
	);

	// get changed files in the current list
	std::set_difference(
		fileMetas.begin(),
		fileMetas.end(),
		file_intersection.begin(),
		file_intersection.end(),
		std::back_inserter(current_difference),
		cmpFileMeta
	);

	// get update list (path exist in both list)
	std::set_intersection(
		current_difference.begin(),
		current_difference.end(),
		old_difference.begin(),
		old_difference.end(),
		std::back_inserter(update_list),
		cmpFileMetaPathOnly
	);
	// deleted file = path only exist in old list
	std::set_difference(
		old_difference.begin(),
		old_difference.end(),
		update_list.begin(),
		update_list.end(),
		std::back_inserter(delete_list),
		cmpFileMetaPathOnly
	);
	// created file = path only exist in current list
	std::set_difference(
		current_difference.begin(),
		current_difference.end(),
		update_list.begin(),
		update_list.end(),
		std::back_inserter(create_list),
		cmpFileMetaPathOnly
	);

	// std::cout << "1. " << "old list" << std::endl;
	// for_each(oldFileMetas.begin(), oldFileMetas.end(), printFileMeta);
	// std::cout << "1. " << "new list" << std::endl;
	// for_each(fileMetas.begin(), fileMetas.end(), printFileMeta);
	// std::cout << "2. " << "unchanged" << std::endl;
	// for_each(file_intersection.begin(), file_intersection.end(), printFileMeta);
	// std::cout << "2.1. " << "old diff" << std::endl;
	// for_each(old_difference.begin(), old_difference.end(), printFileMeta);
	// std::cout << "2.2. " << "cur diff" << std::endl;
	// for_each(current_difference.begin(), current_difference.end(), printFileMeta);
	// std::cout << "3. " << "update" << std::endl;
	// for_each(update_list.begin(), update_list.end(), printFileMeta);
	// std::cout << "4. " << "delete" << std::endl;
	// for_each(delete_list.begin(), delete_list.end(), printFileMeta);
	// std::cout << "5. " << "create" << std::endl;
	// for_each(create_list.begin(), create_list.end(), printFileMeta);

	// clean up
	file_intersection.clear();
	old_difference.clear();
	current_difference.clear();
}

void monitorFile(int socket, const std::string &monitorPath, int refreshInterval, int listenPort) {

	// thread output stream
	std::ostringstream oss, bodyss;
	threadPrint("--- monitorFile thread start", "\n");

	// variables
	std::string listenPortString = std::to_string(listenPort);
	std::vector<FileMeta> serverFileMetas;
	std::vector<FileMeta> localfileMetas;
	std::vector<FileMeta> update_list, delete_list, create_list;

	while (true) {
		oss << "=== monitor loop start\n";

		// get most update file list from server
		if (!createAndSendRequest(socket, true, constants::SERVER_list_path, constants::REQUEST_default_http_version, true, NULL, 0)) {
			oss << "Error createAndSendRequest(): sending GET /list request\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}
		oss << "GET request sent\n";

		// wait for server to response file list, and then decode + sort it
		bodyss.str("");
		if (!myResponseRecv(socket, bodyss)) {
			oss << "Error myResponseRecv(): receiving GET /list response\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}
		decodeFileMetas(cgicc::form_urldecode(bodyss.str()), serverFileMetas);
		sort(serverFileMetas.begin(), serverFileMetas.end(), cmpFileMeta);
		oss << "GET response got\n";

		// generate local file list
		if (listAllFilesInDir(localfileMetas, monitorPath, monitorPath, false) == EXIT_FAILURE) {
			myDied("Cannot monitor directory!!!");
		}
		sort(localfileMetas.begin(), localfileMetas.end(), cmpFileMeta);
		oss << "create local file list\n";

		// difference between server and client
		filesDifference(serverFileMetas, localfileMetas, update_list, delete_list, create_list);
		oss << "diff local and server\n";

		// POST changes to server, HTTP pipeline

		std::string contentString;
		std::string httpVersion = constants::REQUEST_default_http_version;
		oss << "POST /delete\n";
		contentString = listenPortString + constants::HTTP_inline_delimiter + cgicc::form_urlencode(encodeFileMetas(delete_list));
		if (!createAndSendRequest(socket, false, constants::SERVER_delete_path, httpVersion, true, contentString.c_str(), contentString.length())) {
			oss << "Error createAndSendRequest(): sending POST /delete request\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}
		oss << "POST /create\n";
		contentString = listenPortString + constants::HTTP_inline_delimiter + cgicc::form_urlencode(encodeFileMetas(create_list));
		if (!createAndSendRequest(socket, false, constants::SERVER_create_path, httpVersion, true, contentString.c_str(), contentString.length())) {
			oss << "Error createAndSendRequest(): sending POST /create request\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}
		oss << "POST /update\n";
		contentString = listenPortString + constants::HTTP_inline_delimiter + cgicc::form_urlencode(encodeFileMetas(update_list));
		if (!createAndSendRequest(socket, false, constants::SERVER_update_path, httpVersion, true, contentString.c_str(), contentString.length())) {
			oss << "Error createAndSendRequest(): sending POST /update request\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}

		bodyss.str("");
		if (!myResponseRecv(socket, bodyss)) {
			oss << "Error myResponseRecv(): receiving POST /delete response\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}
		bodyss.str("");
		if (!myResponseRecv(socket, bodyss)) {
			oss << "Error myResponseRecv(): receiving POST /update response\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}
		bodyss.str("");
		if (!myResponseRecv(socket, bodyss)) {
			oss << "Error myResponseRecv(): receiving POST /create response\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}
		oss << "POST response DONE\n";

		// clear up vectors
		localfileMetas.clear();
		serverFileMetas.clear();
		update_list.clear();
		delete_list.clear();
		create_list.clear();

		// loop ending
		oss << "=== monitor loop end! Waiting " << refreshInterval << "ms ...\n";
		threadPrint(oss.str().c_str(), "\n");
		oss.str("");

		// sleep
		std::this_thread::sleep_for(std::chrono::milliseconds(refreshInterval));
	}
}

void httpGetHandler(int socket, const std::string &path) {

	// thread output stream
	std::ostringstream oss, bodyss;
	oss << "--- httpGetHandler thread start\n";

	// request buffer
	char buffer[BUFFER_SIZE];
	int bufferSize = BUFFER_SIZE;

	// take out the whole request header
	bodyss.str("");
	if (!myRequestRecv(socket, buffer, bufferSize, bodyss)) {
		oss << "Error: recv() error OR request >= 4096 bytes\n";
		threadPrint(oss.str().c_str(), "\n");
		mySocketClose(socket);
		return;
	}
	oss << "HTTP request:\n" << buffer << '\n';

	// process request
	const std::string requestString = buffer;
	std::string method, url, httpVersion;
	if (!parseAndValidateRequest(requestString, method, url, httpVersion)) {
		oss << "Error: Unknown request header\n";
		threadPrint(oss.str().c_str(), "\n");
		mySocketClose(socket);
		return;
	}
	
	// create response
	oss << "Request file path: " << url << '\n';
	if (!createAndSendResponse(socket, path + url, httpVersion, constants::EMPTY_STRING, 0L)) {
		oss << "Error: file IO OR send()\n";
		threadPrint(oss.str().c_str(), "\n");
		mySocketClose(socket);
		return;
	}

	// close socket
	if (!mySocketClose(socket)) {
		oss << "Error: socket close()\n";
	}
	
	threadPrint(oss.str().c_str(), "--- httpGetHandler thread end\n\n");
}

void acceptLoop(int socket, std::string &monitorPath, int port) {
	int clientSocket;
	struct sockaddr_in clientAddress;
	socklen_t addressSize = sizeof(clientAddress);

	threadPrint("Waiting connection... on port ", std::to_string(port).c_str());
	while (true) {

		// accept new socket
		memset((char *) &clientAddress, 0, addressSize);
		clientSocket = accept(socket, (struct sockaddr *) &clientAddress, &addressSize);
		if (clientSocket == -1) {
			perror("accept()");
			continue;
		}

		// new thread to handle the GET request
		std::thread httpGetHandlerThread(httpGetHandler, clientSocket, std::cref(monitorPath));
		httpGetHandlerThread.detach();
	}
}

int main(int argc, char *argv[]) {

	// check argument
	if (argc < 3) {
		// std::cout << "Usage: mc.exe [sync folder path] [refresh interval in ms] [server IP] [server port] [client listen port]" << std::endl;
		std::cout << "Usage: mc.exe [sync folder path] [config file path]" << std::endl;
		exit(EXIT_FAILURE);
	}

	// get argument
	std::string monitorPath(argv[1]);
	// int refreshInterval = atoi(argv[2]);
	// char *serverIP = "127.0.0.1";
	// int serverPort = 4180;
	// int listenPort = 4181;

	// open config file
	std::string line;
	std::ifstream configFile(argv[2]);
	std::getline(configFile, line);
	std::istringstream iss(line);
	iss >> line;
	int refreshInterval = stoi(line);
	iss >> line;
	char serverIP[16];
	strncpy(serverIP, line.c_str(), 16);
	iss >> line;
	int serverPort = stoi(line);
	iss >> line;
	int listenPort = stoi(line);
	configFile.close();

	// constants
	const Protocol protocol = TCP;

	/*|=======================================================|*/
	/*|            Connect to server and monitor              |*/
	/*|=======================================================|*/
	struct sockaddr_in serverAddress;
	int serverSocket = getConnectSocket(serverIP, serverPort, protocol, &serverAddress);
	// file monitoring thread
	std::thread monitorThread(monitorFile, serverSocket, std::cref(monitorPath), refreshInterval, listenPort);

	/*|=======================================================|*/
	/*|          Listen to GET request from server            |*/
	/*|=======================================================|*/
	#ifdef WIN32
		WSADATA wsa;
		initWinsock(&wsa);
	#endif
	struct sockaddr_in listenAddress;
	int listenSocket = getListenSocket(NULL, listenPort, protocol, &listenAddress);
	// accept loop
	acceptLoop(listenSocket, monitorPath, listenPort);
	
	#ifdef WIN32
		WSACleanup();
	#endif

	threadPrint("Client is KO ed...", "");
	return 0;
}
