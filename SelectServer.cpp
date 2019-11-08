/*
 * A simple TCP select server that accepts multiple connections and echo message back to the clients
 * For use in CPSC 441 lectures
 * Instructor: Prof. Mea Wang
 */

 #include <iostream>
 #include <sys/socket.h> // for socket(), connect(), send(), and recv()
 #include <arpa/inet.h>  // for sockaddr_in and inet_addr()
 #include <stdlib.h>     // for atoi() and exit()
 #include <string.h>     // for memset()
 #include <unistd.h>     // for close()
 #include <stdio.h>
 #include <fstream>
 #include <string>
 #include "game.h"

using namespace std;


////////////////////////////////// Server Variables///////////////////////////////////////
const int BUFFERSIZE = 32;   // Size the message buffers
const int MAXPENDING = 10;    // Maximum pending connections

fd_set recvSockSet;   // The set of descriptors for incoming connections
int maxDesc = 0;      // The max descriptor
bool terminated = false;
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////Server Methods//////////////////////////////////////////
void initServer (int&, int port);
void processSockets (fd_set);
void sendData (string, int, char[], int, string);
string receiveData (int, char[], int&, string);
//////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////Game Variables///////////////////////////////////////////
bool gameOver = false;
const int width = 7;
const int heigth = 3;
vector<player> players;
vector<location> pointsTaken;
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////Game Methods////////////////////////////////////////////
void drawGrid(int , int , vector<player> , int , vector<location>);
void initGameState();
//////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    int serverSock;                  // server socket descriptor
    int clientSock;                  // client socket descriptor
    struct sockaddr_in clientAddr;   // address of the client

    struct timeval timeout = {0, 10};  // The timeout value for select()
    struct timeval selectTime;
    fd_set tempRecvSockSet;            // Temp. receive socket set for select()

    // Check for input errors
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " <Listening Port>" << endl;
        exit(1);
    }

    // Initilize the server
    initServer(serverSock, atoi(argv[1]));

    // Clear the socket sets
    FD_ZERO(&recvSockSet);

    // Add the listening socket to the set
    FD_SET(serverSock, &recvSockSet);
    maxDesc = max(maxDesc, serverSock);

    // Run the server until a "terminate" command is received)
    while (!terminated)
    {
        // copy the receive descriptors to the working set
        memcpy(&tempRecvSockSet, &recvSockSet, sizeof(recvSockSet));

        // Select timeout has to be reset every time before select() is
        // called, since select() may update the timeout parameter to
        // indicate how much time was left.
        selectTime = timeout;
        int ready = select(maxDesc + 1, &tempRecvSockSet, NULL, NULL, &selectTime);
        if (ready < 0)
        {
            cout << "select() failed" << endl;
            break;
        }

        // First, process new connection request, if any.
        if (FD_ISSET(serverSock, &tempRecvSockSet))
        {
            // set the size of the client address structure
            unsigned int size = sizeof(clientAddr);

            // Establish a connection
            if ((clientSock = accept(serverSock, (struct sockaddr *) &clientAddr, &size)) < 0)
                break;
            cout << "Accepted a connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << clientAddr.sin_port << endl;

            // Add the new connection to the receive socket set
            FD_SET(clientSock, &recvSockSet);
            maxDesc = max(maxDesc, clientSock);
        }

        // Then process messages waiting at each ready socket
        else
            processSockets(tempRecvSockSet);
    }

    // Close the connections with the client
    for (int sock = 0; sock <= maxDesc; sock++)
    {
        if (FD_ISSET(sock, &recvSockSet))
            close(sock);
    }

    // Close the server sockets
    close(serverSock);
}

void initServer(int& serverSock, int port)
{
    struct sockaddr_in serverAddr;   // address of the server

    // Create a TCP socket
    // * AF_INET: using address family "Internet Protocol address"
    // * SOCK_STREAM: Provides sequenced, reliable, bidirectional, connection-mode byte streams.
    // * IPPROTO_TCP: TCP protocol
    if ((serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        cout << "socket() failed" << endl;
        exit(1);
    }

    // Free up the port before binding
    // * sock: the socket just created
    // * SOL_SOCKET: set the protocol level at the socket level
    // * SO_REUSEADDR: allow reuse of local addresses
    // * &yes: set SO_REUSEADDR on a socket to true (1)
    // * sizeof(int): size of the value pointed by "yes"
    int yes = 1;
    if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
    {
        cout << "setsockopt() failed" << endl;
        exit(1);
    }

    // Initialize the server information
    // Note that we can't choose a port less than 1023 if we are not privileged users (root)
    memset(&serverAddr, 0, sizeof(serverAddr));         // Zero out the structure
    serverAddr.sin_family = AF_INET;                    // Use Internet address family
    serverAddr.sin_port = htons(port);                  // Server port number
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);     // Any incoming interface

    // Bind to the local address
    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        cout << "bind() failed" << endl;
        exit(1);
    }

    // Listen for connection requests
    if (listen(serverSock, MAXPENDING) < 0)
    {
        cout << "listen() failed" << endl;
        exit(1);
    }
}

