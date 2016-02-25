#ifndef __CREATURESMANAGER_H__
#define __CREATURESMANAGER_H__

#include <string>
#include <deque>
#include <SFML/Network.hpp>

#include "monster.h"
#include "player.h"
#include "npc.h"

using namespace std;

class CreaturesManager
{
public:
	CreaturesManager();
	~CreaturesManager()
    {
    }

	bool loadPlayer(string playerName);
	bool loadMonster(string monsterName, bool reload, unsigned int id = 0);
	bool loadNPC(string name, string script);
	void deathCreature(unsigned int id);
	void respawnMonsters();

	void saveAllPlayers();
	bool savePlayer(string playerName);
	bool savePlayer(unsigned int _id);

	Player* getPlayer(unsigned int _id);
	Player* getPlayer(string _name);
	Monster* getMonster(unsigned int _id);
	Creature* getCreature(unsigned int _id);
	NPC* getNPC(string _name);
	void getPlayersInArea(std::deque<Player*> &spectators, unsigned int x, unsigned int y, unsigned char z);
	void getFloorAreaPlayers(unsigned int posx, unsigned int posy, unsigned char posz, std::deque<Player*> &players);
	void getCreaturesInArea(std::deque<Creature*> &spectators, unsigned int x, unsigned int y, unsigned char z);

	unsigned int getPlayersCount() { return playersList.size(); };
	unsigned int getMonstersCount() { return monstersList.size(); };
	unsigned int getNPCCount() { return NPCList.size(); };

	unsigned int createPlayer(string name);
	unsigned int createMonster(string name, unsigned int posx, unsigned int posy, unsigned int posz, unsigned int respawn_time);

	bool removePlayer(string playerName);
	bool removePlayer(unsigned int _id);
	bool removeCreature(unsigned int _id);

	deque<Player> playersList;
	deque<Monster> monstersList;
	deque<NPC> NPCList;
private:
};




#endif