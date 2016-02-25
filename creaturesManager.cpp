#include <deque>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "player.h"
#include "monster.h"
#include "map.h"
#include "container.h"
#include "creaturesManager.h"
#include "game.h"

extern Game Game;
extern Map Map;
using namespace std;

CreaturesManager::CreaturesManager()
{
}

bool CreaturesManager::loadPlayer(string playerName)
{
	if(!this->getPlayer(playerName))
	{
		deque<std::string> lines;
		deque<std::string> names;
		deque<std::string> values;
		string tmp;

		std::string extension = ".pla";
		std::string filename = "Players\\";
		filename += playerName;
		filename += extension;

		std::ifstream file(filename.c_str());
		if(file.is_open())
		{
		   while(file)
		   {
			  tmp.clear();
			  getline(file, tmp,'\n');
			  lines.push_back(tmp);
		   }   
		   file.close(); 
		}
		else
			return false;

		for(unsigned int i = 0; i < lines.size(); i++)
		{
			if(lines[i].find_first_of("=") != string::npos)
			{
				string name = lines[i];
				name.erase(name.find_first_of("="));
				names.push_back(name);

				string value = lines[i];
				value.erase(0,value.find_first_of("=")+1);
				values.push_back(value);
			}
		}

		playersList.push_back(Player());/////////////////////////////////////////////
		playersList[playersList.size()-1].setCorpseId(140);
		playersList[playersList.size()-1].setLastMeleeAttack();

		for(unsigned int i = 0; i < names.size(); i++)
		{
			if(names[i] == "name")
				playersList[playersList.size()-1].setName(values[i]);	
			if(names[i] == "accountName")
				playersList[playersList.size()-1].setAccountName(values[i]);	
			if(names[i] == "access")
				playersList[playersList.size()-1].access = char(atoi(values[i].c_str()));	
			if(names[i] == "sex")
				playersList[playersList.size()-1].sex = atoi(values[i].c_str());
			if(names[i] == "looktype")
				playersList[playersList.size()-1].setLooktype(atoi(values[i].c_str()));
			if(names[i] == "vocation")
				playersList[playersList.size()-1].setVocation(Vocation(atoi(values[i].c_str())));
			if(names[i] == "posX")
				playersList[playersList.size()-1].setPosX(atoi(values[i].c_str()));
			if(names[i] == "posY")
				playersList[playersList.size()-1].setPosY(atoi(values[i].c_str()));
			if(names[i] == "posZ")
				playersList[playersList.size()-1].setPosZ(atoi(values[i].c_str()));
			if(names[i] == "health")
				playersList[playersList.size()-1].setHealth(atoi(values[i].c_str()));
			if(names[i] == "maxHealth")
				playersList[playersList.size()-1].setMaxHealth(atoi(values[i].c_str()));
			if(names[i] == "mana")
				playersList[playersList.size()-1].setMana(atoi(values[i].c_str()));
			if(names[i] == "maxMana")
				playersList[playersList.size()-1].setMaxMana(atoi(values[i].c_str()));
			if(names[i] == "skillPoints")
				playersList[playersList.size()-1].setSkillPoints(atoi(values[i].c_str()));
			if(names[i] == "statisticsPoints")
				playersList[playersList.size()-1].setStatisticsPoints(atoi(values[i].c_str()));
			if(names[i] == "level")
				playersList[playersList.size()-1].setLevel(atoi(values[i].c_str()));
				playersList[playersList.size()-1].calculateNextLevelExperience();
			if(names[i] == "experience")
				playersList[playersList.size()-1].setExperience(atoi(values[i].c_str()));
			if(names[i] == "speed")
			{
				playersList[playersList.size()-1].setSpeed(atoi(values[i].c_str()));
				playersList[playersList.size()-1].baseSpeed = playersList[playersList.size()-1].getSpeed();
				playersList[playersList.size()-1].speedConditionExpires = 0;
			}
			if(names[i] == "strength")
				playersList[playersList.size()-1].setStrength(atoi(values[i].c_str()));
			if(names[i] == "dexterity")
				playersList[playersList.size()-1].setDexterity(atoi(values[i].c_str()));
			if(names[i] == "intelligence")
				playersList[playersList.size()-1].setIntelligence(atoi(values[i].c_str()));
			if(names[i] == "magicPower")
				playersList[playersList.size()-1].setMagicPower(atoi(values[i].c_str()));
			if(names[i] == "hungryTicks")
				playersList[playersList.size()-1].setHungryTicks(atoi(values[i].c_str()));
			if(names[i] == "lRingEar")
				playersList[playersList.size()-1].equipment.lRingEar = new Item(values[i]);
			if(names[i] == "rRingEar")
				playersList[playersList.size()-1].equipment.rRingEar = new Item(values[i]);
			if(names[i] == "head")
				playersList[playersList.size()-1].equipment.head = new Item(values[i]);
			if(names[i] == "necklace")  
				playersList[playersList.size()-1].equipment.necklace = new Item(values[i]);
			if(names[i] == "backpack")
				playersList[playersList.size()-1].equipment.backpack = new Item(values[i]);
			if(names[i] == "armor")
				playersList[playersList.size()-1].equipment.armor = new Item(values[i]);
			if(names[i] == "shield")
				playersList[playersList.size()-1].equipment.shield = new Item(values[i]);
			if(names[i] == "weapon")
				playersList[playersList.size()-1].equipment.weapon = new Item(values[i]);
			if(names[i] == "belt")
				playersList[playersList.size()-1].equipment.belt = new Item(values[i]);
			if(names[i] == "legs")
				playersList[playersList.size()-1].equipment.legs = new Item(values[i]);
			if(names[i] == "boots")
				playersList[playersList.size()-1].equipment.boots = new Item(values[i]);
			if(names[i] == "lRing")
				playersList[playersList.size()-1].equipment.lRing = new Item(values[i]);
			if(names[i] == "rRing")
				playersList[playersList.size()-1].equipment.rRing = new Item(values[i]);
			if(names[i] == "gloves")
				playersList[playersList.size()-1].equipment.gloves = new Item(values[i]);
			if(names[i] == "arrows")
				playersList[playersList.size()-1].equipment.arrows = new Item(values[i]);
			if(names[i] == "itemStorage")
			{
				std::string token, charek;
				for(unsigned int n = 0; n < values[i].length(); n++)
				{
					charek = values[i].at(n);
					if(charek != "|")
					{
						token += charek;
					}
					if(charek == "|")
					{    
						playersList[playersList.size()-1].itemStorage->addItem(new Item(token));
						token.clear();
					}    
				}
			}
			if(names[i] == "flags")
			{
				std::string token, charek;
				unsigned int key = 0;
				for(unsigned int n = 0; n < values[i].length(); n++)
				{
					charek = values[i].at(n);
					if(charek != "|" && charek != ".")
					{
						token += charek;
					}
					if(charek == ".")
					{
						key = atoi(token.c_str());
						token.clear();
					}
					if(charek == "|")
					{    
						playersList[playersList.size()-1].setFlag(key, atoi(token.c_str()));
						token.clear();
					}    
				}
			}
			if(names[i] == "spells")
			{
				std::string token, charek;
				unsigned char spellID = 0;
				for(unsigned int n = 0; n < values[i].length(); n++)
				{
					charek = values[i].at(n);
					if(charek != "|")
					{
						token += charek;
					}
					if(charek == "|")
					{    
						spellID = atoi(token.c_str());
						playersList[playersList.size()-1].spells.push_back(new MonsterSpell());
						playersList[playersList.size()-1].spells.back()->spellID = spellID;
						playersList[playersList.size()-1].spells.back()->lastUsed = 0;
						
						if(spellID == 1)
							playersList[playersList.size()-1].spells.back()->cooldown = 1000;
						else if(spellID == 2 || spellID == 3 || spellID == 6 || spellID == 9)
							playersList[playersList.size()-1].spells.back()->cooldown = 2000;
						else if(spellID == 3 || spellID == 4 || spellID == 5 || spellID == 7 || spellID == 8 || spellID == 10)
							playersList[playersList.size()-1].spells.back()->cooldown = 3000;
						else
							playersList[playersList.size()-1].spells.back()->cooldown = 2000;

						token.clear();
					}    
				}
			}
		}	
		return true;
	}
	return false;
}

