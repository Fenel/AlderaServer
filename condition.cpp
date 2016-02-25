/*
#include "condition.h"
#include "game.h"
#include "creature.h"

extern Game Game;

Condition::Condition()
{
	speedChange = 0;
	attackChange = 0;
	defenceChange = 0;
	armorChange = 0; 
}

void Condition::setSpeedChange(char change, unsigned int expires)
{
	speedChange = change;
	speedChangeExpires = clock() + expires;
}

void Condition::setAttackChange(char change, unsigned int expires)
{
	attackChange = change;
	attackChangeExpires = clock() + expires;
}

void Condition::setDefenceChange(char change, unsigned int expires)
{
	defenceChange = change;
	defenceChangeExpires = clock() + expires;
}

void Condition::setArmorChange(char change, unsigned int expires)
{
	armorChange = change;
	armorChangeExpires = clock() + expires;
}

char Condition::getSpeedChange() 
{ 
	if(speedChange != 0 && speedChangeExpires < clock())
	{
		speedChange = 0;
		Game.sendCreatureUpdateSpeed(parent);
	}
	return speedChange; 
}

char Condition::getAttackChange()
{
	if(attackChange != 0 && attackChangeExpires < clock())
		attackChange = 0;
	return attackChange; 
}

char Condition::getDefenceChange()
{
	if(defenceChange != 0 && defenceChangeExpires < clock())
		defenceChange = 0;
	return defenceChange;
}

char Condition::getArmorChange()
{
	if(armorChange != 0 && armorChangeExpires < clock())
		armorChange = 0;
	return armorChange;
}
*/