# TODO

1. search for `/* Alvin's part */`
2. compile in windows, test with a browser
3. linux not tested, cant access CSE linux with a browser...
4. remove all debug print, before the performance test

#### Compile

	g++ -Wall -Wno-write-strings -std=c++11 -pthread *.cpp -o NetProbe

#### File list

1. html
	- all request content should be under it
	- GET /index.html = ./html/index.html
2. linux.sh
	- compile and run in linux
3. getTest.txt
	- sample GET reuqest for testing in linux

#### Usage

	NetProbe s [port] [thread_model] [thread_num]

1. s: server
2. port: server listen port
3. thread_model
	- o: on-demand model
	- p: thread-pool model
4. thread_num

#### Sample run

	./NetProbe s 41801 o > out.txt
	./NetProbe s 41801 p 10 > out.txt

#### testing in linux

	# nc connect to server
	cat getTest.txt | nc 127.0.0.1 41801

	# GET request format
	GET http://127.0.0.1:41801/index.html HTTP/1.1 
	User-Agent: curl/7.26.0 
	Host: 127.0.0.1:41800 
	Accept: */* 
	Proxy-Connection: Keep-Alive 

	# GET request string version
	"GET http://abc.com/index.html HTTP/1.1\r\nUser-Agent: curl/7.26.0\r\nHost: 127.0.0.1:41800\r\nAccept: */*\r\nProxy-Connection: Keep-Alive\r\n\r\n"
