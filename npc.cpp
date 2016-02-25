#include <iostream>
#include <fstream>
#include "npc.h"
#include "game.h"
#include "item.h"
#include "player.h"

extern Game Game;

NPC::NPC(string name)
{
	this->setName(name);
}

NPC::~NPC()
{
	//delete all contents here, problematic, for future ;)
}
bool NPC::parseScript(string script)
{
	string tmp;
	deque<string> lines;
	string filename = "NPC\\";
	filename += script;
	filename += ".txt";

	ifstream file(filename.c_str());
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

	unsigned int dialogid = 1;
	char currCondition = 0;
	NPCdialog *dialog = NULL;
	bool inShop = false;
	for(unsigned int i = 0; i < lines.size(); i++)
	{
		if(lines[i].size() == 0)
			continue;
		if(lines[i].find_first_not_of(" ") != string::npos)
			lines[i].erase(0, lines[i].find_first_not_of(" "));
		if(lines[i].find_first_of("#") != string::npos)
			lines[i].erase(lines[i].find_first_of("#"), 1337);
		if(lines[i].find("shop_begin") != string::npos)
		{
			inShop = true;
			continue;
		}
		if(lines[i].find("shop_end") != string::npos)
		{
			inShop = false;
			continue;
		}
		if(inShop)
		{
			enum ShopTransactionType type;
			unsigned int id;
			unsigned int price;
			string temp = lines[i];
			temp.erase(temp.find_first_of(' '), 1337);
			if(temp == "buy")
				type = SHOP_ACTION_BUY;
			else if(temp == "sell")
				type = SHOP_ACTION_SELL;
			else
				type = SHOP_ACTION_NONE;
			
			temp = lines[i];
			temp.erase(0, temp.find_first_of(' ')+1);
			temp.erase(temp.find_first_of(' '), 1337);
			id = atoi(temp.c_str());

			temp = lines[i];
			temp.erase(0, temp.find_last_of(' ')+1);
			price = atoi(temp.c_str());

			shop.push_back(NPCshopEntry());
			shop.at(shop.size()-1).type = type;
			shop.at(shop.size()-1).itemId = id;
			shop.at(shop.size()-1).price = price;
		}
		if(lines[i].find("_begin") != string::npos)
		{
			lines[i].erase(lines[i].find_first_of("_"), lines[i].back());
			if(lines[i] == "main")
			{
				dialogid = 1;
			}
			else
			{
				lines[i].erase(0,1);
				dialogid = atoi(lines[i].c_str());
			}
			dialog = new NPCdialog();
			dialog->trigger = -100000000;
			this->dialogs.insert(std::pair<unsigned int, NPCdialog*>(dialogid, dialog));
		}
		else if(lines[i].find("_end") == string::npos)
		{
			if(dialog)
			{
				string command = lines[i];
				string value = lines[i];

				if(command.find_first_of(" ") != string::npos)
					command.erase(command.find_first_of(" "), 1337); //usuwa 1337 znaków od pierwszej spacji (czyli w domyœle usuwa do koñca)
				if(command == "if")
				{
					currCondition = 1;
					value.erase(0, value.find_first_of(' ')+1);
					dialog->trigger = atoi(value.c_str());
					continue;
				}
				if(command == "then")
					continue;
				if(command == "else")
				{
					currCondition = 2;
					continue;
				}
				if(command == "fi")
				{
					currCondition = 0;
					continue;
				}
				if(command.find("act") == string::npos)
				{
					value.erase(0, value.find_first_of('"')+1);
					value.erase(value.find_last_of('"'), value.back());
				}
				else
					command = "act";

				if(command == "say")
					dialog->addNPCsay(currCondition, value);
				else if(command[0] == 'd')
				{
					unsigned int tempid = 0;
					command.erase(0,1);
					if(command == "end")
						tempid = 0;
					else
						tempid = atoi(command.c_str());

					dialog->addNPCchoice(currCondition, tempid, value);
				}
				else if(command == "act")
				{
					unsigned int tempid = 0;
					unsigned int first;
					int second;
					string value1 = value.substr(value.find_first_of('(')+1, value.find_last_of(')')-value.find_first_of('('));
					string value2 = value1.substr(value1.find_first_of(',')+1, 1337);
					value1.erase(value1.find_first_of(','), 1337);
					first = atoi(value1.c_str());
					second = atoi(value2.c_str());

					lines[i].erase(0, 3);
					lines[i].erase(lines[i].find_first_of('('), 1337);
					command = lines[i];

					if(command == "AddPlayerItem")
						dialog->addNPCaction(currCondition, NPCActionType::NPC_COND_ADDITEM, first, second);
					if(command == "RemovePlayerItem")
						dialog->addNPCaction(currCondition, NPCActionType::NPC_COND_REMOVEITEM, first, second);
					if(command == "SetPlayerValue")
						dialog->addNPCaction(currCondition, NPCActionType::NPC_COND_SETUNIQUEVALUE, first, second);
					if(command == "GetPlayerValue")
						dialog->addNPCaction(currCondition, NPCActionType::NPC_COND_GETUNIQUEVALUE, first, second);
					if(command == "AddPlayerExperience")
						dialog->addNPCaction(currCondition, NPCActionType::NPC_COND_ADDEXPERIENCE, first, second);
				}
			}
		}
	}
	return true;
}

