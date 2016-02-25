#include "container.h"
#include "game.h"
#include "player.h"

extern Game Game;

Container::Container(unsigned int _type) : Item(_type)
{
	maxSize = 30;////////////////////
	totalWeight = 0;
}

Container::~Container()
{
	itemList.clear();
}

Container* Container::getParentContainer() 
{
	if(parent && dynamic_cast<Container*>(parent))
	{
		return dynamic_cast<Container*>(parent);
	}
	return NULL;
}

void Container::addItem(Item* item)
{
	item->parent = this;
	itemList.push_back(item);
}

Item* Container::getItem(unsigned char index)
{
	size_t n = 0;			
	for (ItemList::const_iterator cit = getItems(); cit != getEnd(); ++cit) 
	{
		if(n == index)
			return *cit;
		else
			++n;
	}
	return NULL;
}

Item* Container::getItemId(unsigned int id, unsigned short count)
{
	std::list<Item*>::iterator minCount;
	unsigned short minimumCount = 1337;
	for(std::list<Item*>::iterator cit = itemList.begin(); cit != itemList.end(); ++cit)
	{
		if((*cit)->mID == id && minimumCount > (*cit)->count)
		{
			minCount = cit;
			minimumCount = (*cit)->count;
		}
	}
	if(minimumCount != 1337)
		return *minCount;
	else
		return NULL;
}

unsigned char Container::getItemHoldingCount()
{
	unsigned char counter = 0;

	for(std::list<Item*>::const_iterator iter = itemList.begin(); iter != itemList.end(); ++iter)
	{
		++counter;
	}
	return counter;
}

bool Container::isHoldingItem(Item* item)
{
	for(std::list<Item*>::const_iterator cit = itemList.begin(); cit != itemList.end(); ++cit)
	{
		if(*cit == item)
			return true;
	}
	return false;
}

bool Container::isHoldingItemId(unsigned int id, unsigned short count)
{
	for(std::list<Item*>::const_iterator cit = itemList.begin(); cit != itemList.end(); ++cit)
	{
		if((*cit)->mID == id && (*cit)->count >= count)
			return true;
	}
	return false;
}

void Container::eraseItem(unsigned char index)
{
	size_t n = 0;			
	for (ItemList::const_iterator cit = getItems(); cit != getEnd(); ++cit) 
	{
		if(n == index)
		{
			itemList.erase(cit);
			break;
		}
		else
			++n;
	}
}

void Container::eraseItem(Item *item)
{			
	for (ItemList::const_iterator cit = getItems(); cit != getEnd(); ++cit) 
	{
		if(((*cit)->mID == item->mID) && ((*cit)->count == item->count))
		{
			itemList.erase(cit);
			break;
		}
	}
}

void Container::eraseItemId(unsigned int id, unsigned short count)
{			
	for (ItemList::const_iterator cit = getItems(); cit != getEnd(); ++cit) 
	{
		if((*cit)->mID == id)
		{
			if((*cit)->count == count)
				itemList.erase(cit);
			else if((*cit)->count > count)
				(*cit)->count -= count;
			break;
		}
	}
}

unsigned char Container::getItemIndex(Item *item)
{		
	unsigned char index = 0;
	for (ItemList::const_iterator cit = getItems(); cit != getEnd(); ++cit) 
	{
		if(((*cit) == item))
		{
			return index;
		}
		index++;
	}
	return 255;
}