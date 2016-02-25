#include <iostream>
#include <vector>
#include <algorithm>

#include "tile.h"
#include "thing.h"
#include "itemType.h"

using namespace std;

CreatureDeque& Tile::getCreatures()
{  
    return creatures;
}


bool Tile::isCollision()
{
    if(ground && ground->isCollision())
    {
        return true;
	}

    if(!ground)
    {
        return true;
	}
    
    ItemDeque::const_iterator iit;
	for(iit = topItems.begin(); iit != topItems.end(); ++iit)
    {
		if((*iit)->isCollision())
		   return true;
	}
	
    if(!creatures.empty())
       return true;

	for(iit = downItems.begin(); iit != downItems.end(); ++iit)
    {
		if((*iit)->isCollision())
		   return true;
	}

	return false;
}

bool Tile::isProjectile()
{   
    if(ground && ground->baseItem->blockProjectile)
    {
        return false;
	}

    ItemDeque::const_iterator iit;
	for(iit = topItems.begin(); iit != topItems.end(); ++iit)
    {
		if((*iit)->baseItem->blockProjectile)
		   return false;
	}
	for(iit = downItems.begin(); iit != downItems.end(); ++iit)
    {
		if((*iit)->baseItem->blockProjectile)
		   return false;
	}

	return true;
}

unsigned int Tile::canAddThing()
{
    if(!ground)
    {
        return 0;
	} 
	else
	{
		if(ground->isTeleport())
			return 2;
		if(ground->baseItem->floorChangeNorth && ground->baseItem->floorChangeDown)
			return 4;
		if(ground->baseItem->floorChangeSouth && ground->baseItem->floorChangeDown)
			return 5;
		if(ground->baseItem->floorChangeEast && ground->baseItem->floorChangeDown)
			return 6;
		if(ground->baseItem->floorChangeWest && ground->baseItem->floorChangeDown)
			return 7;
		if(ground->baseItem->floorChangeDown)
			return 3;
		if(ground->baseItem->floorChangeNorth)
			return 8;
		if(ground->baseItem->floorChangeSouth)
			return 9;
		if(ground->baseItem->floorChangeEast)
			return 10;
		if(ground->baseItem->floorChangeWest)
			return 11;
		if(ground->isAlwaysOnTop())
		   return 0;
	}

    ItemDeque::const_iterator iit;
	for(iit = topItems.begin(); iit != topItems.end(); ++iit)
    {
		if((*iit)->isTeleport())
			return 2;
		if((*iit)->baseItem->floorChangeDown)
			return 3;
		if((*iit)->baseItem->floorChangeNorth && (*iit)->baseItem->floorChangeDown)
			return 4;
		if((*iit)->baseItem->floorChangeSouth && (*iit)->baseItem->floorChangeDown)
			return 5;
		if((*iit)->baseItem->floorChangeEast && (*iit)->baseItem->floorChangeDown)
			return 6;
		if((*iit)->baseItem->floorChangeWest && (*iit)->baseItem->floorChangeDown)
			return 7;
		if((*iit)->baseItem->floorChangeNorth)
			return 8;
		if((*iit)->baseItem->floorChangeSouth)
			return 9;
		if((*iit)->baseItem->floorChangeEast)
			return 10;
		if((*iit)->baseItem->floorChangeWest)
			return 11;
		if((*iit)->isAlwaysOnTop())
		   return 0;
	}
	for(iit = downItems.begin(); iit != downItems.end(); ++iit)
    {
		if((*iit)->isTeleport())
			return 2;
		if((*iit)->baseItem->floorChangeDown)
			return 3;
		if((*iit)->baseItem->floorChangeNorth && (*iit)->baseItem->floorChangeDown)
			return 4;
		if((*iit)->baseItem->floorChangeSouth && (*iit)->baseItem->floorChangeDown)
			return 5;
		if((*iit)->baseItem->floorChangeEast && (*iit)->baseItem->floorChangeDown)
			return 6;
		if((*iit)->baseItem->floorChangeWest && (*iit)->baseItem->floorChangeDown)
			return 7;
		if((*iit)->baseItem->floorChangeNorth)
			return 8;
		if((*iit)->baseItem->floorChangeSouth)
			return 9;
		if((*iit)->baseItem->floorChangeEast)
			return 10;
		if((*iit)->baseItem->floorChangeWest)
			return 11;
		if((*iit)->isAlwaysOnTop())
		   return 0;
	}
	return 1;
}

