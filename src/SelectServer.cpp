 /* Modified version of the example code given by Mea Wang for CPSC441.
 * @author Group 5: Dylan Stewart, Nicolas Urrego, Sandesh Regmi, and Wentao Sun
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
#include <sstream>
#include "gamestate.h"

const int BUFFERSIZE = 32;    // Size the message buffers
const int MAXPENDING = 10;    // Maximum pending connections


using namespace std;

/////////////////////////////////Game Variables///////////////////////////////////////////
struct GameState state[3];
//////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////// Server Variables///////////////////////////////////////

fd_set recvSockSet;   // The set of descriptors for incoming connections
int maxDesc = 0;      // The max descriptor
bool terminated = false; // for loop in main
vector<string> clientAddresses; // vector for client IP strings, added when client-server connection is established
int curClientRoom[6] = {0,0,0,0,0,0};

bool playing = false; // when false, server is considered "in lobby"

vector <int> clientSockets; 
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////Server Methods//////////////////////////////////////////
void initServer (int&, int port);
void processSockets (fd_set);
void sendData (string, int, char[], string);
string receiveData (int, char[], int&, string);
string sendState();
string sendCommands();
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////Game Methods////////////////////////////////////////////
void createPlayer(int, int);
void deletePlayer(int, int);
void startGame(int);
void playGame(int, string);
void endGame(int);
//////////////////////////////////////////////////////////////////////////////////////////

/* 
    @Author Mea Wang, edited by Dylan for CPSC441
 */
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
    while(!terminated)
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
            // might be good idea to concact with sin_port before pushing for granularity
            clientAddresses.push_back(inet_ntoa(clientAddr.sin_addr));
            // don't know if this actually works with multiple hosts since i'm not able to test
            clientSockets.push_back(clientSock);

            // Add the new connection to the receive socket set
            FD_SET(clientSock, &recvSockSet);
            maxDesc = max(maxDesc, clientSock);
		sendData(sendState(), clientSock, new char[BUFFERSIZE], inet_ntoa(clientAddr.sin_addr));
        }

        // Then process messages waiting at each ready socket
        else processSockets(tempRecvSockSet);
    }

    // Close the connections with the client
    for (int sock = 0; sock <= maxDesc; sock++)
    {
        if (FD_ISSET(sock, &recvSockSet)) close(sock);
    }

    // Close the server sockets
    close(serverSock);
}

/* 
    @Author Mea Wang
 */
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
	cout << "Listening on port " << port << "\n";
}

/* 
    @Author Mea Wang, Edited for CPSC441 by Dylan
 */
void processSockets (fd_set readySocks)
{
	char* buffer = new char[BUFFERSIZE];       // Buffer for the message from the server
	int size;

    	// Loop through the descriptors and process
    	for (int sock = 0; sock <= maxDesc; sock++)
    	{
        	if (!FD_ISSET(sock, &readySocks))
            	continue;

        	// Clear the buffers
        	memset(buffer, 0, BUFFERSIZE);

      	  	// create struct, populate struct with client information, use IP address and reference clientAddresses vector for identification
        	struct sockaddr_in clientSocketInfo;
        	unsigned int clientSocketInfoSize = sizeof(clientSocketInfo);
        	memset(&clientSocketInfo, 0, clientSocketInfoSize);
        	getpeername(sock, (struct sockaddr *)&clientSocketInfo, &clientSocketInfoSize);

        	string clientIPv4 = inet_ntoa(clientSocketInfo.sin_addr);

        	// Receive data from the cli// do stuff here depending on state and IP;ent
        	string messageReceived;
        	messageReceived = receiveData(sock, buffer, size, clientIPv4);
		string messageToSend = "Invalid Command\nType 'Help' for list of Commands";

		int index;
		for(int i = 0; i < clientAddresses.size(); i++)
		{
			if(clientIPv4.compare(clientAddresses.at(i)) == 0)
			{
				index = i;
				break;
			}
		}
		int roomNo = curClientRoom[index] - 1;

		if(messageReceived.compare("help") == 0)
		{
			messageToSend = sendCommands();
		}
		if(messageReceived.compare("refresh") == 0)
		{
			messageToSend = sendState();
		}
		if(messageReceived.compare("logout") == 0)
		{
			
		}
		if(roomNo == -1)
		{
			if(messageReceived.substr(0,5).compare("join ") == 0)
			{
				int roomNumber = stoi(messageReceived.substr(5, 1));
				curClientRoom[index] = roomNumber;
				createPlayer(roomNumber - 1, index);

				messageToSend = "Joined room ";
				messageToSend += to_string(curClientRoom[index]);

				printf("%dth player joined room %d\n", index, curClientRoom[index]);
			}
		}
		else if(state[roomNo].playing == true)
		{
			playGame(roomNo, messageReceived);
			continue;
		}	
		else
		{
			if(messageReceived.compare("leave") == 0)
			{
				deletePlayer(roomNo, index);
				messageToSend = "Left room ";
				messageToSend += to_string(curClientRoom[index]);
				curClientRoom[index] = 0; 
			}
        		if (messageReceived.compare("start")==0)
          		{
         			state[roomNo].votes++;
          	    		messageToSend = "**wait for start**";
      				// only worrying about getting the game started for at least the one person.
				sendData(messageToSend, sock, buffer, clientIPv4);
				if (state[roomNo].noPlayer == state[roomNo].votes)
      				{
					startGame(roomNo);
				}
				continue;
			}
		}
		sendData(messageToSend, sock, buffer, clientIPv4);
    	}
	delete[] buffer;
}

