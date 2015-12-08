#!/bin/bash

# alias ll="ls -l"
g++ -Wall -Wno-write-strings -std=c++11 -pthread -I. cgicc/CgiUtils.cpp connect.cpp customHTTP.cpp fileMeta.cpp threadUtil.cpp monitorClient.cpp -o mc.exe
# ./mc.exe new 10000 | less

# g++ -Wall -Wno-write-strings -std=c++11 -pthread -I. cgicc/CgiUtils.cpp tt.cpp -o NetProbe
