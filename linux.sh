g++ -Wall -Wno-write-strings -std=c++11 -pthread server.cpp ./common/*.cpp -o a.out
g++ -Wall -Wno-write-strings -std=c++11 -pthread client.cpp ./common/*.cpp -o b.out
