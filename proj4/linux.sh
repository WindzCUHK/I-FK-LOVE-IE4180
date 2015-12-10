#!/bin/bash

# alias ll="ls -l"
g++ -Wall -Wno-write-strings -std=c++11 -pthread -I. cgicc/CgiUtils.cpp connect.cpp customHTTP.cpp fileMeta.cpp threadUtil.cpp monitorClient.cpp -o mc.exe
g++ -Wall -Wno-write-strings -std=c++11 -pthread -I. cgicc/CgiUtils.cpp connect.cpp customHTTP.cpp fileMeta.cpp threadUtil.cpp server.cpp -o server.exe
g++ -Wall -Wno-write-strings -std=c++11 -pthread -I. cgicc/CgiUtils.cpp connect.cpp customHTTP.cpp fileMeta.cpp restoreClient.cpp -o rc.exe
# ./mc.exe new 10000 | less

# ./rc.exe ./ 127.0.0.1 4180
# ./server.exe new 4180

# g++ -Wall -Wno-write-strings -std=c++11 -pthread -I. cgicc/CgiUtils.cpp tt.cpp -o NetProbe



# not implemented
# NetProbeServer -dir [directory]
# NetProbeClient -dir [directory] -config [config_f]
