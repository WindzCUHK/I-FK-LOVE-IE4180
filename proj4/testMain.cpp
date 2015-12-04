#include "myHead.h"

using namespace std;

int main(int argc, char *argv[]) {

	std::vector<FileMeta> fileMetas;
	std::string monitorPath(argv[1]);

	listAllFilesInDir(fileMetas, monitorPath);

	for_each(fileMetas.begin(), fileMetas.end(), printFileMeta);

	return 0;
}
