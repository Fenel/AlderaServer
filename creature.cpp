#include <deque>

#include "thing.h"
#include "creature.h"

Creature::Creature()
{
	id = currentId;
	currentId++;
	meleeCycle = 1500;
	corpseId = 140;

	pathPoints.clear();
	target = NULL;
	targetLastX = 0;
	targetLastY = 0;

	lastSpellAttack = 0;

	health = maxHealth = 500;
	direction = 2;
	speed = 1000;
	lightLevel = 5;

	//condition = new Condition();
	//condition->parent = this;
}

unsigned int Creature::currentId = 1;
	
Creature::~Creature()
{
      id = 0;
	  //delete condition;
}

void Creature::setPos(unsigned int _posX, unsigned int _posY, unsigned char _posZ)
{
	posX = _posX;
	posY = _posY;
	posZ = _posZ;
}

unsigned int Creature::getSpeed()		
 {
	 /*
	 if(condition->getSpeedChange() == 0)
		return speed; 
	 else
		return static_cast<unsigned int>(int(speed)+(int(speed)*condition->getSpeedChange()/100.0f));
	*/
	 return speed; 
 }

void Creature::setStartedWalking()
{
    startedWalking = clock();
}

void Creature::setLastMeleeAttack()
{
    lastMeleeAttack = clock();
}

bool Creature::isWalking()
{
	/*
	if(condition->getSpeedChange() == 0 && startedWalking + speed <= clock())
		return false;
	else if(condition->getSpeedChange() != 0 && startedWalking + speed + int(speed)*condition->getSpeedChange()/100.0f <= clock())
		return false;
	else
		return true;
	*/
	if(startedWalking + speed <= clock())
		return false;
	else
		return true;
}

bool Creature::isMeleeAttacking()
{
	if(lastMeleeAttack + meleeCycle <= clock())
		return false;
	else
		return true;
}

unsigned char Creature::getHealthPercent()
{
	unsigned char result = static_cast<unsigned char>((float(health)/float(maxHealth))*100.0f);
	return result;
}
 
void Creature::addHealth(int _health)
{
	health += _health;
		
	if(health > maxHealth)
		health = maxHealth;
	else if(health <= 0)
		health = 0;
}

void Creature::addPosX(int _posX)
{
	posX += _posX;
}

void Creature::addPosY(int _posY)
{
	posY += _posY;
}

void Creature::addPosZ(int _posZ)
{
	posZ += _posZ;
}

void Creature::addDamageToDamageList(unsigned int _damage, std::string _attackerName)
{
	bool inserted = false;
	for (deque<ReceivedDamageFrom>::iterator it = attackersList.begin(); it < attackersList.end(); it++)
	{
		if(&(*it) && it->dealer == _attackerName)
		{
			it->damage += _damage;
			inserted = true;
		}
	}
	if(!inserted)
	{
		ReceivedDamageFrom damagedd;
		damagedd.dealer = _attackerName;
		damagedd.damage = _damage;
		attackersList.push_back(damagedd);
	}
}

void Creature::setLastSpellAttack(unsigned char spellID)
{
	for(unsigned int it = 0; it < spells.size(); it++)
	{
		if(spells[it]->spellID == spellID)
			spells[it]->lastUsed = clock();
	}
}

bool Creature::isSpellAttacking(unsigned char spellID)
{
	for(unsigned int it = 0; it < spells.size(); it++)
	{
		if(spells[it]->spellID == spellID)
		{
			if(spells[it]->lastUsed + spells[it]->cooldown <= clock())
				return false;
			else
				return true;
		}
	}
	return false;
}