sf::Packet NPC::playerChoose(unsigned int playerid, unsigned int option)
{
	sf::Packet toSend;
	unsigned int nextDialog; 

	Player *player = Game.getPlayerByID(playerid);
	if(!player)
		cout << "Fatal error NPC::playerChoose - player not found!";

	if(this->playerState.find(playerid) == this->playerState.end())
	{
		nextDialog = 1;
		this->playerState.insert(std::pair<unsigned int, unsigned int>(playerid, nextDialog));
		player->lastNPCcondition = 0;
	}
	else if(this->dialogs.at(this->playerState.at(playerid))->choices.size() > option)
	{
		unsigned int counter = 0;

		for(unsigned int i = 0; i < this->dialogs.at(this->playerState.at(playerid))->choices.size(); i++)
		{
			char temp = this->dialogs.at(this->playerState.at(playerid))->choices.at(i)->condition;
			if(temp == player->lastNPCcondition || temp == 0)
			{
				if(counter == option)
				{
					nextDialog = this->dialogs.at(this->playerState.at(playerid))->choices.at(i)->nextDialog;
					break;
				}
				counter++;
			}
		}
		//nextDialog = this->dialogs.at(this->playerState.at(playerid))->choices.at(option)->nextDialog;
	}
	else
		nextDialog = 0;

	playerState[playerid] = nextDialog;

	if(nextDialog == 0)
	{
		playerDisappear(playerid);
		int sendid = 600;
		toSend << sendid;
		toSend << this->getName();
		toSend << static_cast<unsigned char>(255);
	}
	else if(this->dialogs.at(this->playerState.at(playerid))->choices.size() > option)
	{
		int stack = -10000;
		int condition = 0;
		for(unsigned int i = 0; i < this->dialogs.at(nextDialog)->actions.size(); i++)
		{
			if(this->dialogs.at(nextDialog)->actions.at(i)->type == NPCActionType::NPC_COND_ADDITEM && (this->dialogs.at(nextDialog)->actions.at(i)->condition == condition || this->dialogs.at(nextDialog)->actions.at(i)->condition == 0))
			{
				Player *player = Game.getPlayerByID(playerid);
				if(player)
					stack = Game.addItemToPlayerStorage(player, 
														new Item(this->dialogs.at(nextDialog)->actions.at(i)->first, 
														this->dialogs.at(nextDialog)->actions.at(i)->second), 
														20);
			}
			if(this->dialogs.at(nextDialog)->actions.at(i)->type == NPCActionType::NPC_COND_REMOVEITEM && (this->dialogs.at(nextDialog)->actions.at(i)->condition == condition || this->dialogs.at(nextDialog)->actions.at(i)->condition == 0))
			{
				Player *player = Game.getPlayerByID(playerid);
				if(player)
					stack = Game.removeItemIdFromPlayerStorage(player, 
							                                   this->dialogs.at(nextDialog)->actions.at(i)->first, 
							                                   this->dialogs.at(nextDialog)->actions.at(i)->second);
			}
			if(this->dialogs.at(nextDialog)->actions.at(i)->type == NPCActionType::NPC_COND_SETUNIQUEVALUE && (this->dialogs.at(nextDialog)->actions.at(i)->condition == condition || this->dialogs.at(nextDialog)->actions.at(i)->condition == 0))
			{
				Player *player = Game.getPlayerByID(playerid);
				if(player)
					stack = player->setFlag(this->dialogs.at(nextDialog)->actions.at(i)->first, 
							                this->dialogs.at(nextDialog)->actions.at(i)->second);
			}
			if(this->dialogs.at(nextDialog)->actions.at(i)->type == NPCActionType::NPC_COND_GETUNIQUEVALUE && (this->dialogs.at(nextDialog)->actions.at(i)->condition == condition || this->dialogs.at(nextDialog)->actions.at(i)->condition == 0))
			{
				Player *player = Game.getPlayerByID(playerid);
				if(player)
					stack = player->getFlag(this->dialogs.at(nextDialog)->actions.at(i)->first);
			}
			if(this->dialogs.at(nextDialog)->actions.at(i)->type == NPCActionType::NPC_COND_ADDEXPERIENCE && (this->dialogs.at(nextDialog)->actions.at(i)->condition == condition || this->dialogs.at(nextDialog)->actions.at(i)->condition == 0))
			{
				Player *player = Game.getPlayerByID(playerid);
				if(player)
					stack = Game.addPlayerExperience(player, this->dialogs.at(nextDialog)->actions.at(i)->first);
			}

			if(i == 0)
			{
				if(stack == this->dialogs.at(this->playerState.at(playerid))->trigger)
				{
					condition = 1;
				}
				else
				{
					condition = 2;
				}
				player->lastNPCcondition = condition;
			}
		}

		string said;
		int sendid = 600;
		toSend << sendid;
		toSend << this->getName();
		toSend << this->getLooktype();

		std::deque<string> temp;
		unsigned int tempCounter = 0;
		for(unsigned int i = 0; i < this->dialogs.at(nextDialog)->says.size(); i++)
		{
			if(this->dialogs.at(nextDialog)->says.at(i)->condition == condition || this->dialogs.at(nextDialog)->says.at(i)->condition == 0)
			{
				said = *(&this->dialogs.at(nextDialog)->says.at(i)->text);
				temp.push_back(said);
				tempCounter++;
			}
		}
		toSend << tempCounter;
		tempCounter = 0;
		for(unsigned int i = 0; i < temp.size(); i++)
			toSend << temp[i];
		temp.clear();

		for(unsigned int i = 0; i < this->dialogs.at(nextDialog)->choices.size(); i++)
		{
			if(this->dialogs.at(nextDialog)->choices.at(i)->condition == condition || this->dialogs.at(nextDialog)->choices.at(i)->condition == 0)
			{
				said = *(&this->dialogs.at(nextDialog)->choices.at(i)->text);
				temp.push_back(said);
				tempCounter++;
			}
		}
		toSend << tempCounter;
		tempCounter = 0;
		for(unsigned int i = 0; i < temp.size(); i++)
			toSend << temp[i];
		temp.clear();
	}
	return toSend;
}