bool CreaturesManager::loadMonster(string monsterName, bool reload, unsigned int id)
{
	vector<std::string> lines;
	vector<std::string> names;
	vector<std::string> values;
	string tmp;

    std::string extension = ".mon";
    std::string filename = "Monsters\\";
    filename += monsterName;
    filename += extension;

	std::ifstream file(filename.c_str());
    if(file.is_open())
    {
       while(file)
       {
	      tmp.clear();
          getline(file, tmp,'\n');
		  if(!tmp.empty())
             lines.push_back(tmp);
       }   
       file.close(); 
	}
	else
		return false;

	for(unsigned int i = 0; i < lines.size(); i++)
	{
	   string name = lines[i];
	   name.erase(name.find_first_of("="));
	   names.push_back(name);

	   string value = lines[i];
	   value.erase(0,value.find_first_of("=")+1);
	   values.push_back(value);
	}


	Monster *monster = NULL;
	if(!reload)
	{
		monstersList.push_back(Monster());
		monster = &monstersList.back();
	}
	else
	{
		monster = this->getMonster(id);
	}
	monster->setLastMeleeAttack();

	for(unsigned int i = 0; i < names.size(); i++)
	{
		if(names[i] == "name")
			monster->setName(values[i]);	
		if(names[i] == "looktype")
		{
			monster->setLooktype(atoi(values[i].c_str()));
		}
		if(names[i] == "corpse")
		{
			monster->setCorpseId(atoi(values[i].c_str()));
		}
		if(names[i] == "health")
			monster->setHealth(atoi(values[i].c_str()));
		if(names[i] == "maxHealth")
			monster->setMaxHealth(atoi(values[i].c_str()));
		if(names[i] == "level")
			monster->setLevel(atoi(values[i].c_str()));
		if(names[i] == "experience")
			monster->setExperience(atoi(values[i].c_str()));
		if(names[i] == "speed")
		{
			monster->setSpeed(atoi(values[i].c_str()));
			monster->baseSpeed = monster->getSpeed();
			monster->speedConditionExpires = 0;
		}
		if(names[i] == "meleeMaxDamage")
			monster->setMeleeMaxDamage(atoi(values[i].c_str()));
		if(names[i] == "distanceMaxDamage")
			monster->setDistanceMaxDamage(atoi(values[i].c_str()));
		if(names[i] == "distanceRange")
			monster->setDistanceRange(atoi(values[i].c_str()));
		if(names[i] == "shootType")
			monster->setShootType(atoi(values[i].c_str()));
		if(names[i] == "shootCooldown")
			monster->setShootCooldown(atoi(values[i].c_str()));
		if(names[i] == "defence")
			monster->setDefence(atoi(values[i].c_str()));
		if(names[i] == "armor")
			monster->setArmor(atoi(values[i].c_str()));
		if(names[i] == "loot")
		{
			std::string token, charek;
			for(unsigned int n = 0; n < values[i].length(); n++)
			{
				charek = values[i].at(n);
				if(charek != "|")
				{
					token += charek;
				}
				if(charek == "|")
				{    
					charek = values[i].erase(0, values[i].find_last_of("=")+1);
					unsigned int chance = rand() % 100000;
					if(chance <= atoi(charek.c_str()))
					{
						monster->items.push_back(new Item(token));
						monster->items.at(monster->items.size()-1)->randomize();
					}
					token.clear();
				}    
			}
		}
		if(names[i] == "spell")
		{
			std::string token, charek;
			for(unsigned int n = 0; n < values[i].length(); n++)
			{
				charek = values[i].at(n);
				if(charek != "|" && charek != "." && charek != "~")
				{
					token += charek;
				}
				if(charek == ".")
				{
					monster->spells.push_back(new MonsterSpell());
					monster->spells.back()->spellID = atoi(token.c_str());
					token.clear();
				}
				if(charek == "~")
				{
					monster->spells.back()->damage = atoi(token.c_str());
					token.clear();
				}
				if(charek == "|")
				{    
					monster->spells.back()->cooldown = atoi(token.c_str());
					token.clear();
					token = values[i].erase(0, values[i].find_last_of("=")+1);
					monster->spells.back()->chance = atoi(token.c_str());
					token.clear();
					break;
				} 
			}
		}
	}
	
	return true;
}

