#!/bin/bash

# alias ll="ls -l"
g++ -Wall -Wno-write-strings -std=c++11 -pthread -I. *.cpp -o NetProbe