void NPC::playerDisappear(unsigned int playerid)
{
	if(this->playerState.find(playerid) != this->playerState.end())
		this->playerState.erase(this->playerState.find(playerid));
}

int NPC::canBuyItem(unsigned int itemId) //return price, if item is not in offer, returns -1
{
	for(unsigned int i = 0; i < this->shop.size(); i++)
		if(shop[i].type == SHOP_ACTION_BUY && shop[i].itemId == itemId)
			return static_cast<int>(shop[i].price);
	return -1;
}

int NPC::canSellItem(unsigned int itemId) //return price, if item is not in offer, returns -1
{
	for(unsigned int i = 0; i < this->shop.size(); i++)
		if(shop[i].type == SHOP_ACTION_SELL && shop[i].itemId == itemId)
			return static_cast<int>(shop[i].price);
	return -1;	
}

void NPC::getShopEntries(deque<NPCshopEntry> &entries)
{
	entries = shop;
}

void NPCdialog::addNPCsay(char condition, string text)
{
	NPCsay *say = new NPCsay;
	say->condition = condition;
	say->text = text;
	this->says.push_back(say);
}

void NPCdialog::addNPCchoice(char condition, unsigned int nextDialog, string text)
{
	NPCchoice *choice = new NPCchoice;
	choice->condition = condition;
	choice->nextDialog = nextDialog;
	choice->text = text;
	this->choices.push_back(choice);
}

void NPCdialog::addNPCaction(char condition, NPCActionType type, unsigned int first, int second)
{
	NPCaction *action = new NPCaction;
	action->condition = condition;
	action->type = type;
	action->first = first;
	action->second = second;
	this->actions.push_back(action);
}