# TODO

search for `/* Alvin's part */`

# Used libraries

	https://github.com/AndreLouisCaron/httpxx

#### Compile

	g++ -Wall -Wno-write-strings -std=c++11 -pthread *.cpp -o NetProbe

#### Usage

	NetProbe s [port] [thread_model] [thread_num]

1. s: server
2. port: server listen port
3. thread_model
	- o: on-demand model
	- p: thread-pool model
4. thread_num

#### Sample

	./NetProbe s 4180 o > out.txt &
	./NetProbe s 4180 p 10 > out.txt &
	nc 127.0.0.1 41801

	GET / HTTP/1.1 
	User-Agent: curl/7.26.0 
	Host: 127.0.0.1:41800 
	Accept: */* 
	Proxy-Connection: Keep-Alive 
