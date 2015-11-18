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

	./NetProbe s 41801 o > out.txt &
	./NetProbe s 41801 p 10 > out.txt &
	nc 127.0.0.1 41801

	GET http://127.0.0.1:41801/index.html HTTP/1.1 
	User-Agent: curl/7.26.0 
	Host: 127.0.0.1:41800 
	Accept: */* 
	Proxy-Connection: Keep-Alive 
	"GET http://abc.com/index.html HTTP/1.1\r\nUser-Agent: curl/7.26.0\r\nHost: 127.0.0.1:41800\r\nAccept: */*\r\nProxy-Connection: Keep-Alive\r\n\r\n"
