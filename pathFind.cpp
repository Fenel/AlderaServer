#include "pathFind.h"
#include "map.h"
#include "game.h"
#include <fstream>

extern Map Map;
extern Game Game;
extern string intToStr(int);

PathFind::~PathFind()
{

}

void PathFind::getPath(unsigned short fromX, unsigned short fromY, unsigned short toX, unsigned short toY, unsigned char z, std::list<Point> &points)
{
	points.clear();
	this->finalPoints.clear();
	this->checkingQueue.clear();
	this->fillCollisionMap(fromX, fromY, z);
	
	Point pointek;

	pointek.globalX = fromX;
	pointek.globalY = fromY;
	pointek.x = 12;
	pointek.y = 12;
	pointek.z = z;
	pointek.attribute = 0;
	checkingQueue.push_back(pointek); //adding starting point to queue, attribute = 0 (start pos)

	pathMap[12][12].attribute = 0; //set starting point on pathMap as attribute = 0
	
	bool var = 0;
	bool leave = false;
	for(std::list<Point>::iterator it = checkingQueue.begin(); it != checkingQueue.end(); it++)
	{
		if(leave)
			break;
		if(var == 1)
			checkingQueue.pop_front();

		int tempx = 1, tempy = 1;

		for(unsigned char x = it->x - 1; x <= it->x + 1; x++)
		{
			if(leave)
				break;
			tempy = 1;
			for(unsigned char y = it->y - 1; y <= it->y + 1; y++)
			{
				if(x < 25 && y < 25)//&& ((tempx+tempy)%2 == 1)) //if point is not out of map (pathMap[25][25]), modulo by nie chodzi³ na skos
				{
					if( (pathMap[x][y].attribute != -1 || collisionMap[x][y].attribute == 1) && (pathMap[x][y].globalX != toX || pathMap[x][y].globalY != toY) ) //if checking point is allready checked or is not walkable, proceed to next point
						continue;
					else
					{
						pathMap[x][y].attribute = it->attribute + 1;
						pointek.globalX = pathMap[x][y].globalX;
						pointek.globalY = pathMap[x][y].globalY;
						pointek.x = x;
						pointek.y = y;
						pointek.z = z;
						pointek.attribute = it->attribute + 1;
						checkingQueue.push_back(pointek); //add point to list to check this point

						if(pathMap[x][y].globalX == toX && pathMap[x][y].globalY == toY) //if current point is destination point
						{
							leave = true;
							break;
						}
					}
				}
				tempy++;
			}
			tempx++;
		}
		var = 1;
	}

	finalPoints.push_back(pathMap[toX-fromX+12][toY-fromY+12]);
	Point *point = &pathMap[toX-fromX+12][toY-fromY+12];

	Point temptab[25][25];

	for(int y = 0; y < 24; y++)
	for(int x = 0; x < 24; x++)
		temptab[x][y] = pathMap[x][y];

	while(point->attribute != 0) //&& (!finalPoints.empty() && finalPoints.begin()->attribute > 0))
	{
		Point *lastPoint = point;
		bool leave = false;
		int tempx = 1, tempy = 1;
		for(unsigned char x = point->x - 1; x <= point->x + 1; x++)
		{
			tempy = 1;
			for(unsigned char y = point->y - 1; y <= point->y + 1; y++)
			{
				if(x < 25 && y < 25 && (pathMap[x][y].attribute == point->attribute - 1) && ((tempx+tempy)%2 == 1)) //modulo by nie chodzi³ na skos
				{
					finalPoints.push_back(pathMap[x][y]);
					point = &pathMap[x][y];
					temptab[x][y].attribute = -9;/////////////
					leave = true;
				}
				tempy++;
				if(leave)
					break;
			}
			tempx++;
			if(leave)
				break;
		}
		if(leave == false)
		{
			tempx = 1, tempy = 1;
			for(unsigned char x = point->x - 1; x <= point->x + 1; x++)
			{
				tempy = 1;
				for(unsigned char y = point->y - 1; y <= point->y + 1; y++)
				{
					if(x < 25 && y < 25 && (pathMap[x][y].attribute == point->attribute - 1) && ((tempx+tempy)%2 == 0)) //modulo by nie chodzi³ na skos
					{
						finalPoints.push_back(pathMap[x][y]);
						point = &pathMap[x][y];
						temptab[x][y].attribute = -9;/////////////
						leave = true;
					}
					tempy++;
					if(leave)
						break;
				}
				tempx++;
				if(leave)
					break;
			}
		}
		if(lastPoint == point)
			break;
	}
	finalPoints.reverse();

	if(finalPoints.size() > 0)
		finalPoints.pop_front();
	if(finalPoints.size() > 0)
		finalPoints.pop_back();

	points = finalPoints;
}

void PathFind::fillCollisionMap(unsigned short startX, unsigned short startY, unsigned char z)
{
	unsigned int mapX = 0;
	unsigned int mapY = 0;

	for(unsigned int ix = startX - 12; ix <= startX + 12; ix++)
	{
		mapY = 0;
		for(unsigned int iy = startY - 12; iy <= startY + 12; iy++)
		{
			this->collisionMap[mapX][mapY].x = mapX;
			this->collisionMap[mapX][mapY].y = mapY;
			this->collisionMap[mapX][mapY].globalX = ix;
			this->collisionMap[mapX][mapY].globalY = iy;
			this->collisionMap[mapX][mapY].z = z;

			this->pathMap[mapX][mapY].x = mapX;
			this->pathMap[mapX][mapY].y = mapY;
			this->pathMap[mapX][mapY].globalX = ix;
			this->pathMap[mapX][mapY].globalY = iy;
			this->pathMap[mapX][mapY].z = z;
			this->pathMap[mapX][mapY].attribute = -1;
	
			if(Game.canDoCombat(ix, iy, z) && Map.getTile(ix, iy, z) && Map.getTile(ix, iy, z)->isCollision() == false && (Map.getTile(ix, iy, z)->canAddThing() == 1
			|| Map.getTile(ix, iy, z)->canAddThing() == 2 || Map.getTile(ix, iy, z)->getTopItem()->mID == 32 || Map.getTile(ix, iy, z)->getTopItem()->mID == 34))
				this->collisionMap[mapX][mapY].attribute = 0; //tile is walkable
			else
				this->collisionMap[mapX][mapY].attribute = 1; //tile is not walkable

			mapY++;
		}
		mapX++;
	}
	/*
	string szajs[25];
	for(int y = 0; y < 24; y++)
	{
		szajs[y] = "";
		for(int x = 0; x < 24; x++)
		{
			szajs[y] += "  ";
			szajs[y] += intToStr(collisionMap[x][y].attribute);
		}
	}
	std::string filename = "pCollisionMap.txt";
    std::ofstream plik(filename.c_str());
    {
		for(int y = 0; y < 24; y++)
		{
			plik << szajs[y].c_str() << endl;
		}

        plik.close();
    }
	*/
}
