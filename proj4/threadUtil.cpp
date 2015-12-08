#include "myHead.h"
#include "myThread.h"

using namespace std;

void threadPrint(const char *str1, const char *str2) {
	lock_guard<mutex> guarded_lock(g_display_mutex);
	cout << '[' << this_thread::get_id() << "] " << str1 << str2 << endl;
}
