#include <time.h>
#include <string>
#include <sstream>

#include "player.h"
#include "configManager.h"
#include "itemType.h"

extern ConfigManager ConfigManager;

Player::Player()
{
	id = currentId;
	currentId++;

	target = NULL;
	sex = 0;
	name = "Unnamed Player";
	accountName = "";
	access = 0;
	vocation = VOCATION_NONE;
	skillPoints = 0;
	statisticsPoints = 0;
	maxHealth = 100;
	health = maxHealth;
	maxMana = 25;
	mana = maxMana;
	level = 1;
	experience = 0;
	strength = 10;
	dexterity = 10;
	intelligence = 10;
	magicPower = 10;
	hungryTicks = 0;
	lastHungryChecked = 0;
	posX = 19;
	posY = 94;
	posZ = 7;
	spawnPosX = 19;
	spawnPosY = 94;
	spawnPosZ = 7;
	speed = 500;
	lightLevel = 9;
	nextLevelExperience = 100;
	startedWalking = 0;
	looktype = 8;
	corpseId = 140;
	direction = 2;
	itemStorage = new Container(278);
	this->lastActionTime = 0;
	this->offensiveSpellsCooldown = 0;
	this->defensiveSpellsCooldown = 0;

	this->equipment.armor = NULL;
	this->equipment.arrows = NULL;
	this->equipment.backpack = NULL;
	this->equipment.belt = NULL;
	this->equipment.boots = NULL;
	this->equipment.gloves = NULL;
	this->equipment.head = NULL;
	this->equipment.legs = NULL;
	this->equipment.lRing = NULL;
	this->equipment.rRing = NULL;
	this->equipment.lRingEar = NULL;
	this->equipment.rRingEar = NULL;
	this->equipment.necklace = NULL;
	this->equipment.shield = NULL;
	this->equipment.weapon = NULL;

	lastSpellAttack = 0;
}

Player::~Player()
{

}

void Player::calculateNextLevelExperience()
{
	this->nextLevelExperience = (50/3*(this->level+1)*(this->level+1)*(this->level+1) - 100*(this->level+1)*(this->level+1) + 850/3*(this->level+1) - 200);
}

void Player::onAdvance()
{
	this->level++;

	this->calculateNextLevelExperience();
	
	if(this->vocation == VOCATION_WARRIOR)
	{
		maxHealth += 25;
		maxMana += 5;
	}
	else if(this->vocation == VOCATION_ROGUE)
	{
		maxHealth+=15;
		maxMana+=15;
	}
	else if(this->vocation == VOCATION_MAGE || this->vocation == VOCATION_PRIEST)
	{
		maxHealth+=5;
		maxMana+=25;
	}

	if(this->level%5 == 0 && level <= 150)
		this->speed-=10;

	this->statisticsPoints+=3;
	this->skillPoints+=2;
	
	if(this->getExperience() >= this->getNextLevelExperience())
		this->onAdvance();
}

int Player::getFlag(unsigned int _key) //if failed, return -1
{
	for(unsigned int i = 0; i < flags.size(); i++)
	{
		if(flags.at(i).key == _key)
		{
			return flags.at(i).value;
		}
	}
	return -1;
}

bool Player::setFlag(unsigned int _key, int _value)
{
	for(unsigned int i = 0; i < flags.size(); i++)
		if(flags.at(i).key == _key)
		{
			flags.at(i).value = _value;
			return true;
		}

	flags.push_back(UniqueValue());
	flags.at(flags.size()-1).key = _key;
	flags.at(flags.size()-1).value = _value;
	return true;
}

std::string Player::serializeFlags()
{
	std::stringstream file;
	for(unsigned int i = 0; i < this->flags.size(); i++)
	{
		file << flags.at(i).key << "." << flags.at(i).value << "|";
	}
	return file.str();
}

