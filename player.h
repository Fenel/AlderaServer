#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Network.hpp>
#include <deque>
#include "container.h"
#include "creature.h"
#include "enums.h"

using namespace std;
class NPC;

class Player : public Creature
{
protected:
 unsigned int nextLevelExperience;
 unsigned int strength, dexterity, intelligence, magicPower;
 unsigned int mana, maxMana;
 unsigned int skillPoints;
 unsigned int statisticsPoints;
 Vocation vocation;
 string accountName;
 std::deque<UniqueValue> flags;
 unsigned int hungryTicks;
 clock_t lastHungryChecked;
public: 
	char lastNPCcondition;
 Player();
 ~Player();
 bool sex;
 unsigned char access;
 sf::TcpSocket *socket;
 PlayerEquipment equipment;
 Container *itemStorage;
 NPC *npcTalked;
 void calculateNextLevelExperience();
 void onAdvance();
 bool canDoActionTime();
 void setActionTime();
 Vocation getVocation() { return vocation; }
 string getAccountName(){ return accountName; }
 unsigned int getNextLevelExperience() { return nextLevelExperience; };
 unsigned int getMana()		{ return mana; }
 unsigned int getMaxMana()	{ return maxMana; }
 unsigned int getStatisticsPoints() { return statisticsPoints; }
 unsigned int getSkillPoints()		{ return skillPoints; }
 unsigned int getStrength()		 { return strength; }
 unsigned int getDexterity()     { return dexterity; }
 unsigned int getIntelligence()  { return intelligence; }
 unsigned int getMagicPower()    { return magicPower; }
 int getFlag(unsigned int _key);
 bool hasFlags() { if(flags.empty()) return false; else return true; };
 bool hasSpell(unsigned char spellID);
 bool addSpell(unsigned char spellID, unsigned int cooldown);
 unsigned int getHungryTicks() { return hungryTicks; };
 clock_t getLastHungryChecked() { return lastHungryChecked; }

 void setVocation(Vocation _vocation)					    { vocation = _vocation; }
 void setAccountName(string _accountName)					{ accountName = _accountName; }
 void setMana(unsigned int _mana)                           { mana = _mana;	}
 void setMaxMana(unsigned int _mana)                        { maxMana = _mana;	}
 void setStatisticsPoints(unsigned int _points)             { statisticsPoints = _points; }
 void setSkillPoints(unsigned int _points)					{ skillPoints = _points; }
 void setStrength(unsigned int _strength)					{ strength = _strength; }
 void setDexterity(unsigned int _dexterity)					{ dexterity = _dexterity; }
 void setIntelligence(unsigned int _intelligence)			{ intelligence = _intelligence; }
 void setMagicPower(unsigned int _magicPower)				{ magicPower = _magicPower; }
 void setSocket(sf::TcpSocket *_socket)						{ socket = _socket; }
 void setHungryTicks(unsigned int _ticks)					{ hungryTicks = _ticks; }
 void setLastHungryChecked()								{ lastHungryChecked = clock(); }
 bool setFlag(unsigned int _key, int _value);
 std::string serializeFlags();
 std::string serializeSpells();

 void addStrength(int _strength)     { strength += _strength; }
 void addDexterity(int _dexterity)     { dexterity += _dexterity; }
 void addIntelligence(int _intelligence)     { intelligence += _intelligence; }
 void addMagicPower(int _magicPower)     { magicPower += _magicPower; }
 void addMana(int _mana);
 void addExperience(int _experience);
 void addLevel(int _level);
 void addHungryTicks(int _ticks) { hungryTicks += _ticks; };

 unsigned int getMaximumAttack(CombatType combatType = COMBAT_PHYSICALDAMAGE);
 unsigned int getMaximumSpellAttack(unsigned char spellID);
 unsigned int getTotalBasicArmor(CombatType combatType = COMBAT_PHYSICALDAMAGE);
 unsigned int getTotalBasicDefence(CombatType combatType = COMBAT_PHYSICALDAMAGE);
 unsigned int getDefenceModificator(CombatType combatType = COMBAT_PHYSICALDAMAGE);//w procentach

 clock_t lastActionTime;
 clock_t offensiveSpellsCooldown;
 clock_t defensiveSpellsCooldown;

 void setOffensiveSpellsCooldown() { offensiveSpellsCooldown = clock(); };
 void setDefensiveSpellsCooldown() { defensiveSpellsCooldown = clock(); };
 bool isOffensiveSpellsCooldown() { if(offensiveSpellsCooldown + 2000 > clock()) return true; else return false; };
 bool isDefensiveSpellsCooldown() { if(defensiveSpellsCooldown + 1000 > clock()) return true; else return false; };
};


#endif
