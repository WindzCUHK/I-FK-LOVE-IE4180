#include "myHead.h"

using namespace std;

int main(int argc, char *argv[]) {

	std::string monitorPath(argv[1]);

	std::vector<FileMeta> _fileMetas;
	std::vector<FileMeta> _oldFileMetas;
	std::vector<FileMeta> &fileMetas = _fileMetas;
	std::vector<FileMeta> &oldFileMetas = _oldFileMetas;
	std::vector<FileMeta> file_intersection, old_difference, current_difference;
	std::vector<FileMeta> update_list, delete_list, create_list;

	std::vector<FileMeta>::size_type oldSize, currentSize, minSize, offset4old, offset4current;

	// init the list in start up
	if (listAllFilesInDir(oldFileMetas, monitorPath) == EXIT_FAILURE) {
		exit(EXIT_FAILURE);
	}
	sort(oldFileMetas.begin(), oldFileMetas.end(), cmpFileMeta);

	while (true) {

		cout << "=== waiting..." << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));

		cerr << "--- loop start..." << endl;

		// initialize vectors
		fileMetas.clear();
		// create file list
		listAllFilesInDir(fileMetas, monitorPath);
		// sort file list
		sort(fileMetas.begin(), fileMetas.end(), cmpFileMeta);

		// cout << "1. " << "old list" << endl;
		// for_each(oldFileMetas.begin(), oldFileMetas.end(), printFileMeta);
		// cout << "1. " << "new list" << endl;
		// for_each(fileMetas.begin(), fileMetas.end(), printFileMeta);
		// exit(0);

		// compare file hierarchy change
		file_intersection.clear();
		std::set_intersection(
			oldFileMetas.begin(),
			oldFileMetas.end(),
			fileMetas.begin(),
			fileMetas.end(),
			std::back_inserter(file_intersection),
			cmpFileMeta
		);

		// get deleted file list
		old_difference.clear();
		std::set_difference(
			oldFileMetas.begin(),
			oldFileMetas.end(),
			file_intersection.begin(),
			file_intersection.end(),
			std::back_inserter(old_difference),
			cmpFileMeta
		);

		// get created or changed file list
		current_difference.clear();
		std::set_difference(
			fileMetas.begin(),
			fileMetas.end(),
			file_intersection.begin(),
			file_intersection.end(),
			std::back_inserter(current_difference),
			cmpFileMeta
		);

		// get update list (path exist in both list)
		update_list.clear();
		std::set_intersection(
			current_difference.begin(),
			current_difference.end(),
			old_difference.begin(),
			old_difference.end(),
			std::back_inserter(update_list),
			cmpFileMetaPathOnly
		);
		delete_list.clear();
		std::set_difference(
			old_difference.begin(),
			old_difference.end(),
			update_list.begin(),
			update_list.end(),
			std::back_inserter(delete_list),
			cmpFileMetaPathOnly
		);
		create_list.clear();
		std::set_difference(
			current_difference.begin(),
			current_difference.end(),
			update_list.begin(),
			update_list.end(),
			std::back_inserter(create_list),
			cmpFileMetaPathOnly
		);

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

		// swap current file list as old, clear the old one
		if (old_difference.size() > 0 || current_difference.size() > 0) {
			oldFileMetas.clear();
			std::vector<FileMeta> &tmp = oldFileMetas;
			oldFileMetas = fileMetas;
			fileMetas = tmp;
		}
	}

	return 0;
}
