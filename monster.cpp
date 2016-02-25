#include "monster.h"


Monster::Monster()
{
	id = currentId;
	currentId++;

	name = "Unnamed Monster";
	maxHealth = 10;
	health = maxHealth;
	experience = 0;
    level = 1;
	posX = 19;
	posY = 22;
	posZ = 7;
	speed = 500;
	lightLevel = 5;
	startedWalking = 0;
	looktype = 0;
	corpseId = 152;
	direction = 2;
	respawnTime = 60;
	meleeMaxDamage = 0;
	defence = 0;
	armor = 0;
	death = false;
	deathTime = 0;
	target = NULL;

	distanceMaxDamage = 0;
	distanceRange = 1;
	shootType = AMMO_NONE;
	shootCooldown = 1500;
	lastShooted = 0;

	lastSpellAttack = 0;
}

Monster::~Monster()
{

}

void Monster::setRespawnTime(int _respawnTime)
{
	respawnTime = _respawnTime;
}

unsigned int Monster::getMaximumAttack(CombatType combatType)
{
	if(lastMeleeAttack + meleeCycle > clock())
		return distanceMaxDamage;
	else
		return meleeMaxDamage;
}

unsigned int Monster::getMaximumSpellAttack(unsigned char spellID)
{
	for(unsigned int it = 0; it < spells.size(); it++)
		if(spells.at(it)->spellID == spellID)
			return spells.at(it)->damage;
	return 1;
}

unsigned int Monster::getTotalBasicArmor(CombatType combatType)
{
	return armor;
}

unsigned int Monster::getTotalBasicDefence(CombatType combatType)
{
	return defence;
}

unsigned int Monster::getDefenceModificator(CombatType combatType) //w procentach
{
	return 0;
}