bool CreaturesManager::loadNPC(string name, string script)
{
	NPC npc(name);
	if(npc.parseScript(script))
	{
		this->NPCList.push_back(npc);
		return true;
	}
	else
		return false;
}

void CreaturesManager::deathCreature(unsigned int id)
{
	Monster *monster = this->getMonster(id);
	if(monster && monster->getRespawnTime() != 0 && this->loadMonster(monster->getName(),true,id))
	{
		monster->setPos(monster->spawnPosX, monster->spawnPosY, monster->spawnPosZ);
		monster->clearAttackersList();
		monster->death = true;
		monster->deathTime = clock();
	}
	else if(monster && monster->getRespawnTime() == 0)
		this->removeCreature(monster->getId());
}

void CreaturesManager::respawnMonsters()
{
	for (deque<Monster>::iterator it = monstersList.begin(); it < monstersList.end(); it++)
	{
		if (it->death && (it->deathTime + it->getRespawnTime() < clock()))
		{
			Monster *monster = &(*it);

			std::deque<Player*> playersInArea;
			this->getPlayersInArea(playersInArea, monster->getPosX(), monster->getPosY(), monster->getPosZ());
			if(playersInArea.empty())
			{
				it->death = false;

				Tile *tilet = Map.getTile(monster->spawnPosX, monster->spawnPosY, monster->spawnPosZ);
				tilet->addThing(monster);


				for (std::deque<Player*>::iterator it = playersInArea.begin(); it < playersInArea.end(); it++)
				{
					Game.sendPlayerCreature((*it),monster, true);
				}
				Game.createEffect(monster->getPosX(), monster->getPosY(), monster->getPosZ(), 4);

				#ifdef _DEBUG
					std::cout << "Monster id: " << monster->getId() << ", name: " << monster->getName() << " respawned!" << std::endl;
				#endif
			}
		}
	}
}

