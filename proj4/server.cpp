#include "myHead.h"

std::mutex g_display_mutex;
std::mutex fs_changing_mutex;

void donwloadFileHandler(struct sockaddr_in address, FileMeta fm, const std::string &monitorPath) {

	std::ofstream ofs;
	std::ostringstream oss;
	std::string url(fm.path);

	oss << "--- donwloadFileHandler start\n";

	// connect to client
	int clientSocket = getConnectSocketByAddress(TCP, &address);

	// send GET request to client
	oss << "start send GET request\n";
	if (!createAndSendRequest(clientSocket, true, url, constants::REQUEST_default_http_version, false, NULL, 0)) {
		oss << "Error createAndSendRequest(): GET " << url;
		threadPrint(oss.str().c_str(), "\n");
		mySocketClose(clientSocket);
		return;
	}

	// write response content to file
	oss << "open file: " << monitorPath + fm.path << "\n";
	ofs.open(monitorPath + fm.path);
	oss << "waiting for response\n";
	if (!myResponseRecv(clientSocket, ofs)) {
		oss << "Error myResponseRecv(): GET " << url;
		threadPrint(oss.str().c_str(), "\n");
		mySocketClose(clientSocket);
		return;
	}

	// close socket and file
	mySocketClose(clientSocket);
	ofs.close();

	// set file modification time  as client
	oss << "change file modification date\n";
	std::string serverFilePath = monitorPath + fm.path;
	if (!setFileTime(&(fm.timeKey), serverFilePath.c_str())) {
		oss << "Error setFileTime(): GET " << url;
		threadPrint(oss.str().c_str(), "\n");
		return;
	}

	oss << "--- donwloadFileHandler end\n";
	threadPrint(oss.str().c_str(), "\n");
}

void myCreate(std::vector<FileMeta> &v, const std::string &monitorPath, struct sockaddr_in *address) {

	std::vector<std::thread> donwloadFileHandlerThreads;

	for (std::vector<FileMeta>::iterator it = v.begin(); it != v.end(); ++it) {
		// for Dir just create, else GET from client
		if (it->isDir) {
			std::string linuxCommand("mkdir -p ");
			system((linuxCommand + monitorPath + it->path).c_str());
		} else {
			// parallel download by threads
			donwloadFileHandlerThreads.emplace_back(donwloadFileHandler, *address, *it, std::cref(monitorPath));
		}
	}

	// wait for all download threads to finish
	for (auto &thread: donwloadFileHandlerThreads) thread.join();
}

void myDelete(std::vector<FileMeta> &v, const std::string &monitorPath) {
	FileMeta localFileMeta;
	for (std::vector<FileMeta>::iterator it = v.begin(); it != v.end(); ++it) {
		// skip directory, don't delete
		if (!it->isDir) {

			// get local FS data
			std::string localFilePath = monitorPath + it->path;
			initFileMeta(&localFileMeta, localFilePath.c_str(), localFilePath.length(), monitorPath.length());

			// other client may update it (should not happen), check before take action
			if (isEqualFileMeta(*it, localFileMeta)) {
				std::size_t folderPathLength = localFilePath.length() - it->filenameLen;
				std::string folderPath = localFilePath.substr(0, folderPathLength);
				std::string newName = HIDDEN_FILE_STR + localFilePath.substr(folderPathLength);
				newName += TIME_DELIMITER + std::to_string((long long) it->timeKey);

				// no delete, only rename
				if (rename(localFilePath.c_str(), (folderPath + newName).c_str()) != 0) {
					perror("Error: myDelete() => rename()");
				}
			} else {
				perror("myDelete(): Unexpected not equal in file meta-data");
			}
			// only update and delete will make this fail
			// both renamed the file, hence save to skip
		}
	}
}

void parsePostBody(std::string bodyString, short *clientListenPort, std::vector<FileMeta> &v, std::ostringstream &oss) {
	std::size_t found = bodyString.find(constants::HTTP_inline_delimiter);
	*clientListenPort = (short) stoi(bodyString.substr(0, found));
	v.clear();

	// nothing change implies empty body
	if ((found + 1) != bodyString.length()) {
		decodeFileMetas(cgicc::form_urldecode(bodyString.substr(found + 1)), v);
	} else {
		oss << "Nothing to be changed~\n";
	}
}

bool createAndSendFileList(bool isRestore, int socket, const std::string &httpVersion, const std::string &monitorPath, std::ostringstream &oss) {

	// generate file list
	std::vector<FileMeta> fileMetas;
	{
		std::lock_guard<std::mutex> guard(fs_changing_mutex);
		if (listAllFilesInDir(fileMetas, monitorPath, monitorPath, isRestore) == EXIT_FAILURE) {
			oss << "Error: Cannot monitor directory!!!\n";
			return false;
		}
	}

	// send file list
	std::string contentString = cgicc::form_urlencode(encodeFileMetas(fileMetas));
	if (!createAndSendResponse(socket, constants::EMPTY_STRING, httpVersion, contentString, contentString.length())) {
		oss << "Error: file IO OR send()\n";
		return false;
	}

	return true;
}

