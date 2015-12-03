#ifndef _MODEL_H_
#define _MODEL_H_

// constants
#define BUFFER_SIZE 4096
#define HIDDEN_FILE '.'
#ifdef WIN32
	#define PATH_DELIMITER "\\"
#else
	#define PATH_DELIMITER "/"
#endif

namespace constants {
	const std::string global_pathDelimiter(PATH_DELIMITER);
}

// for connection
enum Protocol {
	TCP = 0,
	UDP = 1
};

typedef struct _ConnectInfo {
	int socket;
	struct sockaddr_in address;
} ConnectInfo;

// for files
typedef struct _FileMeta {
	const char *path;
	time_t timeKey;
	bool isDir;
} FileMeta;

typedef std::deque<FileMeta> fileList;


// method on model
void initFileMeta(FileMeta *meta, char *path);
bool cmpFileMeta(FileMeta &a, FileMeta &b);
void printFileMeta(FileMeta &meta);
int listAllFilesInDir(std::vector<FileMeta> &fileMetas, std::string &rootDirPath);

#endif
