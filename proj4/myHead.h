#ifndef _MY_HEAD_H_
#define _MY_HEAD_H_

// common c libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>
#include <ctime>

// common cpp libraries
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <queue>
#include <vector>
#include <algorithm>
#include <string>

// thread libraries
#include <condition_variable>
#include <thread>
#include <mutex>
#include <chrono>

// file meta library
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
	#include "dirent/dirent.h"
#else
	#include <dirent.h>
#endif

// 3rd party
#include "cgicc/CgiDefs.h"
#include "cgicc/Cgicc.h"
#include "cgicc/CgiUtils.h"

// custom header
#include "model.h"
#include "myConnect.h"
#include "myThread.h"


namespace constants {
	const std::string EMPTY_STRING("");
}

#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