/* 
    @author San
 */
string receiveData (int sock, char* inBuffer, int& size, string ip)
{
	// Receive the message from client
    	size = recv(sock, (char *) inBuffer, BUFFERSIZE, 0);

    	string InputMsgSizeInitial = string(inBuffer);
    	int InputMsgSize = stoi(InputMsgSizeInitial);
    	//std::cout << InputMsgSizeInitial << " " << InputMsgSize << '\n';
    	memset(inBuffer, 0, BUFFERSIZE);

    	string currentMsg = "";
   	while(InputMsgSize != 0)
	{
      		if(InputMsgSize >= BUFFERSIZE)
		{
        		InputMsgSize -= recv(sock, (char *) inBuffer, BUFFERSIZE, 0);
        		currentMsg += string(inBuffer);
        		memset(inBuffer, 0, BUFFERSIZE);
      		}
      		else
		{
        		InputMsgSize -= recv(sock, (char *) inBuffer, InputMsgSize, 0);
       			currentMsg += string(inBuffer);
        		memset(inBuffer, 0, BUFFERSIZE);
      		}
    	}
    	currentMsg.erase(currentMsg.find('\n'), 1);
    	cout << "RecvFrom "<< ip << " : " << currentMsg << endl;

    	return currentMsg;
}

/* 
    @author San
 */
void sendData (string msgToSend, int sock, char* buffer, string ip)
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
    bytesSent += send(sock, (char *) dataSending, sizeMsgToSend, 0);

    cout << "SentTo " << ip << " : " << msgToSend << endl;
}

/* 
    @author Dylan
 */
void createPlayer(int roomNo, int index)
{
	int i = state[roomNo].players.size();
        player toCreate;
        string playerName = "Player " + to_string(state[roomNo].players.size()+1);
        toCreate.setPiece(state[roomNo].playerSymbols[i]);
        toCreate.setPos(state[roomNo].startingXCoordinates[i], state[roomNo].startingYCoordinates[i]);
        toCreate.setName(playerName);
	toCreate.setIndex(index);

        state[roomNo].addPlayer(toCreate);
}
void deletePlayer(int roomNo, int index)
{
	state[roomNo].removePlayer(index);
}

void startGame(int roomNo)
{
	char* buffer = new char[BUFFERSIZE];

	usleep(1000000/2);

    	for (int i = 0; i < clientSockets.size(); i++)
	{
		if(curClientRoom[i] == roomNo + 1)
		{
        		string messageToSend = state[roomNo].drawGrid();
        		sendData(messageToSend, clientSockets.at(i), buffer, clientAddresses.at(i));
		}
	}

	usleep(1000000/2);

	state[roomNo].playing = true;
	int index = state[roomNo].turnCount;
	player turnPlayer = state[roomNo].players.at(index);
	string send = "It's your turn " + turnPlayer.getName();
	int i = turnPlayer.getIndex();
	sendData(send, clientSockets.at(i), buffer, clientAddresses.at(i));
}

