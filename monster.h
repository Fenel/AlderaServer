#ifndef MONSTER_H
#define MONSTER_H

#include "creature.h"
#include "item.h"
#include "enums.h"
#include <time.h>
#include <deque>

class Monster : public Creature
{
public:
 Monster();
 ~Monster();
 unsigned int getRespawnTime() { return respawnTime; };
 void setRespawnTime(int _respawnTime);
 void setMeleeMaxDamage(unsigned int _damage){ meleeMaxDamage = _damage; };
 void setDistanceMaxDamage(unsigned int _damage){ distanceMaxDamage = _damage; };
 void setDistanceRange(unsigned char _range){ distanceRange = _range; };
 void setShootType(unsigned char _type){ shootType = AmmoType(_type); };
 void setDefence(unsigned int _defence){ defence = _defence; };
 void setArmor(unsigned int _armor){ armor = _armor; };
 void setLastShooted(){ lastShooted = clock(); };
 void setShootCooldown(unsigned int cooldown){ shootCooldown = cooldown; };

 unsigned int getSpawnPosX(){ return spawnPosX; };
 unsigned int getSpawnPosY(){ return spawnPosY; };
 unsigned char getSpawnPosZ(){ return spawnPosZ; };
 unsigned int getDistanceMaxDamage() { return distanceMaxDamage; };
 unsigned char getDistanceRange(){ return distanceRange; };
 AmmoType getShootType(){ return shootType; };
 clock_t getLastShooted(){ return lastShooted; };
 bool canShoot(){ if(lastShooted + shootCooldown < clock()) return true; else return false; };

 unsigned int getMaximumAttack(CombatType combatType = COMBAT_PHYSICALDAMAGE);
 unsigned int getMaximumSpellAttack(unsigned char spellID);
 unsigned int getTotalBasicArmor(CombatType combatType = COMBAT_PHYSICALDAMAGE);
 unsigned int getTotalBasicDefence(CombatType combatType = COMBAT_PHYSICALDAMAGE);
 unsigned int getDefenceModificator(CombatType combatType = COMBAT_PHYSICALDAMAGE);//in percent

 std::deque<Item*> items;
 bool death;
 clock_t deathTime;
private:
 unsigned int respawnTime;
 unsigned int meleeMaxDamage;
 unsigned int distanceMaxDamage;
 unsigned char distanceRange;
 AmmoType shootType;
 clock_t lastShooted;
 unsigned int shootCooldown;
 unsigned int defence;
 unsigned int armor;
};



#endif