#include "myHead.h"

using namespace std;

int main() {

	char x = 1;
	char y = 20;
	ostringstream oss;
	oss << "abc" << x << 123 << y << "xxx";
	cout << oss.str().length() << endl;
	cout << oss.str().c_str() << endl;

	// string content("asd++++fasdfdsg|@#$%%^&*() ending");
	// cout <<  cgicc::form_urlencode(content) << endl;
	// cout <<  cgicc::form_urldecode(content) << endl;

	return 0;
}
