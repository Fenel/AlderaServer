/*
#ifndef __CONDITION_H__
#define __CONDITION_H__

#include <time.h>

class Creature;

class Condition
{
public:
	Condition();
	~Condition()
    {
    }
	Creature *parent;
	void setSpeedChange(char change, unsigned int expires);
	void setAttackChange(char change, unsigned int expires);
	void setDefenceChange(char change, unsigned int expires);
	void setArmorChange(char change, unsigned int expires);
	char getSpeedChange();
	char getAttackChange();
	char getDefenceChange();
	char getArmorChange();
private:
	char speedChange; //speed change in percents
	char attackChange; //attack change in percents
	char defenceChange; //defence change in percents
	char armorChange; //armor change in percents

	clock_t speedChangeExpires;
	clock_t attackChangeExpires;
	clock_t defenceChangeExpires;
	clock_t armorChangeExpires;
};
    
#endif
*/