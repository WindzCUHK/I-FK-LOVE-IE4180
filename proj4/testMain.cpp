#include "myHead.h"

using namespace std;

int main(int argc, char *argv[]) {

	std::vector<FileMeta> fileMetas;
	std::string monitorPath(argv[1]);

	listAllFilesInDir(fileMetas, monitorPath);

	// print vector

	return 0;
}
