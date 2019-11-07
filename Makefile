IP = 10.1.2.124

all: SelectServer.o TCPClient.o

SelectServer.o: SelectServer.cpp
	g++ -std=c++11 SelectServer.cpp -o SelectServer

TCPClient.o: TCPClient.cpp
	g++ -std=c++11 TCPClient.cpp -o TCPClient

client:
	./TCPClient $(IP) 3333

server:
	./SelectServer 3333
