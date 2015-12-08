#include "myHead.h"

mutex g_display_mutex;

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


	// cout << "1. " << "old list" << endl;
	// for_each(oldFileMetas.begin(), oldFileMetas.end(), printFileMeta);
	// cout << "1. " << "new list" << endl;
	// for_each(fileMetas.begin(), fileMetas.end(), printFileMeta);
	// cout << "2. " << "unchanged" << endl;
	// for_each(file_intersection.begin(), file_intersection.end(), printFileMeta);
	// cout << "2.1. " << "old diff" << endl;
	// for_each(old_difference.begin(), old_difference.end(), printFileMeta);
	// cout << "2.2. " << "cur diff" << endl;
	// for_each(current_difference.begin(), current_difference.end(), printFileMeta);
	// cout << "3. " << "update" << endl;
	// for_each(update_list.begin(), update_list.end(), printFileMeta);
	// cout << "4. " << "delete" << endl;
	// for_each(delete_list.begin(), delete_list.end(), printFileMeta);
	// cout << "5. " << "create" << endl;
	// for_each(create_list.begin(), create_list.end(), printFileMeta);

	// clean up
	file_intersection.clear();
	old_difference.clear();
	current_difference.clear();
}

void monitorFile(int socket, const std::string &monitorPath, int refreshInterval) {

	// thread output stream
	std::ostringstream oss, bodyss;
	threadPrint("--- monitorFile thread start", "\n");

	// variables
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
		if (listAllFilesInDir(localfileMetas, monitorPath) == EXIT_FAILURE) {
			myDied("Cannot monitor directory!!!");
		}
		sort(localfileMetas.begin(), localfileMetas.end(), cmpFileMeta);
		oss << "create local file list\n";

		// difference between server and client
		filesDifference(serverFileMetas, localfileMetas, update_list, delete_list, create_list);
		oss << "diff local and server\n";

		// POST changes to server, HTTP pipeline
		std::string &contentStringRef, httpVersionRef = contants::REQUEST_default_http_version;
		oss << "POST /delete\n";
		contentStringRef = form_urlencode(encodeFileMetas(delete_list));
		if (!createAndSendRequest(socket, false, constants::SERVER_delete_path, httpVersionRef, true, contentStringRef.c_str(), contentStringRef.length())) {
			oss << "Error createAndSendRequest(): sending POST /delete request\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}
		oss << "POST /create\n";
		contentStringRef = form_urlencode(encodeFileMetas(create_list));
		if (!createAndSendRequest(socket, false, constants::SERVER_create_path, httpVersionRef, true, contentStringRef.c_str(), contentStringRef.length())) {
			oss << "Error createAndSendRequest(): sending POST /create request\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}
		oss << "POST /update\n";
		contentStringRef = form_urlencode(encodeFileMetas(update_list));
		if (!createAndSendRequest(socket, false, constants::SERVER_udpate_path, httpVersionRef, true, contentStringRef.c_str(), contentStringRef.length())) {
			oss << "Error createAndSendRequest(): sending POST /update request\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}


		if (!myResponseRecv(socket, bodyss)) {
			oss << "Error myResponseRecv(): receiving POST /delete response\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}
		if (!myResponseRecv(socket, bodyss)) {
			oss << "Error myResponseRecv(): receiving POST /create response\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}
		if (!myResponseRecv(socket, bodyss)) {
			oss << "Error myResponseRecv(): receiving POST /update response\n";
			threadPrint(oss.str().c_str(), "\n");
			mySocketClose(socket);
			return;
		}
		oss << "POST response DONE\n";

		// clear up vertors
		localfileMetas.clear();
		serverFileMetas.clear();
		update_list.clear();
		delete_list.clear();
		create_list.clear();

		// clean up streams
		oss << "=== monitor loop end! Waiting " << refreshInterval << "ms ...\n";
		threadPrint(oss.str().c_str(), "\n");
		oss.clear();
		bodyss.clear();

		// sleep
		std::this_thread::sleep_for(std::chrono::milliseconds(refreshInterval));
	}
}

void httpGetHandler(int socket, const std::string &path) {

	// thread output stream
	std::ostringstream oss;
	oss << '--- httpGetHandler thread start\n';

	// request buffer
	char buffer[BUFFER_SIZE];
	int bufferSize = BUFFER_SIZE;

	// take out the whole request header
	if (!myRequestRecv(socket, buffer, bufferSize, NULL)) {
		oss << "Error: recv() error OR request >= 4096 bytes\n";
		threadPrint(oss.str().c_str(), "\n");
		mySocketClose(socket);
		return;
	}
	oss << "HTTP request:\n" << buffer << '\n';

	// process request
	const std::string requestString = buffer;
	std::string method, url, httpVersion;
	bool isKeepAlive;
	if (!parseAndValidateRequest(requestString, method, url, httpVersion)) {
		oss << "Error: Unknown request header\n";
		threadPrint(oss.str().c_str(), "\n");
		mySocketClose(socket);
		return;
	}

	// create response
	oss << "Request file path: " << url << '\n';
	if (!createAndSendResponse(socket, method, path + url, httpVersion)) {
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

void acceptLoop(int socket, std::string &monitorPath) {
	int clientSocket;
	struct sockaddr_in clientAddress;
	socklen_t addressSize = sizeof(clientAddress);

	threadPrint("Waiting connection... on port ", to_string(port).c_str());
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
	}

	httpGetHandlerThread.join();
}

int main(int argc, char *argv[]) {

	// check argument
	if (argc < 3) {
		cout << "Usage: monitorClient [sync folder path] [refresh interval] [server IP] [server port] [client listen port]" << endl;
		exit(EXIT_FAILURE);
	}

	// get argument
	std::string monitorPath(argv[1]);
	int refreshInterval = atoi(argv[2]);
	char *serverIP = "127.0.0.1";
	int serverPort = 4180;
	int listenPort = 4181;

	// constants
	const Protocol protocol = TCP;

	/*|=======================================================|*/
	/*|            Connect to server and monitor              |*/
	/*|=======================================================|*/
	struct sockaddr_in serverAddress;
	int serverSocket = getConnectSocket(serverIP, serverPort, protocol, &serverAddress);
	// file monitoring thread
	std::thread monitorThread(monitorFile, serverSocket, std::cref(monitorPath), refreshInterval);

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
	acceptLoop(listenSocket, monitorPath);

	threadPrint("Client is KO ed...", "");
	return 0;
}
