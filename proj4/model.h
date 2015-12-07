#ifndef _MODEL_H_
#define _MODEL_H_

// constants
#define BUFFER_SIZE 4096
#define HIDDEN_FILE '.'
#define PATH_MAX 4096
#ifdef WIN32
	#define PATH_DELIMITER "\\"
#else
	#define PATH_DELIMITER "/"
#endif

namespace constants {
	const std::string global_pathDelimiter(PATH_DELIMITER);
}

// for files
typedef struct _FileMeta {
	char path[PATH_MAX];
	short filenameLen;
	time_t timeKey;
	bool isDir;
} FileMeta;

typedef std::deque<FileMeta> fileList;


// method on model
void initFileMeta(FileMeta *meta, const char *path, size_t filenameLen);
bool cmpFileMeta(const FileMeta &a, const FileMeta &b);
bool cmpFileMetaPathOnly(const FileMeta &a, const FileMeta &b);
void printFileMeta(FileMeta &meta);
int listAllFilesInDir(std::vector<FileMeta> &fileMetas, std::string &rootDirPath);
void getFileNameFromPath(std::string &filname, char *path, short fileNameLen);
// serialization 
std::string encodeFileMetas(std::vector<FileMeta> v);
void decodeFileMetas(const std::string &data, std::vector<FileMeta> &v);

#endif
