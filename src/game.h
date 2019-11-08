/**
 * @author Nico
 **/

#include <iostream>
#include <vector>

using namespace std;

//void drawGrid(int width, int heigth, vector<player> players, int turn);

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