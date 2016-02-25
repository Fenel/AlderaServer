#ifndef __MAP_H__
#define __MAP_H__

#include <deque>
#include "tile.h"

typedef std::deque<Creature*> CreatureDeque;

class Map
{
public:    
	Map();
	~Map(){};

	void getSpectators(unsigned int posx, unsigned int posy, unsigned char posz, std::deque<Creature*> &monsters);
    void getSpectators(Position& centerPos, std::deque<Creature*> &monsters);
	void getFloorAreaPlayers(unsigned int posx, unsigned int posy, unsigned char posz, std::deque<Player*> &players);
    
    unsigned int posToIndex(unsigned int x, unsigned int y, unsigned char z);
    void indexToPos(unsigned int &x, unsigned int &y, unsigned char &z, unsigned int index);

    void load();

    void setTile(Position pos, Tile *tile);
	void setTile(unsigned short x, unsigned short y, unsigned char z, Tile *tile);
    
    Tile *getTile(Position pos);
	Tile *getTile(unsigned short x, unsigned short y, unsigned char z);
	
    void save();

private:
    Tile *tiles_tab[6144000];
};

#endif