void CreaturesManager::saveAllPlayers()
{
	for (deque<Player>::iterator it = playersList.begin(); it < playersList.end(); it++)
	{
		savePlayer(it->getName());
	}    
}

bool CreaturesManager::savePlayer(string playerName)
{
   for (deque<Player>::iterator it = playersList.begin(); it < playersList.end(); it++)
   {
      if (it->getName() == playerName)
	  {
			std::string extension = ".pla";
			std::string filename = "Players\\";
			filename += it->getName();
			filename += extension;
			std::ofstream plik_wy(filename.c_str(), ios_base::trunc);
			{
				plik_wy<<"name="<<it->getName()<<"\n";
				plik_wy<<"accountName="<<it->getAccountName()<<"\n";
				plik_wy<<"access="<<int(it->access)<<"\n";
				plik_wy<<"sex="<<it->sex<<"\n";
				plik_wy<<"looktype="<<int(it->getLooktype())<<"\n";
				plik_wy<<"vocation="<<int(it->getVocation())<<"\n";
				plik_wy<<"posX="<<it->getPosX()<<"\n";
				plik_wy<<"posY="<<it->getPosY()<<"\n";
				plik_wy<<"posZ="<<int(it->getPosZ())<<"\n";
				plik_wy<<"health="<<it->getHealth()<<"\n";
				plik_wy<<"maxHealth="<<it->getMaxHealth()<<"\n";
				plik_wy<<"mana="<<it->getMana()<<"\n";
				plik_wy<<"maxMana="<<it->getMaxMana()<<"\n";
				plik_wy<<"level="<<it->getLevel()<<"\n";
				plik_wy<<"experience="<<it->getExperience()<<"\n";
				plik_wy<<"skillPoints="<<it->getSkillPoints()<<"\n";
				plik_wy<<"statisticsPoints="<<it->getStatisticsPoints()<<"\n";
				plik_wy<<"speed="<<it->getSpeed()<<"\n";
				plik_wy<<"strength="<<it->getStrength()<<"\n";
				plik_wy<<"dexterity="<<it->getDexterity()<<"\n";
				plik_wy<<"intelligence="<<it->getIntelligence()<<"\n";
				plik_wy<<"magicPower="<<it->getMagicPower()<<"\n";
				plik_wy<<"hungryTicks="<<it->getHungryTicks()<<"\n";
				if(it->equipment.lRingEar)
					plik_wy<<"lRingEar="<<it->equipment.lRingEar->serialize()<<"\n";
				if(it->equipment.rRingEar)
					plik_wy<<"rRingEar="<<it->equipment.rRingEar->serialize()<<"\n";
				if(it->equipment.head)
					plik_wy<<"head="<<it->equipment.head->serialize()<<"\n";
				if(it->equipment.necklace)
					plik_wy<<"necklace="<<it->equipment.necklace->serialize()<<"\n";
				if(it->equipment.backpack)
					plik_wy<<"backpack="<<it->equipment.backpack->serialize()<<"\n";
				if(it->equipment.armor)
					plik_wy<<"armor="<<it->equipment.armor->serialize()<<"\n";
				if(it->equipment.shield)
					plik_wy<<"shield="<<it->equipment.shield->serialize()<<"\n";
				if(it->equipment.weapon)
					plik_wy<<"weapon="<<it->equipment.weapon->serialize()<<"\n";
				if(it->equipment.belt)
					plik_wy<<"belt="<<it->equipment.belt->serialize()<<"\n";
				if(it->equipment.legs)
					plik_wy<<"legs="<<it->equipment.legs->serialize()<<"\n";
				if(it->equipment.boots)
					plik_wy<<"boots="<<it->equipment.boots->serialize()<<"\n";
				if(it->equipment.lRing)
					plik_wy<<"lRing="<<it->equipment.lRing->serialize()<<"\n";
				if(it->equipment.rRing)
					plik_wy<<"rRing="<<it->equipment.rRing->serialize()<<"\n";
				if(it->equipment.gloves)
					plik_wy<<"gloves="<<it->equipment.gloves->serialize()<<"\n";
				if(it->equipment.arrows)
					plik_wy<<"arrows="<<it->equipment.arrows->serialize()<<"\n";
				if(it->itemStorage->size() != 0)
				{
					plik_wy<<"itemStorage=";
					for(unsigned int iter = 0; iter < it->itemStorage->size(); iter++)
					{
						plik_wy << it->itemStorage->getItem(char(iter))->serialize() << "|";
					}
				}
				if(it->hasFlags() == true)
				{
					plik_wy<<"flags=" << it->serializeFlags();
				}
				plik_wy<<"spells=" << it->serializeSpells();
				plik_wy.close();
			}		
			return true;
	  }
   }    
   return false;
}

