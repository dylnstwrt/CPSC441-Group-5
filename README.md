# CPSC441-Group-5

Makefile should be okay, otherwise it's advisable that if you would like to manually compile:

        $ g++ -std=c++11 <TCPClient.cpp/SelectServer.cpp> -o {TCPClient/SelectServer}

To Run:

        $ ./SelectServer <Listening Port>
        $ ./TCPClient <IP of SelectServer> <Listening Port>
        
RAC Servers (for internal reference):

        10.1.2.124 (Dylan)
        10.1.2.106 (Dylan)

Instructions:

        Messages from clients will be echoed back until the keyword "start" is recieved.

        Once the keyword is recieved, the client will read a message asking it to wait for the game to start.

        Once the all clients have submitted their keyword "start", the game will update all clients with the initial game state.

        After the gamestate is recieved by all clients, the client who's turn it is will be sent a message that unblocks (doesn't reprompt them to wait for a message), and allows them to send a message (which for the time being isn't error checked) containing string of the desired x-coordinate and the desired y-coordinate (both start indexing at 0) delimited by a single comma and no white space. (ex.: "1,2")

        Once recieved, the game state will update on the server-side, and update all hosts with the new gamestate, blocking the client who made the turn, and unblock the client who's turn is next.

        This continues until the number of pieces placed equals the total board size.

        The server doesn't participate in a game instance, but can observe the game state as it updates.