void httpHandler(int socket, struct sockaddr_in address, const std::string &monitorPath) {

	// thread output stream
	std::ostringstream oss, bodyss;
	oss << "--- httpHandler thread start\n";

	// request buffer
	char buffer[BUFFER_SIZE];
	int bufferSize = BUFFER_SIZE;

try {
	do {
		// take out the whole request header
		bodyss.str("");
		if (!myRequestRecv(socket, buffer, bufferSize, bodyss)) {
			oss << "Error: recv() error OR request >= 4096 bytes\n";
			break;
		}
		oss << "HTTP request:\n" << buffer << '\n';
		// debug
		std::cout << "myRequestRecv() => body:\n" << bodyss.str() << std::endl;



		// process request
		const std::string requestString = buffer;
		std::string method, url, httpVersion;
		if (!parseAndValidateRequest(requestString, method, url, httpVersion)) {
			oss << "Error: Unknown request header\n";
			break;
		}

		// create response
		if (method == constants::HTTP_POST) {

			// variables for parse POST body
			std::vector<FileMeta> diffVector;
			std::string bodyString = bodyss.str();
			short clientListenPort = 0;

			// lock before /delete, unlock after /create

			// POST /delete
			if (url.compare(constants::SERVER_delete_path) == 0) {
				std::cout << "```POST /delete\n";

				// lock
				fs_changing_mutex.lock();

				// take delete action
				parsePostBody(bodyString, &clientListenPort, diffVector, oss);
				myDelete(diffVector, monitorPath);

				// send 200 OK
				if (!createAndSendOK(socket, httpVersion)) {
					oss << "Error: createAndSendOK()\n";
					fs_changing_mutex.unlock();
					break;
				}
			}
			// POST /update
			if (url.compare(constants::SERVER_update_path) == 0) {
				std::cout << "```POST /update\n";

				// take update action
				parsePostBody(bodyString, &clientListenPort, diffVector, oss);
				myDelete(diffVector, monitorPath);
				address.sin_port = htons(clientListenPort);
				myCreate(diffVector, monitorPath, &address);

				// send 200 OK
				if (!createAndSendOK(socket, httpVersion)) {
					oss << "Error: createAndSendOK()\n";
					fs_changing_mutex.unlock();
					break;
				}
			}
			// POST /create
			if (url.compare(constants::SERVER_create_path) == 0) {
				std::cout << "```POST /create\n";

				// take create action
				parsePostBody(bodyString, &clientListenPort, diffVector, oss);
				address.sin_port = htons(clientListenPort);
				myCreate(diffVector, monitorPath, &address);

				// send 200 OK
				if (!createAndSendOK(socket, httpVersion)) {
					oss << "Error: createAndSendOK()\n";
					fs_changing_mutex.unlock();
					break;
				}

				fs_changing_mutex.unlock();
			}

		} else if (method == constants::HTTP_GET) {

			if (url.compare(constants::SERVER_list_path) == 0) {
				std::cout << "```GET /list\n";

				if (!createAndSendFileList(false, socket, httpVersion, monitorPath, oss)) {
					oss << "Error: createAndSendFileList(false)\n";
					break;
				}
			} else if (url.compare(constants::SERVER_restore_list_path) == 0) {
				std::cout << "```GET /restoreList\n";

				if (!createAndSendFileList(true, socket, httpVersion, monitorPath, oss)) {
					oss << "Error: createAndSendFileList(true)\n";
					break;
				}
			} else if ((url.length() > constants::SERVER_restore_path.length()) && (url.compare(0, constants::SERVER_restore_path.length(), constants::SERVER_restore_path) == 0)) {
				std::cout << "```GET /restore/(.+)\n";

				std::string filePath = url.substr(constants::SERVER_restore_path.length());
				if (!createAndSendResponse(socket, monitorPath + filePath, httpVersion, constants::EMPTY_STRING, 0L)) {
					oss << "Error: file IO OR send()\n";
					break;
				} else break;
			}

		} else {
			oss << "Error: Unknown request method\n";
			break;
		}

		// loop ending
		oss << "=== httpHandler thread loop ending\n";
		threadPrint(oss.str().c_str(), "\n");
		oss.str("");

	} while (true);

} catch (const std::exception& exc) {
	std::cerr << exc.what();
}


	// thread ending
	oss << "--- httpHandler thread end\n";
	threadPrint(oss.str().c_str(), "\n");
	mySocketClose(socket);
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

		// new thread to handle the POST request
		std::thread httpHandlerThread(httpHandler, clientSocket, clientAddress, std::cref(monitorPath));
		httpHandlerThread.detach();
	}
}

int main(int argc, char *argv[]) {

	// check argument
	if (argc < 3) {
		std::cout << "Usage: server.exe [sync folder path] [server listen port]" << std::endl;
		exit(EXIT_FAILURE);
	}

	// get argument
	std::string monitorPath(argv[1]);
	int listenPort = atoi(argv[2]);

	// constants
	const Protocol protocol = TCP;
	#ifdef WIN32
		WSADATA wsa;
		initWinsock(&wsa);
	#endif

	/*|=======================================================|*/
	/*|             Listen to request from client             |*/
	/*|=======================================================|*/
	struct sockaddr_in listenAddress;
	int listenSocket = getListenSocket(NULL, listenPort, protocol, &listenAddress);
	// accept loop
	acceptLoop(listenSocket, monitorPath, listenPort);

	threadPrint("server is KO ed...", "");
	return 0;
}