bool CreaturesManager::savePlayer(unsigned int _id)
{
   for (deque<Player>::iterator it = playersList.begin(); it < playersList.end(); it++)
   {
      if (it->getId() == _id)
	  {
			return savePlayer(it->getName());
	  }
   }    
   return false;
}


Player* CreaturesManager::getPlayer(unsigned int _id)
{
   for (deque<Player>::iterator it = playersList.begin(); it < playersList.end(); it++)
   {
      if (it->getId() == _id)
	  {
		return &(*it);
		break;
	  }
   }
   return NULL;
}

Player* CreaturesManager::getPlayer(string _name)
{
   for (deque<Player>::iterator it = playersList.begin(); it < playersList.end(); it++)
   {
      if (it->getName() == _name)
	  {
		return &(*it);
		break;
	  }
   }
   return NULL;
}

Monster* CreaturesManager::getMonster(unsigned int _id)
{
   for (deque<Monster>::iterator it = monstersList.begin(); it < monstersList.end(); it++)
   {
      if (it->getId() == _id)
	  {
		return &(*it);
		break;
	  }
   }
   return NULL;
}

Creature* CreaturesManager::getCreature(unsigned int _id)
{
   for (deque<Monster>::iterator it = monstersList.begin(); it < monstersList.end(); it++)
   {
      if (it->getId() == _id)
	  {
		return &(*it);
		break;
	  }
   }
   for (deque<Player>::iterator it = playersList.begin(); it < playersList.end(); it++)
   {
      if (it->getId() == _id)
	  {
		return &(*it);
		break;
	  }
   }
   for (deque<NPC>::iterator it = NPCList.begin(); it < NPCList.end(); it++)
   {
      if (it->getId() == _id)
	  {
		return &(*it);
		break;
	  }
   }
   return NULL;
}

