#ifndef NPC_H
#define NPC_H

#include <deque>
#include <map>
#include <SFML/Network.hpp>
#include "creature.h"

using namespace std;

struct NPCsay
{
	char condition;
	string text;
};

struct NPCchoice
{
	char condition;
	unsigned int nextDialog;
	string text;
};

enum NPCActionType
{
	NPC_COND_ADDITEM		= 1,
	NPC_COND_REMOVEITEM     = 2,
	NPC_COND_SETUNIQUEVALUE = 3,
	NPC_COND_GETUNIQUEVALUE	= 4,
	NPC_COND_ADDEXPERIENCE  = 5,
	NPC_COND_NONE           = 0
};

enum ShopTransactionType
{
	SHOP_ACTION_BUY  = 1,
	SHOP_ACTION_SELL = 2,
	SHOP_ACTION_NONE = 0
};

struct NPCshopEntry
{
	ShopTransactionType type;
	unsigned int itemId;
	unsigned int price;
};

struct NPCaction
{
	char condition;
	NPCActionType type;
	unsigned int first;
	int second;
};

class NPCdialog
{
public:
	NPCdialog() {};
	int trigger;
	deque<NPCsay*> says;
	deque<NPCchoice*> choices;
	deque<NPCaction*> actions;
	void addNPCsay(char condition, string text);
	void addNPCchoice(char condition, unsigned int nextDialog, string text);
	void addNPCaction(char condition, NPCActionType type, unsigned int first, int second);
};

class NPC : public Creature
{
protected:
	map<unsigned int, NPCdialog*> dialogs;
	map<unsigned int, unsigned int> playerState; 
	deque<NPCshopEntry> shop;
public:  
	NPC(string name);
	~NPC();
	bool parseScript(string name);
	sf::Packet playerChoose(unsigned int playerid, unsigned int option);
	void playerDisappear(unsigned int playerid);
	int canSellItem(unsigned int itemId);
	int canBuyItem(unsigned int itemId);
	void getShopEntries(deque<NPCshopEntry> &entries);
};

#endif
