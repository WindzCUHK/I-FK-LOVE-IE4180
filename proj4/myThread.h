#ifndef _MY_THREAD_H_
#define _MY_THREAD_H_

// display mutex
extern std::mutex g_display_mutex;

void threadPrint(const char *str1, const char *str2);


// job queue and its mutex
// queue<ConnectInfo> jobQueue;
// mutex job_q_mutex;
// condition_variable hasNewJob;


#endif
