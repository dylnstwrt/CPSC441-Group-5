IP = 127.0.0.1

all: SelectServer.o TCPClient.o

SelectServer.o: SelectServer.cpp
	g++ SelectServer.cpp -o SelectServer

TCPClient.o: TCPClient.cpp
	g++ TCPClient.cpp -o TCPClient

client:
	./TCPClient $(IP) 9000

server:
	./SelectServer 9000