NPC* CreaturesManager::getNPC(string _name)
{
   for (deque<NPC>::iterator it = NPCList.begin(); it < NPCList.end(); it++)
   {
      if (it->getName() == _name)
	  {
		return &(*it);
		break;
	  }
   }
   return NULL;
}

void CreaturesManager::getCreaturesInArea(std::deque<Creature*> &spectators, unsigned int x, unsigned int y, unsigned char z)
{
	Position pos;
	pos.x = x;
	pos.y = y;
	pos.z = z;
	Map.getSpectators(pos, spectators);
}

void CreaturesManager::getPlayersInArea(std::deque<Player*> &spectators, unsigned int x, unsigned int y, unsigned char z)
{
	std::deque<Creature*> creatures;
	getCreaturesInArea(creatures, x, y, z);

	for(std::deque<Creature*>::iterator it = creatures.begin(); it != creatures.end(); it++)
	{
		Player *player = dynamic_cast<Player *>(*it);
		if(player)
		{
			spectators.push_back(player);
		}
	}
}

void CreaturesManager::getFloorAreaPlayers(unsigned int posx, unsigned int posy, unsigned char posz, std::deque<Player*> &players)
{
	Map.getFloorAreaPlayers(posx, posy, posz, players);
}

unsigned int CreaturesManager::createPlayer(string name)
{
    std::string extension = ".pla";
    std::string filename = "Players\\";
    filename += name;
    filename += extension;

	std::ifstream file(filename.c_str());
    if(file.is_open())
    {
       return 0;
	}

	playersList.push_back(Player());
	playersList[playersList.size()-1].setName(name);
	playersList[playersList.size()-1].calculateNextLevelExperience();
	return playersList[playersList.size()-1].getId();
}

unsigned int CreaturesManager::createMonster(string name, unsigned int posx, unsigned int posy, unsigned int posz, unsigned int respawn_time)
{
	if(loadMonster(name,false))
	{
		monstersList[monstersList.size()-1].setPos(posx, posy, posz);
		monstersList[monstersList.size()-1].spawnPosX = posx;
		monstersList[monstersList.size()-1].spawnPosY = posy;
		monstersList[monstersList.size()-1].spawnPosZ = posz;
		monstersList[monstersList.size()-1].setRespawnTime(respawn_time);
		return monstersList[monstersList.size()-1].getId();
	}
	return 0;
}

bool CreaturesManager::removePlayer(string playerName)
{
   for (deque<Player>::iterator it = playersList.begin(); it < playersList.end(); it++)
   {
      if (it->getName() == playerName)
	  {
		 playersList.erase(it);
		 return true;
	  }
   }    
   return false;
}

bool CreaturesManager::removePlayer(unsigned int _id)
{
	return removePlayer(getPlayer(_id)->getName());
}

bool CreaturesManager::removeCreature(unsigned int _id)
{
   for (deque<Player>::iterator it = playersList.begin(); it < playersList.end(); it++)
   {
      if (it->getId() == _id)
	  {
		 playersList.erase(it);
		 return true;
	  }
   }  
   for (deque<Monster>::iterator it = monstersList.begin(); it < monstersList.end(); it++)
   {
      if (it->getId() == _id)
	  {
		 monstersList.erase(it);
		 return true;
	  }
   } 
   return false;
}