void processSockets (fd_set readySocks)
{
    char* buffer = new char[BUFFERSIZE];       // Buffer for the message from the server
    int size;                                    // Actual size of the message

    // Loop through the descriptors and process
    for (int sock = 0; sock <= maxDesc; sock++)
    {
        if (!FD_ISSET(sock, &readySocks))
            continue;

        // Clear the buffers
        memset(buffer, 0, BUFFERSIZE);

        struct sockaddr_in clientSocketInfo;
        unsigned int clientSocketInfoSize = sizeof(clientSocketInfo);
        memset(&clientSocketInfo, 0, clientSocketInfoSize);
        getpeername(sock, (struct sockaddr *)&clientSocketInfo, &clientSocketInfoSize);

        string clientIPv4 = inet_ntoa(clientSocketInfo.sin_addr);

        // Receive data from the client
        string message;
        message = receiveData(sock, buffer, size, clientIPv4);

        // do stuff here depending on state and IP;

        // Echo the message back to the client
        sendData(message, sock, buffer, size, clientIPv4);
    }

    delete[] buffer;
}

string receiveData (int sock, char* inBuffer, int& size, string ip)
{
    // Receive the message from client
    size = recv(sock, (char *) inBuffer, BUFFERSIZE, 0);

    // Check for connection close (0) or errors (< 0)
    /*
    if (size <= 0)
    {
        cout << "recv() failed, or the connection is closed. " << endl;
        FD_CLR(sock, &recvSockSet);

        // Update the max descriptor
        while (FD_ISSET(maxDesc, &recvSockSet) == false)
              maxDesc -= 1;
        return;
    }
    */

    string InputMsgSizeInitial = string(inBuffer);
    int InputMsgSize = stoi(InputMsgSizeInitial);
    //std::cout << InputMsgSizeInitial << " " << InputMsgSize << '\n';
    memset(inBuffer, 0, BUFFERSIZE);

    string currentMsg = "";
    while(InputMsgSize != 0){
      if(InputMsgSize >= BUFFERSIZE){
        InputMsgSize -= recv(sock, (char *) inBuffer, BUFFERSIZE, 0);
        currentMsg += string(inBuffer);
        memset(inBuffer, 0, BUFFERSIZE);
      }
      else{
        InputMsgSize -= recv(sock, (char *) inBuffer, InputMsgSize, 0);
        currentMsg += string(inBuffer);
        memset(inBuffer, 0, BUFFERSIZE);
      }
    }
    currentMsg.erase(currentMsg.find('\n'), 1);
    cout << "RecvFrom "<< ip << " : " << currentMsg << endl;

    return currentMsg;
}

void sendData (string msgToSend, int sock, char* buffer, int size, string ip)
{
    int bytesSent = 0;                   // Number of bytes sent
    //std::cout << msgToSend << '\n';
    //msgToSend = "This is a message testing getting data";
    int sizeMsgToSend = msgToSend.length();
    string initialMsgToSend = std::to_string(sizeMsgToSend); // initial message = size of message to send
    int sizeInitialMsgToSend = initialMsgToSend.length(); // size of initial message
    char* dataSending = new char[sizeInitialMsgToSend]; // buffer has to be size of the message to be sending first.
    strcpy(dataSending, initialMsgToSend.c_str()); // copying size of message into buffer
    send(sock, (char *) dataSending, sizeInitialMsgToSend, 0); // sending the size of the message

    memset(dataSending, 0, sizeInitialMsgToSend);

    dataSending = new char[sizeMsgToSend];
    strcpy(dataSending, msgToSend.c_str()); // copying the message to be sent into the buffer
    // Sent the data
    bytesSent += send(sock, (char *) dataSending, sizeMsgToSend, 0); // sending the acutal message

    /*
    if (bytesSent < 0 || bytesSent != size)
    {
        cout << "error in sending" << endl;
        return;
    }
    if (strncmp(buffer, "terminate", 9) == 0)
        terminated = true;
    */


    cout << "SentTo " << ip << " : " << msgToSend << endl;
}

void drawGrid(int width, int heigth, vector<player> players, int turn, vector<location> pointsTaken)
{
	for (int y = 0; y <= (heigth * 2) ; y++)
	{
		cout << "               ";
		if (y % 2 == 0 )
		{
			for (int x = 0; x <= width * 2; x++)
			{
				// odds are space and evens are lines
				if (x % 2 != 0)
				{
					cout << "-";
				}
				else
				{
					bool printed = false;
					bool taken = false;

					// plot taken points
					for (int xx = 0; xx < pointsTaken.size(); xx++)
					{
						int ptx = pointsTaken[xx].getXpos();
						int pty = pointsTaken[xx].getYpos();
						ptx = ptx * 2;
						pty = pty * 2;
						if (x == ptx && y == pty)
						{
							// print @ for points that players have already been in
							cout << "@";
							printed = true;
							taken = true;
						}
					}
					
					// plot current player points
					if (taken == false)
					{
						for (int i = 0; i < players.size(); i++)
						{
							int px = players[i].getXpos();
							int py = players[i].getYpos();
							px = px * 2;
							py = py * 2;
							if (x == px && y == py)
							{
								cout << players[i].getPiece();
								printed = true;
							}
						}					
					
					}
										
					if (printed == false)
					{
						cout << " ";
					}
				}
			}
			cout << endl;
		}
		else
		{
			for (int x = 0; x <= width * 2; x++)
			{
				// odds are space and evens are lines
				if (x % 2 == 0)
				{
					cout << "|";
				}
				else
				{
					cout << " ";
				}
			}
			cout << " " << endl;
		}
		
	}
}