void Tile::addThing(Thing *thing)
{    
	thing->parent = this;

    Creature *creature = dynamic_cast<Creature *>(thing);
	if(creature)
	{
        creatures.insert(creatures.begin(), creature);
		creature->setPos(this->pos.x,this->pos.y,this->pos.z);
	}
    else
	{
        Item *item = dynamic_cast<Item *>(thing);
		if(!item)
			return;
		
        if(item->isGroundTile())
		{
            if(ground)
                delete ground;
            ground = item;
		}
		else if(item->isAlwaysOnTop() /*|| item->isTeleport()*/)
		{
			topItems.push_back(item);
        } 
		else if(item->isSplash())
		{
			ItemDeque::iterator it;
			for(it = downItems.begin(); it != downItems.end(); ++it)
			{
				if((*it)->isSplash())
				{
					downItems.erase(it);
					delete (*it);
					break;
				}
			}
			downItems.insert(downItems.begin(),item);
		}
		else
			downItems.push_back(item);
    }
}

unsigned char Tile::getIndexOfThing(Thing* thing)
{
	if(ground){
		if(ground == thing){
			return 0;
		}
	}

    unsigned char n = 0;
	ItemDeque::const_iterator iit;
	for(iit = topItems.begin(); iit != topItems.end(); ++iit)
    {
		++n;
		if((*iit) == thing)
			return n;
	}

	CreatureDeque::const_iterator cit;
	for(cit = creatures.begin(); cit != creatures.end(); ++cit)
    {
		++n;
		if((*cit) == thing)
			return n;
	}

	for(iit = downItems.begin(); iit != downItems.end(); ++iit)
    {
		++n;
		if((*iit) == thing)
			return n;
	}

	return 255;
}

bool Tile::hasContainer()
{
	ItemDeque::const_iterator iit;
	for(iit = topItems.begin(); iit != topItems.end(); ++iit)
    {
		if((*iit)->getContainer() != NULL)
			return true;
	}

	for(iit = downItems.begin(); iit != downItems.end(); ++iit)
    {
		if((*iit)->getContainer() != NULL)
			return true;
	}
	return false;
}

Thing *Tile::getThing(unsigned char stackpos)
{
    if(ground)
    {
		if(stackpos == 0)
		{
			return ground;
		}

		--stackpos;
	}

	if(stackpos < topItems.size())
		return topItems[stackpos];

	stackpos -= (unsigned char)topItems.size();

	if(stackpos < creatures.size())
		return creatures[stackpos];

	stackpos -= (unsigned char)creatures.size();

	if(stackpos < downItems.size())
		return downItems[stackpos];
    return NULL;
}

Item* Tile::getTopItem()
{
	if(topItems.size() > 0)
	{
		return topItems[topItems.size()-1];
	}
	if(downItems.size() > 0)
	{
		return downItems[downItems.size()-1];
	}
	if(ground)
    {
		return ground;
	}
    return NULL;
}

void Tile::transformThing(unsigned char stackpos, Thing *thing)
{
	Item* item = dynamic_cast<Item *>(thing);
	if(item == NULL)
		return;

	Item* oldItem = NULL;

	if(ground)
	{
		if(stackpos == 0)
		{
			oldItem = ground;
			ground = item;
		}

		--stackpos;
	}

	if(stackpos < (unsigned char)topItems.size())
	{
		ItemDeque::iterator it = topItems.begin();
		it += stackpos;
		stackpos = 0;

		oldItem = (*it);
		it = topItems.erase(it);
		topItems.insert(it, item);
	}

	stackpos -= (unsigned char)topItems.size();

	if(stackpos < (unsigned char)creatures.size())
	{
		return;
	}

	stackpos -= (unsigned char)creatures.size();

	if(stackpos < (unsigned char)downItems.size())
	{
		ItemDeque::iterator it = downItems.begin();
		it += stackpos;
		stackpos = 0;

		oldItem = (*it);
		it = downItems.erase(it);
		downItems.insert(it, item);
	}

	if(stackpos == 0)
	{
		delete oldItem;
	}
}

