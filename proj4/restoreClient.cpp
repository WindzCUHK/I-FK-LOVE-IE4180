#include "myHead.h"
#include <map>

using namespace std;

string getOriginalPath(const string &path, short fileNameLength) {
	size_t folderPathLength = path.length() - fileNameLength;
	string filename = path.substr(folderPathLength);
	
	if (filename.at(0) != HIDDEN_FILE) {
		return path;
	} else {
		string folderPath = path.substr(0, folderPathLength);

		size_t timePosition = filename.find_last_of(TIME_DELIMITER);
		string originalName = filename.substr(1, timePosition - 1);

		return folderPath + originalName;
	}
}

int main(int argc, char *argv[]) {

	// check argument
	if (argc < 4) {
		std::cout << "Usage: restore.exe [restore folder path] [server IP] [server port]" << std::endl;
		exit(EXIT_FAILURE);
	}

	// get argument
	std::string restorePath(argv[1]);
	char *serverIP = argv[2];
	int serverPort = atoi(argv[3]);

	// connect to server
	#ifdef WIN32
		WSADATA wsa;
		initWinsock(&wsa);
	#endif
	struct sockaddr_in serverAddress;
	int serverSocket = getConnectSocket(serverIP, serverPort, TCP, &serverAddress);

	// get list from server
	if (!createAndSendRequest(serverSocket, true, constants::SERVER_restore_list_path, constants::REQUEST_default_http_version, true, NULL, 0)) {
		mySocketClose(serverSocket);
		myDied("Error createAndSendRequest(): sending GET /restoreList request\n");
	}
	cout << "sent - GET /restoreList" << endl;

	// wait for server response
	ostringstream bodyss;
	vector<FileMeta> serverFileMetas;
	cout << "waiting for GET response ..." << endl;
	if (!myResponseRecv(serverSocket, bodyss)) {
		mySocketClose(serverSocket);
		myDied("Error myResponseRecv(): receiving GET /restoreList response\n");
	}
	decodeFileMetas(cgicc::form_urldecode(bodyss.str()), serverFileMetas);
	sort(serverFileMetas.begin(), serverFileMetas.end(), cmpFileMeta);
	cout << "got - GET /restoreList" << endl;

	// debug
	for (auto &fm: serverFileMetas) {printFileMeta(fm);}

	// parse file list
	vector<string> pathKeys;
	map<string, map<time_t, FileMeta>> fileMap;

	map<string, map<time_t, FileMeta>>::iterator path_map_it;
	map<time_t, FileMeta>::iterator time_map_it;

	for (auto &fm: serverFileMetas) {

		// skip directories
		if (!fm.isDir) {
			string displayPathName = getOriginalPath(fm.path, fm.filenameLen);

			// if no key find, insert key
			path_map_it = fileMap.find(displayPathName);
			if (path_map_it == fileMap.end()) {
				pathKeys.push_back(displayPathName);
			}

			// insert to map
			fileMap[displayPathName][fm.timeKey] = fm;
		}
	}

	// display restore file list
	cout << "File can be restored:\n";
	for (vector<string>::size_type i = 0; i != pathKeys.size(); i++) {
		cout << "  " << i << "\t - " << pathKeys[i] << '\n';
	}

	// get user input
	int pathIndex = 0;
	cout << "Please input the file index you want to restore: ";
	cin >> pathIndex;

	// checking user input
	if (pathIndex < 0 || ((int) pathKeys.size()) < pathIndex) {
		mySocketClose(serverSocket);
		cout << "Invalid index. Got problems on your eyes? Check them first before u come back." << endl;
		exit(EXIT_SUCCESS);
	}

	// display file versions
	vector<time_t> timeKeys;
	string selectedPath = pathKeys[pathIndex];
	cout << "Version can be restored for " << selectedPath << ":\n";
	int i = 0;
	const size_t timePrintingBufferSize = 20;
	char timePrintingBuffer[timePrintingBufferSize];
	for (auto &timeMapPair: fileMap[selectedPath]) {
		timeKeys.push_back(timeMapPair.first);
		strftime(timePrintingBuffer, timePrintingBufferSize, "%Y-%m-%d %H:%M:%S", localtime(&(timeMapPair.first)));
		cout << "  " << i << "\t - " << timePrintingBuffer << '\n';
		i++;
	}

	// get user input again
	int timeIndex = 0;
	cout << "Please input the timestamps index of version you want to restore: ";
	cin >> timeIndex;

	// checking user input again
	if (timeIndex < 0 || ((int) timeKeys.size()) < timeIndex) {
		mySocketClose(serverSocket);
		cout << "Invalid index. Got problems on your hand? Handicap-proof is difficult." << endl;
		exit(EXIT_SUCCESS);
	}

	// selected file
	FileMeta *selectedFile = &(fileMap[selectedPath][timeKeys[timeIndex]]);

	// get file from server
	if (!createAndSendRequest(serverSocket, true, constants::SERVER_restore_path + selectedFile->path, constants::REQUEST_default_http_version, false, NULL, 0)) {
		mySocketClose(serverSocket);
		myDied("Error createAndSendRequest(): sending GET /restore request\n");
	}
	cout << "sent - GET /restore" << endl;

	// pre-process file path
	string restoreFilePath = restorePath;
	char *path_delimiter = PATH_DELIMITER;
	// append '/' at the restore folder path
	if (((char) restoreFilePath.back()) != path_delimiter[0]) restoreFilePath += path_delimiter;
	// get file name from selected path
	size_t fileNamePosition = selectedPath.find_last_of(PATH_DELIMITER);
	restoreFilePath += selectedPath.substr(fileNamePosition + 1);

	// wait for server response
	ofstream ofs;
	ofs.open(restoreFilePath);
	if (!myResponseRecv(serverSocket, ofs)) {
		mySocketClose(serverSocket);
		myDied("Error myResponseRecv(): receiving GET /restore response\n");
	}
	ofs.close();
	cout << "got - GET /restore" << endl;

	// ending
	cout << "File restore to " << restoreFilePath << endl;
	cout << "Don't delete it again~" << endl;
	mySocketClose(serverSocket);

	#ifdef WIN32
		WSACleanup();
	#endif

	return 0;
}
