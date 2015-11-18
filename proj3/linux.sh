#!/bin/bash

g++ -Wall -Wno-write-strings -std=c++11 -pthread *.cpp -o NetProbe

cmd="./NetProbe s 41801 o > out.txt"
echo 'Starting: ' $cmd
$cmd
echo ''
