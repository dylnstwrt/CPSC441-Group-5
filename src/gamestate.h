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
	// This is the winning point in the grid
	location hiddenSpot; 
	// This is the winning player of the match
	player WinningPlayer;
	// Winning ending message boolean
	bool winMsg = false;

	int availablePoints = (WIDTH + 1) * (HEIGHT + 1);
	int usedpoints = 4;
	unsigned int turnCount = 0;

	string playerSymbols[4] = { "*", "x", "K", "G" };
	int startingXCoordinates[4] = { 0, WIDTH, WIDTH, 0 };
	int startingYCoordinates[4] = { 0, HEIGHT, 0, HEIGHT };

	int turnsMade = 0;

	bool playing = false;
	int votes = 0;

	void reset()
	{
		gameOver = false;
		winMsg = false;
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
	// Generating hidding winning spot
	void generateSpot()
	{
		bool winningSpot = false;
		while (!winningSpot)
		{
			// change srand seed if you want a constant value
			srand(time(NULL));
			int wX = (rand() % WIDTH) + 1;
			int wY = (rand() % HEIGHT) + 1;
			// Check that the winning spot is not at the starting points of the players
			if ((wX == 0 && wY == 0) || (wX == 0 && wY == HEIGHT) || (wX == WIDTH && wY == HEIGHT) || (wX == WIDTH && wY == 0))
			{
				winningSpot = false;
			}
			else
			{
				winningSpot = true;
				hiddenSpot.setPos(wX, wY);
			}
		}
	}
	
	string drawGrid()
	{
		stringstream formatString;
		formatString << endl;
		for (int y = 0; y <= (HEIGHT * 4); y++)
		{
			formatString << "               ";
			if (y % 4 == 0)
			{
				for (int x = 0; x <= WIDTH * 5; x++)
				{
					if (x % 5 != 0)
					{
						formatString << "-";
					}
					else
					{
						bool printed = false;
						bool taken = false;
						// This method checks previous points that been occupied and prints '@'
						for (int xx = 0; xx < pointsTaken.size(); xx++)
						{
							int ptx = pointsTaken[xx].getXpos() * 5;
							int pty = pointsTaken[xx].getYpos() * 4;
							if (x == ptx && y == pty)
							{
								formatString << "@";
								printed = true;
								taken = true;
							}
						}
						// This method prints the current location of the players.
						// If a player's current location has already been occupied ('@'). They lose 
						// their turn and it won't print their piece
						if (taken == false)
						{
							for (int i = 0; i < players.size(); i++)
							{
								int px = players.at(i).getXpos() * 5;
								int py = players.at(i).getYpos() * 4;
								if (x == px && y == py)
								{
									formatString << players.at(i).getPiece();
									printed = true;
								}
								// check to see if a player has won
								if (hiddenSpot.getXpos() == px && hiddenSpot.getYpos() == py)
								{
									WinningPlayer.setPiece(players.at(i).getPiece());
									WinningPlayer.setPos(players.at(i).getXpos(), players.at(i).getYpos());
									WinningPlayer.setName(players.at(i).getName());
									winMsg = true;
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
				for (int x = 0; x <= WIDTH * 5; x++)
				{
					if (x % 5 == 0)
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
		if (winMsg)
		{
			formatString << "Player: " << WinningPlayer.getName() << " has won" << endl;
		}
		return formatString.str();
	}
	
	// Get winning player
	player getWinner()
	{
		return WinningPlayer;
	}
};

#endif