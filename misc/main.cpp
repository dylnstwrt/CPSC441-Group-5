#include <iostream>
#include <vector>

using namespace std;

//void drawGrid(int width, int heigth, vector<player> players, int turn);
string gameData ="";


class player {
	int xpos, ypos;
	int points;
	string piece;
	string playerName;
public:
	// set name
	void setName(string n)
	{
		playerName = n;
	}
	// set location
	void setPos(int x, int y)
	{
		xpos = x;
		ypos = y;
	}
	void setXpos(int x)
	{
		xpos = x;
	}
	void setYpos(int y)
	{
		ypos = y;
	}
	// add point
	void addPoint(int po)
	{
		points = po;
	}
	// set piece
	void setPiece(string pie)
	{
		piece = pie;
	}
	// get player's name
	string getName()
	{
		return playerName;
	}
	// get point
	int getXpos()
	{
		return xpos;
	}
	int getYpos()
	{
		return ypos;
	}
	// get points
	int getPoints()
	{
		return points;
	}
	// get piece
	string getPiece()
	{
		return piece;
	}
};
class location {
	int xpos, ypos;
public:
	void setPos(int x, int y)
	{
		xpos = x;
		ypos = y;
	}
	// get point
	int getXpos()
	{
		return xpos;
	}
	int getYpos()
	{
		return ypos;
	}

};
void drawGrid(int width, int heigth, vector<player> players, int turn, vector<location> pointsTaken)
{
	for (int y = 0; y <= (heigth * 2) ; y++)
	{
		gameData.append("              ");
		//cout << "               ";
		if (y % 2 == 0 )
		{
			for (int x = 0; x <= width * 2; x++)
			{
				// odds are space and evens are lines
				if (x % 2 != 0)
				{
					gameData.append("-");
					//cout << "-";
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
							gameData.append("@");
							//cout << "@";
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
								gameData.append(players[i].getPiece());
								//cout << players[i].getPiece();
								printed = true;
							}
						}					
					
					}
										
					if (printed == false)
					{
						gameData.append(" ");
						//cout << " ";
					}
				}
			}
			gameData.append("\n");
			//cout << endl;
		}
		else
		{
			for (int x = 0; x <= width * 2; x++)
			{
				// odds are space and evens are lines
				if (x % 2 == 0)
				{
					gameData.append("|");
					//cout << "|";
				}
				else
				{
					gameData.append(" ");
					//cout << " ";
				}
			}
			gameData.append(" \n");
			//cout << " " << endl;
		}
		
	}
}
string gameDataa()
{
	return gameData;
}

int main()
{
	bool gameOver = false;
	const int width = 7;
	const int heigth = 3;
	vector<player> players;
	vector<location> pointsTaken;

	// TCP connection creates player -> they select their piece and name
	// also adds player to the players vector
	player playerOne;
	playerOne.setPiece("*");
	playerOne.setPos(0, 0); // top left
	playerOne.setName("playerUno");

	player playerTwo;
	playerTwo.setPiece("x");
	playerTwo.setPos(width, heigth); // bottom right
	playerTwo.setName("playerTWO");

	player playerThree;
	playerThree.setPiece("K");
	playerThree.setPos(width, 0); // bottom right
	playerThree.setName("player#3");

	player playerFour;
	playerFour.setPiece("G");
	playerFour.setPos(0, heigth); // bottom right
	playerFour.setName("playerFOUR");


	// add players to game
	players.push_back(playerOne);
	players.push_back(playerTwo);
	players.push_back(playerThree);
	players.push_back(playerFour);

	int availablePoints = (width+1) * (heigth+1);
	int usedpoints = 4;
	unsigned int turnCount = 0;

	while (!gameOver)
	{
		drawGrid(width, heigth, players, turnCount, pointsTaken);
		string xx = gameDataa();
		cout << xx;
		gameData = "";

		cout << "It's " + players[turnCount].getName() + "'s turn" << endl;
		int x, y;
		cout << "New x value: ";
		cin >> x;
		cout << "New y value: ";
		cin >> y;

		// add current player position of points that have been taken
		location pp;
		pp.setPos(players[turnCount].getXpos(), players[turnCount].getYpos());
		pointsTaken.push_back(pp);

		players[turnCount].setXpos(x);
		players[turnCount].setYpos(y);

		
		// go to next turn
		turnCount++;
		// reset turn to first player
		if (turnCount >= players.size())
		{
			turnCount = 0;
		}
		usedpoints++;
		if (usedpoints == availablePoints)
		{
			gameOver = true;
			drawGrid(width, heigth, players, turnCount, pointsTaken);
		}	
	}

	cout << "----------------------------GAME OVER--------------------------" << endl;

	system("pause");
	return 0;
}