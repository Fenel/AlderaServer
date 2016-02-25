#ifndef __PATHFIND_H__
#define __PATHFIND_H__

#include <list>
#include <deque>
#include <string>

struct Point
{
	//Point(unsigned short _x, unsigned short _y, unsigned char _z, short _attribute);

	unsigned short x, globalX;
	unsigned short y, globalY;
	unsigned char z;
	short attribute;
};
/*
Point::Point(unsigned short _x, unsigned short _y, unsigned char _z, short _attribute)
{
	x = _x;
	y = _y;
	z = _z;
	attribute = _attribute;
}
*/

class PathFind
{
public:
	PathFind(){};
	~PathFind();
	void getPath(unsigned short fromX, unsigned short fromY, unsigned short toX, unsigned short toY, unsigned char z, std::list<Point> &points);
	void fillCollisionMap(unsigned short startX, unsigned short startY, unsigned char z); //fills collision map and clear path map
private:
	Point collisionMap[25][25];
	Point pathMap[25][25];
	std::list<Point> finalPoints; //tiles to reach final point
	std::list<Point> checkingQueue; //tiles to visit in next step
};

#endif 
