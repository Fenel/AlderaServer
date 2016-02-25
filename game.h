#ifndef GAME__H
#define GAME__H

#include <deque>
#include "enums.h"

class Thing;
class Creature;
class Player;
class NPC;
class Tile;
class CreaturesManager;
class Tile;
class Item;
class Container;
class Map;

using namespace std;

class Game
{
public:
	Game();
	~Game();

	void loop();

    bool loadSpawn();
	bool loadNPCS();
	bool loadMap(); 
	bool loadOnlinePlayersRecord();
	bool saveOnlinePlayersRecord(unsigned int record);
	void serverSave(bool logout = false);

	unsigned int maxPlayersCount;
	bool logicThreadRunning;

	unsigned int getOnlinePlayersCount();
	Player* getPlayerByName(string _name);
	Player* getPlayerByID(unsigned int _id);
	Creature* getCreatureByID(unsigned int _id);	
	ClientContext* getClientByPlayerName(string _name);
	ClientContext* getClientByPlayerID(unsigned int _id);
	ClientContext* getClientByPlayer(Player* player);

	bool isValidName(std::string name, std::string &result);
	void processPlayerLogin(Player *player);
	void sendPlayerStatistics(Player *player);
	void sendPlayerStartingTiles(Player *player);
	void sendPlayerStartingCreatures(Player *player);
	void sendPlayerStartingItemStorage(Player *player);
	void sendPlayerCreature(Player *player, Creature *creature, bool move = true);
	void sendPlayerLoginToPlayers(Player *player);
	void sendCreatureUpdateHealth(Creature *creature);
	void sendCreatureUpdateLight(Creature *creature);
	void sendCreatureUpdateSpeed(Creature *creature);
	void onPlayerLogin(Player *player);
	void onCreatureDisappear(Creature *creature);
	void onCreatureDeath(Creature *creature);
	void sendPlayerTargetConfirmation(Player *player, unsigned int cid, bool cancel);
	void attackCreature(Creature *receiver, Creature *attacker, CombatType combatType, unsigned char spellID = 0);
	void createEffect(unsigned int x, unsigned int y, unsigned char z, unsigned char type);
	void createTextEffect(unsigned int x, unsigned int y, unsigned char z, unsigned char color, string text);
	void createShootEffect(unsigned int fromx, unsigned int fromy, unsigned char fromz, unsigned int tox, unsigned int toy, unsigned char toz, unsigned char type);
	bool addItemToContainer(Container *container, Item *item);
	bool removeItemFromContainer(Container *container, Item *item);
	bool removeItemFromPlayerEquipment(Player *player, int slot, unsigned short count = 0);
	bool equipItem(Player *player, int slot, Item *item);
	bool addItemToPlayerStorage(Player *player, Item *item, unsigned int bpid);
	bool removeItemFromPlayerStorage(Player *player, unsigned char index, unsigned int bpid);
	bool removeItemFromPlayerStorage(Player *player, Item *item);
	bool removeItemIdFromPlayerStorage(Player *player, unsigned int id, unsigned short count = 1);
	void createItemOnTile(unsigned int id, unsigned short count, Tile *tile);
    void addThingToTile(Thing *thing, Tile *tile);
    void transformThingFromTile(unsigned char stackpos, Thing *thing, Tile *tile);
    bool removeThingFromTile(unsigned char stackpos, Tile *tile);
	bool removeTopThingFromTile(Tile *tile, unsigned short count = 0);
    Thing *getThingFromTile(unsigned char stackpos, Tile *tile);
    unsigned char getIndexOfThingFromTile(Thing* thing, Tile *tile); 
	unsigned char getTileType(unsigned int x, unsigned int y, unsigned char z);
	bool moveThing(unsigned char stackpos, Tile *fromTile, Tile *toTile);
	bool moveCreature(Creature* creature, unsigned char direction, unsigned char *special = NULL);
	bool moveCreature(Creature* creature, char movex, char movey, unsigned char *special = NULL);
	void internalMovePlayer(Player *player, unsigned char direction);
	void doPlayerChangeDirection(Player *player, unsigned char direction);
	void doTeleportPlayer(Player *player, unsigned int destPosX, unsigned int destPosY, unsigned char destPosZ);
	void doCreatureTemporaryChangeSpeed(Creature *creature, float factor, unsigned int duration);
	void receiveThrowItem(Player *player, sf::Packet toReceive);
	void receiveEquipItem(Player *player, sf::Packet toReceive);
	void receiveRequestItemStats(Player *player, sf::Packet toReceive);
	void receiveUseItem(Player *player, sf::Packet toReceive);
	void receivePlayerAttackCreature(Player *player, sf::Packet toReceive);
	int  getPlayerFlag(Player *player, unsigned int _key);
	bool setPlayerFlag(Player *player, unsigned int _key, int _value);
	bool canEquipItem(Player *player, Item *item, unsigned short slot);
	bool canThrow(Player *player, Thing *thing, unsigned int fromx, unsigned int fromy, unsigned char fromz, unsigned int tox, unsigned int toy, unsigned char toz, bool playerEquip = false);
	bool canSeeThroughTiles(unsigned int fromx, unsigned int fromy, unsigned int tox, unsigned int toy, unsigned char z);
	bool canDoCombat(Creature *creature);
	bool canDoCombat(unsigned int x, unsigned int y, unsigned char z);
	bool isNearPos(Creature *creature, unsigned int x, unsigned int y, unsigned char z);
	bool isInRange(Creature *creature, Creature* creature2, unsigned char range);
	bool isInRange(Creature *creature, unsigned int posx, unsigned int posy, unsigned char posz, unsigned char range);
	bool isInRange(unsigned int fromx, unsigned int fromy, unsigned char fromz, unsigned int tox, unsigned int toy, unsigned char toz, unsigned char range);
	void sendPacketToPlayersInArea(sf::Packet packet, unsigned int x, unsigned int y, unsigned char z);
    bool sendMessageToPlayer(unsigned int id, unsigned char type, string sender, unsigned int senderLevel, string message);
	bool sendMessageToPlayer(Player *player, unsigned char type, string sender, unsigned int senderLevel, string message, string from = "");
	bool sendMessageToAllPlayers(unsigned char type, string sender, unsigned int senderLevel, const string message);
	void sendPacketToAllPlayers(sf::Packet toSend);
	void sendTileToPlayer(Player *player, Tile *tile);
	void receivePlayerMessage(Player *player, sf::Packet toReceive);
	void sendPlayerUpdateMana(Player *player);
	void receivePlayerUseSpell(Player *player, sf::Packet toReceive);
	void sendPlayerUseSpellCancel(Player *player, unsigned char spellID);
	bool useSpell(Creature *creature, unsigned char spellID);
	bool canPlayerPayMoney(Player *player, unsigned int money);
	void sendPlayerNPCshop(Player *player, NPC *npc);
	bool addPlayerExperience(Player *player, unsigned int amount);

	clock_t getLastSave(){	return lastSave; };
	void setLastSave(){	  lastSave = clock();};

	void processClient(ClientContext *clientContext);
	void doLogic();

	sf::SocketSelector selector;
private:
    clock_t server_started_time;// = clock(); 
	unsigned int currentClientsID;
	bool serverRunning;
	string serverVersion;
  
	unsigned int onlinePlayersRecord;
	string onlinePlayersRecordDate;
	clock_t lastSave;

	sf::TcpListener listenerSocket;
	sf::TcpSocket socket;
	sf::Thread *logicThread;

	static deque<ClientContext> Game::clientsList;

	unsigned char saveNotifed;
	clock_t playersConnectionChecked;
	clock_t lastDoLogic;
	clock_t playersConditionChecked;
};





#endif GAME__H