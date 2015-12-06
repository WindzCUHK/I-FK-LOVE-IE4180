#include "myHead.h"

// private methods
void getFileTime(time_t *timeKey, struct stat *fileAttributes) {

	time_t ctime = fileAttributes->st_ctime;
	time_t mtime = fileAttributes->st_mtime;

	if (difftime(ctime, mtime) > 0) {
		*timeKey = ctime;
	} else {
		*timeKey = mtime;
	}
}

void initFileMeta(FileMeta *meta, const char *path, size_t filenameLen) {

	// get file stat
	struct stat fileAttributes;
	stat(path, &fileAttributes);

	// get time
	time_t timeKey;
	getFileTime(&timeKey, &fileAttributes);

	// get isDir
	bool isDir = ((S_ISDIR(fileAttributes.st_mode)) ? true : false);

	// assign data
	strncpy(meta->path, path, PATH_MAX);
	meta->filenameLen = filenameLen;
	meta->timeKey = timeKey;
	meta->isDir = isDir;
}

// function pointer: bool (*cmp_fm_pt) (FileMeta &, FileMeta &) = cmpFileMeta;
bool cmpFileMeta(const FileMeta &a, const FileMeta &b) {

	// return true, if the first argument is less than (i.e. is ordered before) the second.

	int pathResult = strncmp(a.path, b.path, PATH_MAX);
	if (pathResult < 0) {
		// std::cout << a.path << "\" < \"" << b.path << "\"" << std::endl;
		return true;
	}
	if (pathResult > 0) {
		// std::cout << a.path << "\" > \"" << b.path << "\"" << std::endl;
		return false;
	}

	int timeDiff = (int) difftime(a.timeKey, b.timeKey);	// b - a
	if (timeDiff > 0) return false;
	if (timeDiff < 0) return true;

	if (a.isDir != b.isDir) return true;
	else return false;
}
bool cmpFileMetaPathOnly(const FileMeta &a, const FileMeta &b) {
	if (strncmp(a.path, b.path, PATH_MAX) < 0) return true;
	else return false;
}

// bool isEqualFileMeta(const FileMeta &a, const FileMeta &b) {
// 	if (
// 		(strcmp(a.path, b.path) == 0) &&
// 		(difftime(a.timeKey, b.timeKey) == 0) &&
// 		(a.isDir == b.isDir)
// 	) return true;
// 	else return false;
// }

// bool isEqualPath(const FileMeta &a, const FileMeta &b) {
// 	if (strcmp(a.path, b.path) == 0) return true;
// 	else return false;
// }

void printFileMeta(FileMeta &meta) {
	printf("{isDir: %d, timeKey: %lld, path: \"%s\"}\n", meta.isDir, (long long) meta.timeKey, meta.path);
}

int listAllFilesInDir(std::vector<FileMeta> &fileMetas, std::string &rootDirPath) {

	// memory
	FileMeta meta;

	// pointers
	DIR *dir;
	struct dirent *ent;

	// objects
	std::string dirPath = rootDirPath + constants::global_pathDelimiter;

	if ((dir = opendir(rootDirPath.c_str())) == NULL) {
		perror("listAllFiles() => opendir()");
		return EXIT_FAILURE;
	} else {
		while ((ent = readdir(dir)) != NULL) {

			// skip '.' file
			std::string fileName(ent->d_name);
			if (fileName.at(0) == HIDDEN_FILE) continue;

			// create file path and check file stat
			std::string filePath = dirPath + fileName;
			// std::cout << filePath << std::endl;
			initFileMeta(&meta, filePath.c_str(), fileName.length());

			// append file, if needed, read directory
			fileMetas.push_back(meta);
			if (meta.isDir) {
				if (listAllFilesInDir(fileMetas, filePath) == EXIT_FAILURE) {
					return EXIT_FAILURE;
				}
			}
		}
		closedir(dir);
	}

	return EXIT_SUCCESS;
}

void getFileNameFromPath(std::string &filname, char *path, short fileNameLen) {
	size_t pathLen = strlen(path);
	std::string str(path);
	filname = str.substr(pathLen - fileNameLen, fileNameLen);
}
