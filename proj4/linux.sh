#!/bin/bash

# alias ll="ls -l"
g++ -Wall -Wno-write-strings -std=c++11 -pthread -I. cgicc/CgiUtils.cpp *.cpp -o NetProbe
./NetProbe new | less

# g++ -Wall -Wno-write-strings -std=c++11 -pthread -I. cgicc/CgiUtils.cpp tt.cpp -o NetProbe
