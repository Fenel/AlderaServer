#ifndef __TILE_H__
#define __TILE_H__

#include <SFML/Network.hpp>
#include <deque>
#include "item.h"
#include "creature.h"
#include "enums.h"

typedef std::deque<Item*> ItemDeque;
typedef std::deque<Creature*> CreatureDeque;

class Tile : public Thing
{
public:
	Tile(Position _tilePos){pos.x = _tilePos.x; pos.y = _tilePos.y; pos.z = _tilePos.z; ground = NULL; type = 0;};
	Tile(int Px, int Py, unsigned char Pz) {pos.x = Px; pos.y = Py, pos.z = Pz; ground = NULL; type = 0;};
	Tile() { ground = NULL; type = 0;};

	unsigned int canAddThing();
    void addThing(Thing *thing);
    void transformThing(unsigned char stackpos, Thing *thing);
    bool removeThing(unsigned char stackpos);
	bool removeTopItem(unsigned short count = 1);
    void removeCreatureByID(unsigned int id);    
    Thing* getThing(unsigned char stackpos);
	bool hasContainer();
	Item* getTopItem();
    unsigned char getIndexOfThing(Thing* thing);
	sf::Packet returnTilePacket();
	void getAllItems(std::deque<Item*> &items); 

	bool isCollision();
	bool isProjectile();
    
    CreatureDeque& getCreatures();

	Position pos;
	unsigned char type;
private:
	Item* ground;
	ItemDeque topItems;
	ItemDeque downItems;
	CreatureDeque creatures;
};


#endif