std::string Player::serializeSpells()
{
	std::stringstream file;
	for(unsigned int i = 0; i < this->spells.size(); i++)
	{
		file << spells.at(i)->spellID << "|";
	}
	return file.str();
}

bool Player::canDoActionTime()
{
	if(this->lastActionTime <= (clock() - ConfigManager.playerActionInterval))
		return true;
	else
		return false;
}

void Player::setActionTime()
{
	this->lastActionTime = clock();
}

void Player::addMana(int _mana)
{
	mana += _mana;
		
	if(mana > maxMana)
		mana = maxMana;
	else if(mana <= 0)
		mana = 0;
}

void Player::addExperience(int _experience)
{
	if(this->experience + _experience > 0)
		this->experience += _experience;
	if(this->experience + _experience <= 0)
		this->experience = 0;
}

void Player::addLevel(int _level)
{
	if(this->level + _level > 0)
		this->level += _level;
	if(this->level + _level <= 0)
		this->level = 0;
}

unsigned int Player::getMaximumAttack(CombatType combatType)
{
	unsigned int attack = 1;

	if(this->equipment.weapon != NULL)
	{
		if(this->equipment.weapon->baseItem->shootRange > 1)
			attack += float(this->getDexterity()*this->equipment.weapon->baseItem->attack*0.1);
		else
			attack += float(this->getStrength()*this->equipment.weapon->baseItem->attack*0.1);
	}
	else 
		attack += float(this->getStrength()*0.1);

	return attack;
}

unsigned int Player::getMaximumSpellAttack(unsigned char spellID)
{
	if(spellID == 2) //testowe exori vis
		return (magicPower*3 + level*2);
	if(spellID == 3) //testowe exori
		return int(1.2*float(magicPower*3) + float(level*2));
	if(spellID == 4) //testowe exori flam
		return (magicPower*3 + level*2);
	if(spellID == 5) //testowe exevo flam
		return (magicPower*3 + level*2);
	if(spellID == 6) //testowe exana flam
		return int(1.5*float(magicPower*3 + level*2));
	if(spellID == 7) //testowe exana flam hur
		return (magicPower*3 + level*2);
	if(spellID == 8) //testowe exana vis hur
		return (magicPower*3 + level*2);
	if(spellID == 9) //testowe exori gran vis
		return 2*(magicPower*3 + level*2);
	if(spellID == 10) //testowe exevo gran mas vis
		return 3*(magicPower*3 + level*2);

	return 1;
}

unsigned int Player::getTotalBasicArmor(CombatType combatType)
{
	unsigned int armor = 0;

	if(this->equipment.head)
		armor += this->equipment.head->baseItem->armor;
	if(this->equipment.armor)
		armor += this->equipment.armor->baseItem->armor;
	if(this->equipment.legs)
		armor += this->equipment.legs->baseItem->armor;
	if(this->equipment.belt)
		armor += this->equipment.belt->baseItem->armor;
	if(this->equipment.gloves)
		armor += this->equipment.gloves->baseItem->armor;
	if(this->equipment.boots)
		armor += this->equipment.boots->baseItem->armor;
	
	return armor;
}

unsigned int Player::getTotalBasicDefence(CombatType combatType)
{
	unsigned int defence = 0;

	if(this->equipment.shield != NULL)
		defence += this->equipment.shield->baseItem->defence;

	return defence;
}

unsigned int Player::getDefenceModificator(CombatType combatType) //w procentach
{
	return 0;
}

bool Player::hasSpell(unsigned char spellID)
{
	for(unsigned int it = 0; it < spells.size(); it++)
	{
		if(spells[it]->spellID == spellID)
			return true;
	}
	return false;
}

bool Player::addSpell(unsigned char spellID, unsigned int cooldown)
{
	if(hasSpell(spellID))
		return false;

	spells.push_back(new MonsterSpell());
	spells.back()->cooldown = cooldown;
	spells.back()->spellID = spellID;
	spells.back()->lastUsed = 0;
}