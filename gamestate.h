#include "game.h"

#ifndef GAMESTATE_H
#define GAMESTATE_H

struct GameState
{
	const int WIDTH = 7;
	const int HEIGHT = 3;
	bool gameOver = false;
	vector<player> players;
	int noPlayer = 0;
	vector<location> pointsTaken;

	int availablePoints = (WIDTH+1) * (HEIGHT+1);
	int usedpoints = 4;
	unsigned int turnCount = 0;

	string playerSymbols[4] = {"*", "x", "K", "G"};
	int startingXCoordinates[4] = {0, WIDTH, WIDTH, 0};
	int startingYCoordinates[4] = {0, HEIGHT, 0, HEIGHT};

	int turnsMade = 0;
	
	bool playing = false;
	int votes = 0;

	void reset()
	{
		gameOver = false;
		players.clear();
		pointsTaken.clear();
		turnsMade = 0;
		playing = false;
		votes = 0;
		noPlayer = 0;
	}
	void addPlayer(player p)
	{
		players.push_back(p);
		noPlayer++;
	}
	void removePlayer(int i)
	{
		for(int j = 0; j < noPlayer; j++)
		{
			if(players.at(j).getIndex() == i)
			{
				players.erase(players.begin() + j);
				noPlayer--;
				break;
			}
		}
	}
	
	string drawGrid()
	{   
   	 	stringstream formatString;
    		formatString << endl;
		for (int y = 0; y <= (HEIGHT * 2) ; y++)
		{
        		formatString << "               ";
			if (y % 2 == 0 )
			{
				for (int x = 0; x <= WIDTH * 2; x++)
				{
				// odds are space and evens are lines
					if (x % 2 != 0)
					{
                				formatString  << "-";
					}
					else
					{
						bool printed = false;
						bool taken = false;
						for (int xx = 0; xx < pointsTaken.size(); xx++)
						{
							int ptx = pointsTaken[xx].getXpos()*2;
							int pty = pointsTaken[xx].getYpos()*2;
							if (x == ptx && y == pty)
							{
                            					formatString << "@";
								printed = true;
								taken = true;
							}
						}
						if (taken == false)
						{
							for (int i = 0; i < players.size(); i++)
							{
								int px = players.at(i).getXpos()*2;
								int py = players.at(i).getYpos()*2;
								if (x == px && y == py)
								{
                                					formatString << players.at(i).getPiece();
									printed = true;
								}
							}
						}						
						if (printed == false)
						{	
                        				formatString << " ";
						}
					}
				}
        	    		formatString << endl;
			}
			else
			{
				for (int x = 0; x <= WIDTH * 2; x++)
				{
					if (x % 2 == 0)
					{
                    				formatString << "|";
					}
					else
					{
                    				formatString << " ";
					}
				}
            		formatString << " " << endl;
			}
		}
    	return formatString.str(); 
	}
};

#endif