/* 
    @author Dylan. Game loop modelled after main() in misc/main.cpp; which was written by Nico
 */
void playGame(int roomNo, string messageRecieved)
{
	string messageToSend;
	char* buffer = new char[BUFFERSIZE];

    	usleep(1000000/2);

	int turnCount = state[roomNo].turnCount;
	player turnPlayer = state[roomNo].players.at(turnCount);
	int index = turnPlayer.getIndex();
	// allow client to recieve message before piling another one on the recvsock causing issues with current protocol
	// more of an issue if first person connected is the last one to ready up.
        usleep(1000000/2);

       	// wait for client's response

        // for tokenizing message from client
        vector<string> coordinates;
        stringstream stream(messageRecieved);
        
        string xCoordinateString;
        string yCoordinateString;

        getline(stream, xCoordinateString, ',');
        getline(stream, yCoordinateString, ',');

        // parse int from strings
        int xCoord = stoi(xCoordinateString);
        int yCoord = stoi(yCoordinateString);

	location pp;

        // set previous position of current client as taken in game state
        pp.setPos(turnPlayer.getXpos(), turnPlayer.getYpos());
        state[roomNo].pointsTaken.push_back(pp);

       	// update clients position to input
        state[roomNo].players.at(turnCount).setXpos(xCoord);
       	state[roomNo].players.at(turnCount).setYpos(yCoord);

       	// change turn
        state[roomNo].turnCount++;
	state[roomNo].turnCount = state[roomNo].turnCount%state[roomNo].noPlayer;
	state[roomNo].usedpoints++;
	turnCount = state[roomNo].turnCount;
	turnPlayer = state[roomNo].players.at(turnCount);
	index = turnPlayer.getIndex();
	if (state[roomNo].usedpoints == state[roomNo].availablePoints)
	{
		state[roomNo].gameOver = true;
		state[roomNo].drawGrid();
    		terminated = true;
	}
	else
	{
		string toSend = state[roomNo].drawGrid();
            	for (int i = 0; i < clientSockets.size(); i++)
		{
			if(curClientRoom[i] == roomNo + 1)
			{
                		sendData(toSend, clientSockets.at(i), buffer, clientAddresses.at(i));
			}
        	}
		usleep(1000000/2);
		messageToSend = "It's your turn " + turnPlayer.getName();
		sendData(messageToSend, clientSockets.at(index), buffer, clientAddresses.at(index));
    	}
}

string sendState()
{
	string message = "\n";
	for(int i = 0; i < 3; i++)
	{
		message += "Room: ";
		message += to_string(i + 1);
		message += " Status: ";
		
		if(state[i].playing)
		{
			message += "Playing ";
		}
		else
		{
			message += "Waiting ";
		}

		message += "Players: ";
		message += to_string(state[i].noPlayer);
		message += "/4\n";
	}
	return message;
}

string sendCommands()
{
	string message ="Commands:\njoin [1-3]\tJoins room number specified by arguement 1\nleave\t Leaves current room\nrefresh\t Refreshes state of rooms\nStart\t Tells server you are ready. Game starts once all players start\n[0-3],[0-7]\t Plot points onto game";

	return message;
}

/**
 * @brief used to clear the gamestate instance of the completed game in the states array
 * 
 * @param roomNo 
 * @author Dylan
 */
void endGame(int roomNo){
	string toSend = state[roomNo].drawGrid();
	state[roomNo].reset();
		for (int i = 0; i < clientSockets.size(); i++)
		{
			if (curClientRoom[i] == roomNo + 1)
			{
				char* buffer = new char[BUFFERSIZE];
				sendData(toSend, clientSockets.at(i), buffer, clientAddresses.at(i));
				usleep(1000000 / 2);
				sendData("!Returned to Lobby!", clientSockets.at(i), buffer, clientAddresses.at(i));
				curClientRoom[i] = 0;
				delete[] buffer;
			}
		}
