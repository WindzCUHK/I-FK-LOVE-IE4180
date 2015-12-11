#ifndef _MODEL_H_
#define _MODEL_H_

// file time
#ifdef WIN32
	#include <sys/utime.h>
#else
	#include <utime.h>
#endif

// constants
#define BUFFER_SIZE 4096
#define PATH_MAX 4096
#ifdef WIN32
	#define PATH_DELIMITER "\\"
#else
	#define PATH_DELIMITER "/"
#endif
#define HIDDEN_FILE '.'

#define HIDDEN_FILE_STR "."
#define TIME_DELIMITER "_"

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
bool setFileTime(time_t *timeKey, const char *filepath);
void initFileMeta(FileMeta *meta, const char *path, size_t filenameLen, size_t rootDirPathOffset);
bool cmpFileMeta(const FileMeta &a, const FileMeta &b);
bool cmpFileMetaPathOnly(const FileMeta &a, const FileMeta &b);
bool isEqualFileMeta(const FileMeta &a, const FileMeta &b);
void printFileMeta(FileMeta &meta);
int listAllFilesInDir(std::vector<FileMeta> &fileMetas, const std::string &rootDirPath, const std::string &dirPath, bool needHidden);
void getFileNameFromPath(std::string &filname, char *path, short fileNameLen);
// serialization 
std::string encodeFileMetas(std::vector<FileMeta> v);
void decodeFileMetas(const std::string &data, std::vector<FileMeta> &v);

#endif
