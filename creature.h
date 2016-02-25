#ifndef CREATURE_H
#define CREATURE_H

#include <string>
#include <deque>
#include <time.h>

#include "thing.h"
#include "enums.h"
#include "pathFind.h"
//#include "condition.h"

using namespace std;

class Creature : public Thing
{
 public:
 Creature();
 ~Creature();

 string getName()				{ return name; };
 unsigned int getId()			{ return id; };
 unsigned int getHealth()		{ return health; };
 unsigned int getMaxHealth()	{ return maxHealth; };
 unsigned int getLevel()		{ return level; };
 unsigned int getExperience()   { return experience; };
 unsigned int getPosX()			{ return posX; };
 unsigned int getPosY()			{ return posY; }
 unsigned char getPosZ()		{ return posZ; };
 unsigned char getLooktype()	{ return looktype; };
 unsigned short getCorpseId()	{ return corpseId; };
 unsigned char getDirection()	{ return direction; };
 clock_t getStartedWalking()	{ return startedWalking; };
 clock_t getLastMeleeAttack()   { return lastMeleeAttack; };
 unsigned int getSpeed();		

 unsigned char getLightLevel()  { return lightLevel; };
 Creature *getTarget()			{ return target; };
 
 void setId(unsigned int _id)                                { id = _id; };
 void setName(string _name)									{ name = _name; };
 void setPos(unsigned int _posX, unsigned int _posY, unsigned char _posZ);
 void setPosX(unsigned int _posX)							{ posX = _posX; };
 void setPosY(unsigned int _posY)							{ posY = _posY; };
 void setPosZ(unsigned char _posZ)							{ posZ = _posZ; };
 void setLooktype(unsigned char _looktype)					{ looktype = _looktype; };
 void setCorpseId(unsigned short _corpse)					{ corpseId = _corpse; };
 void setDirection(unsigned char _direction)				{ direction = _direction; };
 void setHealth(unsigned int _health)						{ health = _health; };
 void setMaxHealth(unsigned int _maxHealth)					{ maxHealth = _maxHealth; };
 void setLevel(unsigned int _level)							{ level = _level; };
 void setExperience(unsigned int _experience)				{ experience = _experience; };
 void setSpeed(unsigned int _speed)							{ speed = _speed; };
 void setLightLevel(unsigned char _lightLevel)				{ lightLevel = _lightLevel; };
 void setTarget(Creature *_target)							{ target = _target; };
 void setStartedWalking();
 void setLastMeleeAttack();

 bool isWalking();
 bool isMeleeAttacking();
 unsigned char getHealthPercent();
 
 void addHealth(int _health);
 void addPosX(int _posX);
 void addPosY(int _posY);
 void addPosZ(int _posZ);
 void addDamageToDamageList(unsigned int _damage, std::string _attackerName);
 void clearAttackersList(){ this->attackersList.clear(); };
 std::deque<ReceivedDamageFrom> *getAttackersList(){ return &(this->attackersList); };

 virtual unsigned int getMaximumAttack(CombatType combatType){	return 1;	};
 virtual unsigned int getMaximumSpellAttack(unsigned char spellID){	return 1;	};
 virtual unsigned int getTotalBasicArmor(CombatType combatType){	return 0;	};
 virtual unsigned int getTotalBasicDefence(CombatType combatType){	return 0;	};
 virtual unsigned int getDefenceModificator(CombatType combatType){	   return 0;	};

 unsigned int spawnPosX, spawnPosY;
 unsigned char spawnPosZ;
 std::list<Point> pathPoints;
 unsigned int targetLastX, targetLastY;
 unsigned char targetLastZ;

 std::deque<MonsterSpell*> spells;
 bool isSpellAttacking(unsigned char spellID);
 void setLastSpellAttack(unsigned char spellID);

 //Condition *condition;
 unsigned int baseSpeed;
 clock_t speedConditionExpires;
protected:
 unsigned int id;
 std::string name;

 unsigned char looktype, direction;
 unsigned short corpseId;
 unsigned int posX, posY;
 unsigned char posZ;
 int health, maxHealth;
 unsigned int level;
 unsigned int experience;
 unsigned int speed;
 unsigned char lightLevel;
 Creature *target;
 std::deque<ReceivedDamageFrom> attackersList;

 clock_t startedWalking;
 clock_t lastMeleeAttack;
 unsigned int meleeCycle;
 clock_t lastSpellAttack;

 static unsigned int currentId;
};

#endif //__CREATURE_H__
