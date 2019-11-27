/*
 * A simple TCP client that sends messages to a server and display the message
   from the server.
 * For use in CPSC 441 lectures
 * Instructor: Prof. Mea Wang
 */

#include <iostream>
#include <sys/socket.h> // for socket(), connect(), send(), and recv()
#include <arpa/inet.h>  // for sockaddr_in and inet_addr()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()

using namespace std;

const int BUFFERSIZE = 32;   // Size the message buffers

void sendData (string, int, char[], int);
string receiveData (int, char[], int&);
bool playing = false;

/**
 * @author San and Dylan
 **/

int main(int argc, char *argv[])
{
    int sock;                        // A socket descriptor
    struct sockaddr_in serverAddr;   // Address of the server
    char inBuffer[BUFFERSIZE];       // Buffer for the message from the server
    int bytesRecv;                   // Number of bytes received

    char outBuffer[BUFFERSIZE];      // Buffer for message to the server
    int msgLength;                   // Length of the outgoing message
    int bytesSent;                   // Number of bytes sent

    // Check for input errors
    if (argc != 3)
    {
        cout << "Usage: " << argv[0] << " <Server IP> <Server Port>" << endl;
        exit(1);
    }

    // Create a TCP socket
    // * AF_INET: using address family "Internet Protocol address"
    // * SOCK_STREAM: Provides sequenced, reliable, bidirectional, connection-mode byte streams.
    // * IPPROTO_TCP: TCP protocol
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
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
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
    {
        cout << "setsockopt() failed" << endl;
        exit(1);
    }

    // Initialize the server information
    // Note that we can't choose a port less than 1023 if we are not privileged users (root)
    memset(&serverAddr, 0, sizeof(serverAddr));         // Zero out the structure
    serverAddr.sin_family = AF_INET;                    // Use Internet address family
    serverAddr.sin_port = htons(atoi(argv[2]));         // Server port number
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);    // Server IP address

    // Connect to the server
    // * sock: the socket for this connection
    // * serverAddr: the server address
    // * sizeof(*): the size of the server address
    if (connect(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
    {
        cout << "connect() failed" << endl;
        exit(1);
    }


    cout << "Please enter a message to be sent to the server ('logout' to terminate): ";
    fgets(outBuffer, BUFFERSIZE, stdin);
    while (strncmp(outBuffer, "logout", 6) != 0)
    {

        msgLength = strlen(outBuffer);
        string msg = string(outBuffer);
        string msgLengthStr = to_string(msgLength);
        int sizeMsgLengthStr = msgLengthStr.length();

        //std::cout <<  msgLength << " " << msgLengthStr << " " << sizeMsgLengthStr << '\n';
        strcpy(outBuffer, msgLengthStr.c_str()); // copying size of message into buffer

        // Send the message to the server
        //bytesSent = send(sock, (char *) &outBuffer, sizeMsgLengthStr, 0);
        sendData (msg, sock, (char *) &outBuffer, sizeMsgLengthStr);

        /*
        if (bytesSent < 0 || bytesSent != msgLength)
        {
            cout << "error in sending" << endl;
            exit(1);
        }
        */

        //strcpy(outBuffer, msg.c_str());
        //send(sock, (char *) &outBuffer, msgLength, 0);



        /*
        // Receive the response from the server
        bytesRecv = recv(sock, (char *) &inBuffer, msgLength, 0);
        cout << msgLength << endl;
        // Check for connection close (0) or errors (< 0)
        if (bytesRecv <= 0 || bytesRecv != msgLength)
        {
            cout << "recv() failed, or the connection is closed. " << endl;
            exit(1);
        }
        cout << "Server: " << inBuffer;
        */


        // Clear the buffers
        memset(&outBuffer, 0, BUFFERSIZE);


        string msgRecev;
        msgRecev = receiveData(sock, (char*)inBuffer, bytesRecv);

        std::cout << "Server: " << msgRecev << '\n';

        // check for blocking message
        if (msgRecev.compare(0,2,"**") == 0) {
            playing = true;
        }
        
        // issues with amount being read from socket when multiple messages are queued
        // bandaid is using microsleeps to make sure only one message at a time would be waiting on the socket
        // otherwise we read multiple messages that have been sent, i.e. string = integer string + message + first n bits of next message.

        // when playing is true and the last message recieved isn't the unblocking message (i.e. "It's your turn player #")
        while(msgRecev.compare(0,4,"It's") != 0 && playing) {
            char* tempBuff = new char[BUFFERSIZE];
            msgRecev = receiveData(sock, (char*)tempBuff, bytesRecv);
            cout << "Server: " << msgRecev << endl;
            if (msgRecev.compare(0,1,"!") == 0) playing = false;
            delete[] tempBuff; 
        }
        

        cout << "Please enter a message to be sent to the server ('logout' to terminate): ";
        fgets(outBuffer, BUFFERSIZE, stdin);
    }

    // Close the socket
    close(sock);
    exit(0);
}

/**
 * @author San
 * */
string receiveData (int sock, char* inBuffer, int& size)
{
    // Receive the message from client

    recv(sock, (char *) inBuffer, BUFFERSIZE, 0);


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
    //std::cout << currentMsg << '\n';
    //currentMsg.erase(currentMsg.find('\n'), 1);
    //cout << "Client Reciev: " << currentMsg << endl;

    return currentMsg;
}

/**
 * @author San
 **/
void sendData (string msgToSend, int sock, char* buffer, int size)
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


    cout << "Client Send: " << msgToSend << endl;
}
