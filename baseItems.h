#ifndef __BASEITEMS_H__
#define __BASEITEMS_H__

#include "itemType.h"


class BaseItems
{
public:
	BaseItems();
	~BaseItems()
    {
    }
	void load();
	ItemType* getItemType(unsigned int mID);
    int itemsCount;
private:
	std::vector<ItemType> items;
};
    
#endif