bool Tile::removeThing(unsigned char stackpos)
{
    Thing *thing = getThing(stackpos);
    if(!thing)
        return false;

    Creature* creature = dynamic_cast<Creature*>(thing);
	if(creature)
    {
		CreatureDeque::iterator it = std::find(creatures.begin(), creatures.end(), thing);

		if(it != creatures.end())
            creatures.erase(it);
	}
	else 
    {
		Item* item = dynamic_cast<Item*>(thing);
        if(!item)
            return false;

		if(item == ground) 
        {
            //delete ground;
			ground = NULL;
			return true;
		}

		ItemDeque::iterator iit;

		if(item->baseItem->isAlwaysOnTop())
        {
			for(iit = topItems.begin(); iit != topItems.end(); ++iit)
            {
				if(*iit == item)
                {
					//delete (*iit);
					topItems.erase(iit);
					return true;
				}
			}
		}
		else 
        {
			for (iit = downItems.begin(); iit != downItems.end(); ++iit)
            {
				if(*iit == item)
                {
                    //delete (*iit);
                    downItems.erase(iit);
					return true;
				}
			}
		}
	}
	return false;
}

bool Tile::removeTopItem(unsigned short count)
{
	if(topItems.size() > 0)
	{
		topItems.erase(topItems.end()-1);
		return true;
	}
	if(downItems.size() > 0)
	{
		downItems.erase(downItems.end()-1);
		return true;
	}
	if(ground)
    {
		ground = NULL;
		return true;
	}
    return false;
}

void Tile::removeCreatureByID(unsigned int _id)
{
    //CreatureVector::iterator it = creatures.begin();
    
   // while(it != creatures.end())
    //{
   //     if(_id)// == *it->id)
    //    {
    //        &(*it->id) = 0;
    //        creatures.erase(it);
     //   }
    //}
   //int temp = 0;
   for (deque<Creature*>::iterator it = creatures.begin(); it < creatures.end(); it++)
   {
	  if((*it) && (*it)->getId() == _id)//creatures[temp] && creatures[temp]->getId() == _id)
      {
		 creatures.erase(it);
         break;
      }
      //temp++;
   }    
}

sf::Packet Tile::returnTilePacket()
{
	sf::Packet toSend;
	int pid = 101;

	unsigned int itemCount = 0;
	if(ground)
		itemCount++;
	itemCount += topItems.size();
	itemCount += downItems.size();

	toSend << pid << pos.x << pos.y << pos.z << itemCount; //itemCount deprecated

    if(ground)
    {
	   toSend << ground->mID << ground->count;
    }
    for(unsigned int i = 0; i < topItems.size(); i++)
    {
		if(topItems[i]->mID > 1)
			toSend << topItems[i]->mID << topItems[i]->count;
    }
    for(unsigned int i = 0; i < downItems.size(); i++)
    {
		if(downItems[i]->mID > 1)
			toSend << downItems[i]->mID << downItems[i]->count;
	}
    return toSend;
}   

void Tile::getAllItems(std::deque<Item*> &items)
{
	if(ground)
		items.push_back(ground);

    for(unsigned int i = 0; i < topItems.size(); i++)
    {
		if(topItems[i]->mID > 1)
			items.push_back(topItems[i]);
    }
    for(unsigned int i = 0; i < downItems.size(); i++)
    {
		if(downItems[i]->mID > 1)
			items.push_back(downItems[i]);
	}
}
