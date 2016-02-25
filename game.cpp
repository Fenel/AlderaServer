#include <SFML/Network.hpp>
#include <SFML/System.hpp>

#include <iostream>
#include <fstream>
#include <windows.h>
#include <time.h>
#include <sstream>
#include <string>
#include <deque>
#include <list>

#include "configManager.h"
#include "monster.h"
#include "player.h"
#include "baseItems.h"
#include "creaturesManager.h"
#include "accountsManager.h"
#include "BansManager.h"
#include "map.h"
#include "game.h"
#include "enums.h"
#include "container.h"
#include "pathFind.h"
#include "npc.h"

extern ConfigManager ConfigManager;
extern BaseItems BaseItems;
extern CreaturesManager CreaturesManager;
extern AccountsManager AccountsManager;
extern BansManager BansManager;
extern Map Map;
extern PathFind PathFind;

extern string intToStr(int);
extern bool isLettersOnly(std::string str);
extern void coutCurrentTime();

deque<ClientContext> Game::clientsList;

using namespace std;

Game::Game()
{
	clock_t server_started_time = clock(); 
	serverRunning = true;
	serverVersion = "0.3.4";
	currentClientsID = 1;
	lastSave = 0;
	saveNotifed = 0;
	playersConnectionChecked = 0;
	lastDoLogic = 0;
	playersConditionChecked = 0;

	srand((unsigned)time(NULL)); //resetowanie generatora liczb losowych

    std::cout<<"|*****************************|\n";
    std::cout<<"|        Aldera Server        |\n";
    std::cout<<"| |*************************| |\n"; 
    std::cout<<"| |      Version v" << this->serverVersion << "     | |\n";
    std::cout<<"| |   Compiled 26.02.2015   | |\n";
    std::cout<<"| |*************************| |\n";
    std::cout<<"|*****************************|\n\n";
    coutCurrentTime(); 
    std::cout << "::Starting server...\n";
    coutCurrentTime(); 
	std::cout << "::Server Configuration:" << std::endl;
	std::cout << "           - Port: " << ConfigManager.port << std::endl;
	if(ConfigManager.pvp)
		std::cout << "           - PvP: enabled" << std::endl;
	else
		std::cout << "           - PvP: disabled" << std::endl;
	std::cout << "           - PZ Lock: " << ConfigManager.pzLock << std::endl;
	std::cout << "           - Hunting Lock: " << ConfigManager.huntingLock << std::endl;
	std::cout << "           - Protection Level: " << ConfigManager.protectionLevel << std:: endl;
	if(ConfigManager.noDamageToPartyMembers)
		std::cout << "           - No Damage To Party Members: enabled" << std::endl;
	else
		std::cout << "           - No Damage To Party Members: disabled" << std::endl;
	if(ConfigManager.save)
		std::cout << "           - Save: enabled" << std::endl;
	else
		std::cout << "           - Save: disabled" << std::endl;
	std::cout << "           - Save Interval: " << ConfigManager.saveInterval << std:: endl;
	std::cout << "           - Max Players: " << ConfigManager.maxPlayers << std:: endl;
	std::cout << "           - Player Action Interval: " << ConfigManager.playerActionInterval << std:: endl;
	BaseItems.load();
	loadMap();
    loadSpawn();
	loadNPCS();
	std::cout << "           - Number of monsters: " << CreaturesManager.monstersList.size() << std:: endl;
	loadOnlinePlayersRecord();

    listenerSocket.listen(ConfigManager.port);

    coutCurrentTime(); 
    std::cout<<"::Server is running.";
	std::cout<<" ( Total: " << clock() - server_started_time << " ms )\n";  

	loop();
}

Game::~Game()
{

}

void Game::loop()
{
	sf::TcpSocket *client;

	selector.add(listenerSocket);
	listenerSocket.setBlocking(false);
	sf::Time timeout = sf::milliseconds(10); //10ms timeout
	while(serverRunning)
	{
		if(selector.wait(timeout)) //10ms timeout
		{
			if (selector.isReady(listenerSocket))
			{
				client = new sf::TcpSocket;
				if(listenerSocket.accept(*client) == sf::Socket::Done)
				{
					selector.add(*client); //dodanie wskaŸnika na socket clienta do selectora

					clientsList.push_back(ClientContext());
					clientsList[clientsList.size()-1].socket = client;
					clientsList[clientsList.size()-1].player = NULL;
					clientsList[clientsList.size()-1].lostPlayer = false;
					clientsList[clientsList.size()-1].id = currentClientsID;
					currentClientsID++;

					coutCurrentTime();
					std::cout << "::New client, ip: " << client->getRemoteAddress() << ", id: " << currentClientsID << std::endl;
				}
				else //nie uzyskaliœmy po³¹czenia, kasujemy socket clienta
				{
					delete client;
				}
			}
			else
			{
				for (std::deque<ClientContext>::iterator it = clientsList.begin(); it != clientsList.end(); ++it)
				{
					sf::TcpSocket& client = *(it->socket);
					if(selector.isReady(client))
					{
						this->processClient(&(*it));
					}
				}
			}
		}
		doLogic(); //odpalenie wykonywania logiki gry, wewnêtrznie sprawdzane by wykonywa³o siê co oko³o 100ms
	}
}

void Game::doLogic()
{
	if(lastDoLogic + 100 < clock())
	{
		if((ConfigManager.save == true) && (this->getLastSave() + ConfigManager.saveInterval*1000 < clock()))
		{
			this->setLastSave();
			this->serverSave();
			saveNotifed = 0;
		}
		if((ConfigManager.save == true) && (this->getLastSave() + ConfigManager.saveInterval*1000 - 60*1000 < clock()) && saveNotifed == 0)
		{
			this->sendMessageToAllPlayers(1 ,"System" ,0 , "Server save in 1 minute.");
			saveNotifed = 1;
		}
		if((ConfigManager.save == true) && (this->getLastSave() + ConfigManager.saveInterval*1000 - 10*1000 < clock()) && saveNotifed == 1)
		{
			this->sendMessageToAllPlayers(1 ,"System" ,0 , "Server save in 10 seconds.");
			saveNotifed = 2;
		}
		CreaturesManager.respawnMonsters();

		sf::Packet toSend;
		int pid = 1500;
		toSend << pid;

		if(playersConditionChecked + 200 < clock())
		{
			for(unsigned int i = 0; i < CreaturesManager.playersList.size(); i++)
			{
				Player *player = &CreaturesManager.playersList.at(i);
				if(player->getHungryTicks() > 0 && player->getLastHungryChecked() + 5000 < clock())
				{
					player->addHealth(5);
					player->addHungryTicks(-1);
					player->setLastHungryChecked();
					this->sendCreatureUpdateHealth(player);

					if(player->getHungryTicks() == 0) //gracz nie jest ju¿ najedzony
						this->sendMessageToPlayer(player, 0, "", 0, "You're starting to be very hungry.");
				}
				if(player->speedConditionExpires != 0 && player->speedConditionExpires < clock())
				{
					player->setSpeed(player->baseSpeed);
					player->speedConditionExpires = 0;
					this->sendCreatureUpdateSpeed(player);
				}
			}
			playersConditionChecked = clock();
		}
		if(playersConnectionChecked + ConfigManager.checkConnectionInterval < clock())
		{
			for(deque<ClientContext>::iterator it = clientsList.begin(); it != clientsList.end(); it++)
			{
				if( (it->socket->send(toSend) != sf::Socket::Done) && it->player == NULL && it->lostPlayer)
				{
					coutCurrentTime();
					if(it->player)
						std::cout << "::Player: " << it->player->getName() << ", id: " << it->player->getId() << " without connection, ClientContext erased" << std::endl;
					else
						std::cout << "::Client id: " << it->id << " without connection, ClientContext erased" << std::endl;

					if(it->player)
					{
						Player *player = CreaturesManager.getPlayer(it->player->getName());
						if(player)
						{
							this->onCreatureDisappear(player);
						}
					}

					selector.remove(*(it->socket));
					it->socket->disconnect();
					it->player = NULL;
					it->lostPlayer = true;
					clientsList.erase(it);

					break;
				}
			}
			playersConnectionChecked = clock();
		}
		for(unsigned int i = 0; i < CreaturesManager.monstersList.size(); i++)
		{
			Monster *creature = &CreaturesManager.monstersList.at(i);

			if(creature->speedConditionExpires != 0 && creature->speedConditionExpires < clock())
			{
				creature->setSpeed(creature->baseSpeed);
				creature->speedConditionExpires = 0;
				this->sendCreatureUpdateSpeed(creature);
			}
			if(creature && creature->isWalking() == false && creature->death == false)
			{
				std::deque<Player*>floorAreaPlayers;
				CreaturesManager.getFloorAreaPlayers(creature->getPosX(), creature->getPosY(), creature->getPosZ(), floorAreaPlayers);

				if(floorAreaPlayers.empty() == false)
				{
					if(creature->getTarget() == NULL)
						creature->setTarget(floorAreaPlayers.at( rand() % floorAreaPlayers.size() ));
					
					Creature *target = creature->getTarget();

					if(target && this->canDoCombat(target) && target->getPosX() > creature->getPosX()-10 && target->getPosX() < creature->getPosX()+10 &&
						target->getPosY() > creature->getPosY()-8 && target->getPosY() < creature->getPosY()+8 && target->getPosZ() == creature->getPosZ() )
					{
						if( (creature->targetLastX != target->getPosX() || creature->targetLastY != target->getPosY()) 
							|| creature->pathPoints.empty() == true)
						{
							PathFind.getPath(creature->getPosX(), creature->getPosY(), target->getPosX(), target->getPosY(), creature->getPosZ(), creature->pathPoints);
							creature->targetLastX = target->getPosX();
							creature->targetLastY = target->getPosY();
						}
						if(creature->pathPoints.empty() == false)
						{
							int movex = creature->pathPoints.front().globalX - creature->getPosX();
							int movey = creature->pathPoints.front().globalY - creature->getPosY();

							if(this->moveCreature(creature, movex, movey) == false)
							{
								PathFind.getPath(creature->getPosX(), creature->getPosY(), target->getPosX(), target->getPosY(), creature->getPosZ(), creature->pathPoints);
								creature->targetLastX = target->getPosX();
								creature->targetLastY = target->getPosY();
							}
							else
								creature->pathPoints.pop_front();
						}
					}
					else
						target = NULL;
				}
				if((creature->pathPoints.empty() == true || creature->getTarget() == NULL) && !this->isNearPos(creature->getTarget(), creature->getPosX(), creature->getPosY(), creature->getPosZ()) )
				{
					bool tMoved = false;
					unsigned char tempMove;

					while(tMoved == false)
					{
						tempMove = (rand()%4 + 1)*2;
						if((tempMove == 4 && creature->getPosX() > creature->getSpawnPosX() - 5) ||
						   (tempMove == 6 && creature->getPosX() < creature->getSpawnPosX() + 5) ||
						   (tempMove == 8 && creature->getPosY() > creature->getSpawnPosY() - 5) ||
						   (tempMove == 2 && creature->getPosY() < creature->getSpawnPosY() + 5))
						{
							char diff_x = 0, diff_y = 0;
							if(tempMove == 4)
							  diff_x = -1;
							else if(tempMove == 6)
							  diff_x = 1;
							else if(tempMove == 8)
							  diff_y = -1;
							else if(tempMove == 2)
							  diff_y = 1;

							if( this->canDoCombat(creature->getPosX() + diff_x, creature->getPosY() + diff_y, creature->getPosZ()) )
							{
								this->moveCreature(&CreaturesManager.monstersList.at(i), tempMove);
								creature->setStartedWalking();
								tMoved = true;
							}
						}
					}
				}
			}
			if(creature)
			{
				Creature *target = creature->getTarget();
				if(creature->death == false && target)
				{
					if(!creature->isMeleeAttacking() && this->isNearPos(creature, target->getPosX(), target->getPosY(), target->getPosZ()))
					{
						this->attackCreature(target, creature, COMBAT_PHYSICALDAMAGE);
						creature->setLastMeleeAttack();
					}
					if(creature->getDistanceMaxDamage() > 0 && creature->canShoot())
					{
						if(isInRange(creature->getPosX(), creature->getPosY(), creature->getPosZ(), target->getPosX(), target->getPosY(), target->getPosZ(), creature->getDistanceRange()) 
						&& canSeeThroughTiles(creature->getPosX(), creature->getPosY(), target->getPosX(), target->getPosY(), target->getPosZ()))
						{
							creature->setLastShooted();
							this->createShootEffect(creature->getPosX(), creature->getPosY(), creature->getPosZ(), target->getPosX(), target->getPosY(), target->getPosZ(), creature->getShootType());
							this->attackCreature(target, creature, COMBAT_PHYSICALDAMAGE);
						}
					}
					for(unsigned int it = 0; it < creature->spells.size(); it++)
					{
						if(!creature->isSpellAttacking(creature->spells[it]->spellID))
						{
							if(creature->spells[it]->chance > rand()%100000)
							{
								this->useSpell(creature, creature->spells[it]->spellID);
								creature->setLastSpellAttack(creature->spells[it]->spellID);
								break;
							}
							creature->setLastSpellAttack(creature->spells[it]->spellID);
						}
					}
				}
			}
		}
		lastDoLogic = clock();
	}
}

bool Game::loadMap()
{
	Map.load();
	return true;
}

bool Game::loadSpawn()
{
	clock_t time_started = clock(); 
	coutCurrentTime();
    
	std::cout<<"::Loading spawn.txt...";
    
	int licznik = 1;
	std::ifstream spawnfile("Map/spawn.txt"); 
	if (spawnfile.is_open())
	{
		std::string bufor;
		while(getline(spawnfile, bufor))
		{
			std::string c;
			std::string name;
			int posx, posy, posz, respawn_time;
			std::string ss = "";
			int n = bufor.length();
			int licznik = 1;
			for(int i = 0; i < n; i++)
			{
				c = bufor[i];
				if(c != "|")
					ss += c;
				if(c == "|")
				{
					if( licznik == 1)      
						name = ss;   
					else if( licznik == 2)      
						posx = atoi(ss.c_str());  
					else if( licznik == 3)      
						posy = atoi(ss.c_str());
					else if( licznik == 4)       
						posz = atoi(ss.c_str());
					else if( licznik == 5)
					{      
						respawn_time = atoi(ss.c_str());
						respawn_time *= 1000;
					}                                       
					licznik++;
					ss = "";
				}
			}
			unsigned int monId = CreaturesManager.createMonster(name, posx, posy, posz, respawn_time);

			Tile *tilet = Map.getTile(posx, posy, char(posz));
			if(tilet)
				tilet->addThing(CreaturesManager.getMonster(monId));
			else
				CreaturesManager.deathCreature(monId);
		}
		std::cout<<" Done!";
	}
	else
	{
		coutCurrentTime();
		std::cout << "\n::Error while opening spawn.txt\n";
		return false;
	}
	spawnfile.close();
	
	std::cout<<" ( " << clock() - time_started << " ms )\n";   
	return true;
}

bool Game::loadNPCS()
{
	clock_t time_started = clock(); 
	coutCurrentTime();

	std::cout<<"::Loading npc.txt...";

	int licznik = 1;
	std::ifstream npcfile("Map/npc.txt"); 
	if (npcfile.is_open())
	{
		std::string bufor;
		while(getline(npcfile, bufor))
		{
			std::string c;
			std::string name, script;
			int posx, posy, posz;
			unsigned char looktype;
			std::string ss = "";
			int n = bufor.length();
			int licznik = 1;
			for(int i = 0; i < n; i++)
			{
				c = bufor[i];
				if(c != "|")
				ss += c;
				if(c == "|")
				{
					if( licznik == 1)      
						name = ss;   
					else if( licznik == 2)      
						posx = atoi(ss.c_str());  
					else if( licznik == 3)      
						posy = atoi(ss.c_str());
					else if( licznik == 4)       
						posz = atoi(ss.c_str());
					else if( licznik == 5)   
						looktype = atoi(ss.c_str());  
					else if( licznik == 6)      
						script = ss;   
					licznik++;
					ss = "";
				}
			}
			CreaturesManager.loadNPC(name, script);
			NPC *tempnpc = CreaturesManager.getNPC(name);
			if(tempnpc)
			{
				tempnpc->setPos(posx, posy, posz);
				tempnpc->setLooktype(looktype);
				tempnpc->setName(name);
				Tile *tilet = Map.getTile(tempnpc->getPosX(), tempnpc->getPosY(), tempnpc->getPosZ());
				if(tilet)
					tilet->addThing(tempnpc);
			}
		}
		std::cout<<" Done!";
	}
	else
	{
		coutCurrentTime();
		std::cout << "\n::Error while opening npc.txt\n";
		return false;
	}
	npcfile.close();

	std::cout<<" ( " << clock() - time_started << " ms )\n";   
	return true;
}

bool Game::loadOnlinePlayersRecord()
{
    clock_t time_started = clock(); 
    coutCurrentTime();
    std::cout<<"::Loading OnlinePlayersRecord.txt...";      

	std::string filename = "OnlinePlayersRecord.txt";
        
    std::ifstream plik(filename.c_str());
    {
       if(plik.is_open())
       {
          std::string bufor_parse, temp;

          char cc;
          while(true)
          {
             cc = plik.get();
             if(plik.eof()) break;
             bufor_parse += cc;
          }    
          std::string c, ss;
          int n = bufor_parse.length();
          int licznik = 1;
          for(int i = 0; i < n; i++)
          { 
             c = bufor_parse[i];
             if(c.c_str() != "|")
             {
                ss += c;
             }
             if(c == "|")
             {
                if( licznik == 1)
					onlinePlayersRecord = atoi(ss.c_str());
                if( licznik == 2)
				   onlinePlayersRecordDate = ss;

                licznik++;
                ss = "";
             }
          }       
	      std::cout<<" Done!";
          plik.close();
       }
	   else
	   {
		   onlinePlayersRecord = 0;
		   onlinePlayersRecordDate = "None";

		   std::cout << "\n";
           std::cout << "         ::Error while opening OnlinePlayersRecord.txt";
	   }
    }
	std::cout<<" ( " << clock() - time_started << " ms )\n"; 

	return true;
}

bool Game::saveOnlinePlayersRecord(unsigned int record)
{
	coutCurrentTime();
	cout << "::New online record: " << record << " players online!" << endl;

	string sending_message = "New online record: ";
	sending_message += intToStr(record);
	sending_message += " players online!";       
	this->sendMessageToAllPlayers(1 ,"System" ,0 , sending_message);

	stringstream date;
    SYSTEMTIME st;
    GetLocalTime(&st);
    if(st.wHour < 10)
        date << "0" << st.wHour << ":";
    else 
		date << st.wHour << ":";
    
    if(st.wMinute < 10)
        date << "0" << st.wMinute << ":";
    else 
		date << st.wMinute << ":";
    
    if(st.wSecond < 10)
        date << "0" << st.wSecond;
    else 
		date << st.wSecond;

    std::string filename = "OnlinePlayersRecord.txt";
    std::ofstream plik(filename.c_str());
    {
		plik<<record<<"|";
	    plik<<date.str().c_str()<<"|";
        plik.close();
    }
	onlinePlayersRecord = record;
	onlinePlayersRecordDate = date.str();
	return true;
}


void Game::serverSave(bool logout)
{
	this->sendMessageToAllPlayers(1 ,"System" ,0 , "Server is saving. Please wait...");
	//Saving events:
	CreaturesManager.saveAllPlayers();
	//End of saving events.
	std::string message = "Server saved. Next save in ";
	message += intToStr(ConfigManager.saveInterval/60);
	message += " minutes.";
	this->sendMessageToAllPlayers(1 ,"System" ,0 , message);
	#ifdef _DEBUG
		coutCurrentTime(); 
		std::cout << "::Server saved.\n";
	#endif
}

Player* Game::getPlayerByID(unsigned int _id)
{
   return CreaturesManager.getPlayer(_id);
}

Player* Game::getPlayerByName(string _name)
{
   return CreaturesManager.getPlayer(_name);
}

Creature* Game::getCreatureByID(unsigned int _id)
{
	return CreaturesManager.getCreature(_id);
}

ClientContext* Game::getClientByPlayerName(string _name)
{
	for(unsigned int i = 0; i < this->clientsList.size(); i++)           
		if(this->clientsList[i].player && this->clientsList[i].player->getName() == _name)
			return &(this->clientsList[i]);
	return NULL;
}

ClientContext* Game::getClientByPlayerID(unsigned int _id)
{
	for(unsigned int i = 0; i < this->clientsList.size(); i++)           
		if(this->clientsList[i].player && this->clientsList[i].player->getId() == _id)
			return &(this->clientsList[i]);
	return NULL;
}

ClientContext* Game::getClientByPlayer(Player* player)
{
	return getClientByPlayerID(player->getId());
}

bool Game::isValidName(std::string name, std::string &result)
{
	if(name.size() < 3) 
	{
		result = "Name must have at least 3 characters.";
		return false;
	}
	if(name.size() > 12) 
	{
		result = "Name is too long.";
		return false;
	}
	if(!isLettersOnly(name))
	{
		result = "Name contains non-letter characters.";
		return false;
	}
	if((name.find("GM ") != string::npos) || (name.find("GOD ") != string::npos) 
	|| (name.find("Gm ") != string::npos) || (name.find("God ") != string::npos) 
	|| (name.find("GoD ") != string::npos) || (name.find("gm ") != string::npos) 
	|| (name.find("god ") != string::npos) || (name.find("Admin ") != string::npos)
	|| (name.find("admin ") != string::npos) || (name.find("ADMIN ") != string::npos)) 
	{
		result = "Name contains prohibited symbols.";
		return false;
	}
	return true;
}

void Game::processPlayerLogin(Player *player)
{
	if(player)
	{
		Tile *tile = Map.getTile(player->getPosX(),player->getPosY(),player->getPosZ());
		if(tile)
			tile->addThing(player);
	
		this->sendPlayerStatistics(player);
  
		Item *tempItem = NULL;

		if(player->equipment.lRingEar)
		{
			tempItem = player->equipment.lRingEar;
			player->equipment.lRingEar = NULL;
			this->equipItem(player, 1, tempItem);
		}
		if(player->equipment.head)
		{
			tempItem = player->equipment.head;
			player->equipment.head = NULL;
			this->equipItem(player, 2, tempItem);
		}
		if(player->equipment.rRingEar)
		{
			tempItem = player->equipment.rRingEar;
			player->equipment.rRingEar = NULL;
			this->equipItem(player, 3, tempItem);
		}
		if(player->equipment.necklace)
		{
			tempItem = player->equipment.necklace;
			player->equipment.necklace = NULL;
			this->equipItem(player, 4, tempItem);
		}
		if(player->equipment.armor)
		{
			tempItem = player->equipment.armor;
			player->equipment.armor = NULL;
			this->equipItem(player, 5, tempItem);
		}
		if(player->equipment.backpack)
		{
			tempItem = player->equipment.backpack;
			player->equipment.backpack = NULL;
			this->equipItem(player, 6, tempItem);
		}
		if(player->equipment.weapon)
		{
			tempItem = player->equipment.weapon;
			player->equipment.weapon = NULL;
			this->equipItem(player, 7, tempItem);
		}
		if(player->equipment.belt)
		{
			tempItem = player->equipment.belt;
			player->equipment.belt = NULL;
			this->equipItem(player, 8, tempItem);
		}
		if(player->equipment.shield)
		{
			tempItem = player->equipment.shield;
			player->equipment.shield = NULL;
			this->equipItem(player, 9, tempItem);
		}
		if(player->equipment.lRing)
		{
			tempItem = player->equipment.lRing;
			player->equipment.lRing = NULL;
			this->equipItem(player, 10, tempItem);
		}
		if(player->equipment.legs)
		{
			tempItem = player->equipment.legs;
			player->equipment.legs = NULL;
			this->equipItem(player, 11, tempItem);
		}
		if(player->equipment.rRing)
		{
			tempItem = player->equipment.rRing;
			player->equipment.rRing = NULL;
			this->equipItem(player, 12, tempItem);
		}
		if(player->equipment.gloves)
		{
			tempItem = player->equipment.gloves;
			player->equipment.gloves = NULL;
			this->equipItem(player, 13, tempItem);
		}
		if(player->equipment.boots)
		{
			tempItem = player->equipment.boots;
			player->equipment.boots = NULL;
			this->equipItem(player, 14, tempItem);
		}
		if(player->equipment.arrows)
		{
			tempItem = player->equipment.arrows;
			player->equipment.arrows = NULL;
			this->equipItem(player, 15, tempItem);
		}

		unsigned int sendId = 100;
		sf::Packet toSend;
		toSend << sendId << player->getId() << player->getPosX() << player->getPosY() << player->getPosZ();
		player->socket->send(toSend);

		this->sendPlayerStartingItemStorage(player);
		this->onPlayerLogin(player);

		this->sendPlayerStartingTiles(player);
		this->sendPlayerLoginToPlayers(player);
		this->createEffect(player->getPosX(), player->getPosY(), player->getPosZ(), 4);
		this->sendPlayerStartingCreatures(player);

		this->sendMessageToPlayer(player, 1 ,"" ,0 , "Welcome to Aldera.");
		this->sendMessageToPlayer(player, 1 ,"" ,0 , "If you encounter any bugs, write !bug MESSAGE'");
		if(player->getFlag(1) < 1)
		{
			this->sendMessageToPlayer(player, 1 ,"" ,0 , "This is your first log in, yea?");
			this->sendMessageToPlayer(player, 1 ,"" ,0 , "Don't be afraid to ask others for help ;)");
			player->setFlag(1,1);
		}
	}
}

void Game::sendPlayerStatistics(Player *player)
{
	sf::Packet toSend;
	int sendId = 99;
	toSend.clear();
	toSend << sendId;
	toSend << player->getLevel() << player->getExperience() << player->sex << player->getVocation() << player->getSpeed() 
		<< player->getSkillPoints() << player->getStatisticsPoints()
		<< player->getStrength() << player->getDexterity() << player->getIntelligence() << player->getMagicPower()
		<< player->getHealth() << player->getMaxHealth() << player->getMana() << player->getMaxMana();

	player->socket->send(toSend);
}

void Game::sendPlayerStartingTiles(Player *player)
{
	unsigned char zz1, zz2;
	if(player->getPosZ() >= 7)
	{
		zz1 = 6;
		zz2 = 14;
	}
	else
	{
		zz1 = player->getPosZ() - 2;
		zz2 = 7;
	}

	for(zz1; zz1 <= zz2; zz1++)
	{
		for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
		{
			for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
			{ 
				Tile *tile = Map.getTile(x, y, zz1);
				if(tile)
					this->sendTileToPlayer(player,tile);
			}     
		}
	}
}

void Game::sendPlayerStartingCreatures(Player *player)
{
	sf::Packet toSend;
	int pid = 111;
	
	deque<Creature*> spectators;
	Map.getSpectators(player->getPosX(), player->getPosY(), player->getPosZ(), spectators);

	for(deque<Creature*>::iterator it = spectators.begin(); it != spectators.end(); it++)
	{
		Player *player2 = dynamic_cast<Player*>(*it);
		if(player == player2)
			continue;

		toSend.clear();
		toSend << pid << (*it)->getPosX() << (*it)->getPosY() << (*it)->getPosZ() << (*it)->getId() << (*it)->getName() << (*it)->getHealthPercent() << (*it)->getLooktype() << (*it)->getDirection() << (*it)->getSpeed() << (*it)->getLightLevel() << false;
		player->socket->send(toSend);
	}
}

void Game::sendPlayerStartingItemStorage(Player *player)
{
	sf::Packet toSend;
	int pid = 106;
	unsigned int bpid = 20;

	if(player->itemStorage->size() != 0)
	{
		for(unsigned int iter = 0; iter < player->itemStorage->size(); iter++)
		{
			toSend.clear();
			toSend << pid << bpid << player->itemStorage->getItem(char(iter))->mID << player->itemStorage->getItem((iter))->count;
			player->socket->send(toSend);
		}
	}
}

void Game::sendPlayerCreature(Player *player, Creature *creature, bool move)
{
	sf::Packet toSend;
	int pid = 111;

	toSend << pid << creature->getPosX() << creature->getPosY() << creature->getPosZ() << creature->getId() << creature->getName() << creature->getHealthPercent() << creature->getLooktype() << creature->getDirection() << creature->getSpeed() << creature->getLightLevel() << move;
	player->socket->send(toSend);
}

void Game::sendPlayerLoginToPlayers(Player *player)
{
	sf::Packet toSend;
	int pid = 111;
	toSend << pid << player->getPosX() << player->getPosY() << player->getPosZ() << player->getId() << player->getName() << player->getHealthPercent() << player->getLooktype() << player->getDirection() << player->getSpeed() << player->getLightLevel() << false;

	deque<Player*> spectators;
	CreaturesManager.getPlayersInArea(spectators, player->getPosX(), player->getPosY(), player->getPosZ());

	for(deque<Player*>::iterator it = spectators.begin(); it != spectators.end(); it++)
	{
		if((*it)->socket)
			(*it)->socket->send(toSend);
	}
}

void Game::sendCreatureUpdateHealth(Creature *creature)
{
	sf::Packet toSend;
	int pid = 140;
	toSend << pid << creature->getId() << creature->getHealthPercent();

	this->sendPacketToPlayersInArea(toSend, creature->getPosX(), creature->getPosY(), creature->getPosZ());
}

void Game::sendCreatureUpdateLight(Creature *creature)
{
	sf::Packet toSend;
	int pid = 141;
	toSend << pid << creature->getId() << creature->getLightLevel();

	this->sendPacketToPlayersInArea(toSend, creature->getPosX(), creature->getPosY(), creature->getPosZ());
}

void Game::sendCreatureUpdateSpeed(Creature *creature)
{
	if(creature)
	{
		sf::Packet toSend;
		int pid = 143;
		/*
		int speed = creature->getSpeed();
		if(creature->condition->getSpeedChange() != 0)
			speed += int(float(creature->getSpeed())*100.0f/float(creature->condition->getSpeedChange()));

		toSend << pid << creature->getId() << static_cast<unsigned int>(speed);
		*/
		toSend << pid << creature->getId() << creature->getSpeed();
		this->sendPacketToPlayersInArea(toSend, creature->getPosX(), creature->getPosY(), creature->getPosZ());
	}
}

void Game::onPlayerLogin(Player *player)
{
	if(CreaturesManager.getPlayersCount() > onlinePlayersRecord)
	{
		this->saveOnlinePlayersRecord(CreaturesManager.getPlayersCount());
	}
	if(!player->itemStorage->isHoldingItemId(283))
		this->addItemToPlayerStorage(player, new Item(283,10), 20);
	if(!player->itemStorage->isHoldingItemId(357))
		this->addItemToPlayerStorage(player, new Item(357,100), 20);
	if(!player->itemStorage->isHoldingItemId(358))
		this->addItemToPlayerStorage(player, new Item(358,100), 20);
	if(!player->itemStorage->isHoldingItemId(419))
		this->addItemToPlayerStorage(player, new Item(419), 20);
	if(!player->itemStorage->isHoldingItemId(460))
		this->addItemToPlayerStorage(player, new Item(460), 20);
	if(!player->itemStorage->isHoldingItemId(469))
		this->addItemToPlayerStorage(player, new Item(469), 20);

	this->addItemToPlayerStorage(player, new Item(461,1000), 20);
	this->addItemToPlayerStorage(player, new Item(470,1000), 20);
}

void Game::onCreatureDisappear(Creature *creature)
{
	if(creature)
	{
		int posx = creature->getPosX();
		int posy = creature->getPosY();
		int posz = creature->getPosZ();
		unsigned int cid = creature->getId();

		if( ConfigManager.save && dynamic_cast<Player*>(creature) ) //przy "znikniêciu" creature, sprawdzam, czy to nie gracz, jak tak to zapisujê
			CreaturesManager.savePlayer(cid);

		Map.getTile(posx, posy, posz)->removeCreatureByID(cid);
		CreaturesManager.removeCreature(cid);
		std::deque<Player*> spectators;

		sf::Packet toSend;
		int pid = 113;
		toSend << pid << cid;
		this->sendPacketToPlayersInArea(toSend, posx, posy, posz);
	}
}

void Game::sendPlayerTargetConfirmation(Player *player, unsigned int cid, bool cancel) //wys³anie graczowi zaznaczenia creatury
{
	if(player)
	{
		sf::Packet toSend;
		int pid = 220;

		toSend.clear();
		toSend << pid << cid << cancel;
		player->socket->send(toSend);
	}
}

void Game::onCreatureDeath(Creature *creature)
{
	if(creature)
	{
		std::deque<Player*> spectators;
		Player *player = dynamic_cast<Player*>(creature);
		Monster *monster = dynamic_cast<Monster*>(creature);
		if(player)
		{
			if(player->access >= 5) //nieœmiertelnoœæ GM'ów...
			{
				creature->addHealth(10000);
				this->sendCreatureUpdateHealth(creature);
				this->createTextEffect(player->getPosX(), player->getPosY(), player->getPosZ(), 1, "Immortal object");
				return;
			}
			sf::Packet toSend;
			int pid = 0;
			toSend << pid;
			player->socket->send(toSend);

			for(deque<ClientContext>::iterator it = clientsList.begin(); it != clientsList.end(); it++)
			{
				if(it->player == player)
				{
					#ifdef _DEBUG
						//std::cout << "::Player dead, client disconnected - ip: " << it->socket->GetRemoteAddress() << ", id: " << it->id << std::endl;
					#endif

					selector.remove(*(it->socket));
					it->socket->disconnect();
					it->player = NULL;
					it->lostPlayer = true;

					clientsList.erase(it);
					break;
				}
			}
		}
		if(monster)
		{
			std::deque<ReceivedDamageFrom> *attackersList = monster->getAttackersList();
			for (deque<ReceivedDamageFrom>::iterator it = attackersList->begin(); it < attackersList->end(); it++)
			{
				unsigned int receivedExperience = float(it->damage)/float(monster->getMaxHealth())*float(monster->getExperience());
				Player *player2 = CreaturesManager.getPlayer(it->dealer);
				this->addPlayerExperience(player2, receivedExperience);
			}
		}
		Map.getTile(creature->getPosX(), creature->getPosY(), creature->getPosZ())->removeCreatureByID(creature->getId());

		sf::Packet toSend;
		int pid = 113; //creature disappear
		toSend << pid << creature->getId();
		this->sendPacketToPlayersInArea(toSend, creature->getPosX(), creature->getPosY(), creature->getPosZ());

		Tile *tile = Map.getTile(creature->getPosX(), creature->getPosY(), creature->getPosZ());

		if(BaseItems.getItemType(creature->getCorpseId())->isContainer())
		{
			Container *corpse = new Container(creature->getCorpseId());
			this->addThingToTile(corpse, tile);
			if(monster)
			{
				string loot = monster->getName();
				loot += " dropped: ";
				for(unsigned int i = 0; i < monster->items.size(); i++)
				{
					if(corpse->size() < corpse->capacity())
					{
						this->addItemToContainer(corpse, monster->items.at(i));
						if(monster->items.at(i)->count == 1)
						{
							if(monster->items.at(i)->baseItem->name.at(0) == 'a' || monster->items.at(i)->baseItem->name.at(0) == 'e'
							|| monster->items.at(i)->baseItem->name.at(0) == 'i' || monster->items.at(i)->baseItem->name.at(0) == 'o'
							|| monster->items.at(i)->baseItem->name.at(0) == 'u')
								loot += "an ";
							else
								loot += "a ";
							
							loot += monster->items.at(i)->baseItem->name;
						}
						else
						{
							loot += intToStr(monster->items.at(i)->count);
							loot += " ";
							loot += monster->items.at(i)->baseItem->pluralName;
						}
						if(i+1 < monster->items.size())
							loot += ", ";
					}
					else
						break;
				}
				if(monster->items.size() == 0)
					loot += "nothing";

				monster->items.clear();
				std::deque<ReceivedDamageFrom> *attackersList = monster->getAttackersList();
				for(deque<ReceivedDamageFrom>::iterator it = attackersList->begin(); it < attackersList->end(); it++)
					this->sendMessageToPlayer(this->getPlayerByName(it->dealer), 1, "", 0, loot);

				CreaturesManager.deathCreature(creature->getId());
			}
			if(player)
			{
				for(unsigned int i = 0; i < player->itemStorage->itemList.size(); i++)
				{
					if(corpse->size() < corpse->capacity())
						this->addItemToContainer(corpse, player->itemStorage->getItem(i));
					else
						break;
				}
				player->itemStorage->itemList.clear();
			}
		}
		else
			this->addThingToTile(new Item(creature->getCorpseId()),tile);

		if(player)
		{
			player->setHealth(player->getMaxHealth());
			player->setMana(player->getMaxMana());
			player->setPos(player->spawnPosX, player->spawnPosY, player->spawnPosZ);

			if(ConfigManager.save)
				CreaturesManager.saveAllPlayers();

			CreaturesManager.removePlayer(player->getName());
		}
	}
}

void Game::attackCreature(Creature *receiver, Creature *attacker, CombatType combatType, unsigned char spellID)
{
	if( (ConfigManager.pvp == true || (attacker && dynamic_cast<Monster*>(attacker)) || (receiver && dynamic_cast<Monster*>(receiver)))
		&& this->canDoCombat(attacker) && this->canDoCombat(receiver) && receiver->getName() != "" && !dynamic_cast<NPC*>(receiver) && !dynamic_cast<NPC*>(attacker))
	{
		if(spellID == 0)
		{
			if(receiver && attacker && receiver->getTotalBasicDefence(combatType) < (rand() % 101))//ca³kowita obrona tarcz¹ < wylosowanej liczby 0-100
			{
				int damage = rand() % attacker->getMaximumAttack(combatType) + 1;

				if(receiver->getTotalBasicArmor(combatType) < damage)//ca³kowita obrona pancerzy mniejsza od wylosowanych obra¿eñ
				{
					#ifdef _DEBUG
						cout << "::" << attacker->getName() << " dealed " << damage << " to: " << receiver->getName() << endl;
					#endif

					damage = damage - receiver->getTotalBasicArmor(combatType); //zabieramy z obra¿eñ to co zosta³o na pancerzu

					if(damage > receiver->getHealth())
						receiver->addDamageToDamageList(receiver->getHealth(), attacker->getName());
					else
						receiver->addDamageToDamageList(damage, attacker->getName());

					receiver->addHealth(-damage);
					this->sendCreatureUpdateHealth(receiver);
					this->createEffect(receiver->getPosX(), receiver->getPosY(), receiver->getPosZ(), 3);//blood hit
					this->createTextEffect(receiver->getPosX(), receiver->getPosY(), receiver->getPosZ(), 2, intToStr(damage));

					if(receiver->getHealth() == 0)
						this->onCreatureDeath(receiver);

					attacker->setTarget(NULL);
				}
				else //cios nie przebi³ siê przez pancerz
				{
					this->createEffect(receiver->getPosX(), receiver->getPosY(), receiver->getPosZ(), 11); //armor blocked
				}
			}
			else if(receiver)//przyjêto atak na tarczê
			{
				this->createEffect(receiver->getPosX(), receiver->getPosY(), receiver->getPosZ(), 2); //dymek
			}
		}
		else if(spellID > 0)
		{
			if(receiver && attacker)
			{
				int minimumAttack = attacker->getMaximumSpellAttack(spellID)/2;
				int maximumAttack = attacker->getMaximumSpellAttack(spellID);
				int damage = rand()%(maximumAttack - minimumAttack) + minimumAttack + 1;
				int resistance = receiver->getDefenceModificator(combatType);
				damage = damage - (damage*resistance/100); //ostateczne obra¿enia = obra¿enia - obra¿enia*(%odpornoœci)/100

				if(damage > receiver->getHealth())
					receiver->addDamageToDamageList(receiver->getHealth(), attacker->getName());
				else
					receiver->addDamageToDamageList(damage, attacker->getName());

				receiver->addHealth(-damage);
				this->sendCreatureUpdateHealth(receiver);
				this->createEffect(receiver->getPosX(), receiver->getPosY(), receiver->getPosZ(), 3);//blood hit
				this->createTextEffect(receiver->getPosX(), receiver->getPosY(), receiver->getPosZ(), 2, intToStr(damage));

				if(receiver->getHealth() == 0)
					this->onCreatureDeath(receiver);

				attacker->setTarget(NULL);
			}
		}
	}
	else if(attacker)
	{
		attacker->setTarget(NULL);
		this->sendPlayerTargetConfirmation(dynamic_cast<Player*>(attacker), 0, true);
	}
}

void Game::createEffect(unsigned int x, unsigned int y, unsigned char z, unsigned char type)
{
	sf::Packet toSend;
	int pid = 230;
	toSend << pid << x << y << z << type;
	
	this->sendPacketToPlayersInArea(toSend, x, y, z);
}

void Game::createTextEffect(unsigned int x, unsigned int y, unsigned char z, unsigned char color, string text)
{
	sf::Packet toSend;
	int pid = 231;
	toSend << pid << x << y << z << color << text;
	
	this->sendPacketToPlayersInArea(toSend, x, y, z);
}

void Game::createShootEffect(unsigned int fromx, unsigned int fromy, unsigned char fromz, unsigned int tox, unsigned int toy, unsigned char toz, unsigned char type)
{
	sf::Packet toSend;
	int pid = 232;
	toSend << pid << fromx << fromy << fromz << tox << toy << toz << type;
	
	deque<Player*> spectators;
	deque<Player*> uniqueSpectators;
	CreaturesManager.getPlayersInArea(spectators, fromx, fromy, fromz);
	CreaturesManager.getPlayersInArea(spectators, tox, toy, toz);

	bool push;
	for(deque<Player*>::iterator it = spectators.begin(); it != spectators.end(); it++)
	{
		push = true;
		for(deque<Player*>::iterator it2 = uniqueSpectators.begin(); it2 != uniqueSpectators.end(); it2++)
		{
			if(*it == *it2)
			{
				push = false;
				break;
			}
		}
		if(push)
			uniqueSpectators.push_back(*it);
	}

	for(deque<Player*>::iterator it = uniqueSpectators.begin(); it != uniqueSpectators.end(); it++)
	{
		if((*it)->socket)
		{
			(*it)->socket->send(toSend);
		}  
	}
}

bool Game::addItemToContainer(Container *container, Item *item)
{
	if(container->size() < container->capacity())
	{
		if(item->parent)
		{
			Tile *tilet = dynamic_cast<Tile*>(item->parent);
			if(tilet)
			{
				removeThingFromTile(tilet->getIndexOfThing(item), tilet);
			}
		}

		container->addItem(item);
		Tile *tile = dynamic_cast<Tile*>(container->parent);
		if(tile)
		{
			sf::Packet toSend;
			int pid = 190;
			toSend << pid << tile->pos.x << tile->pos.y << tile->pos.z << tile->getIndexOfThing(container) << item->mID << item->count;
			this->sendPacketToPlayersInArea(toSend, tile->pos.x, tile->pos.y, tile->pos.z);
			return true;
		}
	}
	return false;
}

bool Game::removeItemFromContainer(Container *container, Item *item)
{
	Tile *tile = dynamic_cast<Tile*>(container->parent);
	if(tile)
	{
		sf::Packet toSend;
		int pid = 191;
		toSend << pid << tile->pos.x << tile->pos.y << tile->pos.z << tile->getIndexOfThing(container) << container->getItemIndex(item);
		this->sendPacketToPlayersInArea(toSend, tile->pos.x, tile->pos.y, tile->pos.z);
		container->eraseItem(container->getItemIndex(item));
		return true;
	}
	return false;
}

bool Game::removeItemFromPlayerEquipment(Player *player, int slot, unsigned short count)
{
	unsigned short slotPos = 0;
	if(slot == 1)
		slotPos = 1;
	else if(slot == 2)
		slotPos = 2;
	else if(slot == 3)
		slotPos = 4;
	else if(slot == 4)
		slotPos = 8;
	else if(slot == 5)
		slotPos = 16;
	else if(slot == 6)
		slotPos = 32;
	else if(slot == 7)
		slotPos = 64;
	else if(slot == 8)
		slotPos = 128;
	else if(slot == 9)
		slotPos = 256;
	else if(slot == 10)
		slotPos = 512;
	else if(slot == 11)
		slotPos = 1024;
	else if(slot == 12)
		slotPos = 2048;
	else if(slot == 13)
		slotPos = 4096;
	else if(slot == 14)
		slotPos = 8192;
	else if(slot == 15)
		slotPos = 16384;

	unsigned short sendSlot = 255;
	if(slotPos == SLOTP_LEARRING && player->equipment.lRingEar)
	{
		player->equipment.lRingEar = NULL;
		sendSlot = 1;
	}
	else if(slotPos == SLOTP_HEAD && player->equipment.head) 
	{
		player->equipment.head = NULL;
		sendSlot = 2;
	}
	else if(slotPos == SLOTP_REARRING && player->equipment.rRingEar)  
	{
		player->equipment.rRingEar = NULL;
		sendSlot = 3;
	}
	else if(slotPos == SLOTP_NECKLACE && player->equipment.necklace)    
	{
		player->equipment.necklace = NULL;
		sendSlot = 4;
	}
	else if(slotPos == SLOTP_ARMOR && player->equipment.armor) 
	{
		player->equipment.armor = NULL;
		sendSlot = 5;
	}
	else if(slotPos == SLOTP_BACKPACK && player->equipment.backpack)
	{
		player->equipment.backpack = NULL;
		sendSlot = 6;
	}
	else if(slotPos == SLOTP_WEAPON && player->equipment.weapon) 
	{
		if(count != 0)
			player->equipment.weapon->count-=count;
		if(count == 0 || player->equipment.weapon->count == 0)
			player->equipment.weapon = NULL;
		sendSlot = 7;
	}
	else if(slotPos == SLOTP_BELT && player->equipment.belt)   
	{
		player->equipment.belt = NULL;
		sendSlot = 8;
	}
	else if(slotPos == SLOTP_SHIELD && player->equipment.shield) 
	{
		if(count != 0)
			player->equipment.shield->count-=count;
		if(count == 0 || player->equipment.shield->count == 0)
			player->equipment.shield = NULL;
		sendSlot = 9;
	}
	else if(slotPos == SLOTP_LRING && player->equipment.lRing)    
	{
		player->equipment.lRing = NULL;
		sendSlot = 10;
	}
	else if(slotPos == SLOTP_LEGS && player->equipment.legs)
	{
		player->equipment.legs = NULL;
		sendSlot = 11;
	}
	else if(slotPos == SLOTP_RRING && player->equipment.rRing)   
	{
		player->equipment.rRing = NULL;
		sendSlot = 12;
	}
	else if(slotPos == SLOTP_GLOVES && player->equipment.gloves)   
	{
		player->equipment.gloves = NULL;
		sendSlot = 13;
	}
	else if(slotPos == SLOTP_BOOTS && player->equipment.boots)
	{
		player->equipment.boots = NULL;
		sendSlot = 14;
	}
	else if(slotPos == SLOTP_ARROWS && player->equipment.arrows)  
	{
		if(count != 0)
			player->equipment.arrows->count-=count;
		if(count == 0 || player->equipment.arrows->count == 0)
			player->equipment.arrows = NULL;
		sendSlot = 15;
	}
	if(sendSlot != 255)
	{
		sf::Packet toSend;
		int pid = 80;
		toSend << pid << sendSlot << count;
		player->socket->send(toSend);
		return true;
	}
	return false;
}

bool Game::equipItem(Player *player, int slot, Item *item)
{
	unsigned short sendSlot = 255;

	unsigned short slotPos = 0;

	if(slot == 1)
		slotPos = 1;
	else if(slot == 2)
		slotPos = 2;
	else if(slot == 3)
		slotPos = 4;
	else if(slot == 4)
		slotPos = 8;
	else if(slot == 5)
		slotPos = 16;
	else if(slot == 6)
		slotPos = 32;
	else if(slot == 7)
		slotPos = 64;
	else if(slot == 8)
		slotPos = 128;
	else if(slot == 9)
		slotPos = 256;
	else if(slot == 10)
		slotPos = 512;
	else if(slot == 11)
		slotPos = 1024;
	else if(slot == 12)
		slotPos = 2048;
	else if(slot == 13)
		slotPos = 4096;
	else if(slot == 14)
		slotPos = 8192;
	else if(slot == 15)
		slotPos = 16384;

	if(slotPos == SLOTP_LEARRING && !player->equipment.lRingEar)
	{
		player->equipment.lRingEar = item;
		sendSlot = 1;
	}
	else if(slotPos == SLOTP_HEAD && !player->equipment.head) 
	{
		player->equipment.head = item;
		sendSlot = 2;
	}
	else if(slotPos == SLOTP_REARRING && !player->equipment.rRingEar)  
	{
		player->equipment.rRingEar = item;
		sendSlot = 3;
	}
	else if(slotPos == SLOTP_NECKLACE && !player->equipment.necklace)    
	{
		player->equipment.necklace = item;
		sendSlot = 4;
	}
	else if(slotPos == SLOTP_ARMOR && !player->equipment.armor) 
	{
		player->equipment.armor = item;
		sendSlot = 5;
	}
	else if(slotPos == SLOTP_BACKPACK && !player->equipment.backpack)
	{
		player->equipment.backpack = item;
		sendSlot = 6;
	}
	else if(slotPos == SLOTP_WEAPON && !player->equipment.weapon) 
	{
		if(item->baseItem->slotPosition == 32768 && player->equipment.shield)
				return false;

		player->equipment.weapon = item;
		sendSlot = 7;
	}
	else if(slotPos == SLOTP_BELT && !player->equipment.belt)   
	{
		player->equipment.belt = item;
		sendSlot = 8;
	}
	else if(slotPos == SLOTP_SHIELD && !player->equipment.shield) 
	{
		player->equipment.shield = item;
		sendSlot = 9;
	}
	else if(slotPos == SLOTP_LRING && !player->equipment.lRing)    
	{
		player->equipment.lRing = item;
		sendSlot = 10;
	}
	else if(slotPos == SLOTP_LEGS && !player->equipment.legs)
	{
		player->equipment.legs = item;
		sendSlot = 11;
	}
	else if(slotPos == SLOTP_RRING && !player->equipment.rRing)   
	{
		player->equipment.rRing = item;
		sendSlot = 12;
	}
	else if(slotPos == SLOTP_GLOVES && !player->equipment.gloves)   
	{
		player->equipment.gloves = item;
		sendSlot = 13;
	}
	else if(slotPos == SLOTP_BOOTS && !player->equipment.boots)
	{
		player->equipment.boots = item;
		sendSlot = 14;
	}
	else if(slotPos == SLOTP_ARROWS && !player->equipment.arrows)  
	{
		player->equipment.arrows = item;
		sendSlot = 15;
	}
	if(sendSlot != 255)
	{
		sf::Packet toSend;
		int pid = 81;
		toSend << pid << sendSlot << item->mID << item->count;
		player->socket->send(toSend);
		return true;
	}
	else
		return false;
}

bool Game::addItemToPlayerStorage(Player *player, Item *item, unsigned int bpid)
{
	if(player->itemStorage->getItemHoldingCount() < player->itemStorage->capacity())
	{
		if(item->baseItem->stackable)
		{
			Item* itemOld = player->itemStorage->getItemId(item->mID);
			if(itemOld && itemOld->count < 1000 && itemOld->count + item->count <= 1000)
			{
				itemOld->count += item->count;
			}
			else if(itemOld && itemOld->count < 1000 && itemOld->count + item->count > 1000)
			{
				unsigned short tempCount = item->count;
				item->count = itemOld->count + item->count - 1000;
				itemOld->count = 1000;
				player->itemStorage->addItem(item);
				//this->addItemToPlayerStorage(player, item, bpid);
				item->parent = player->itemStorage;

				sf::Packet toSend;
				int pid = 106;
				toSend << pid << bpid << item->mID << tempCount;
				player->socket->send(toSend);
				return true;
			}
			else
			{
				player->itemStorage->addItem(item);
				item->parent = player->itemStorage;
			}
		}
		else
		{
			player->itemStorage->addItem(item);
			item->parent = player->itemStorage;
		}
			
		sf::Packet toSend;
		int pid = 106;
		toSend << pid << bpid << item->mID << item->count;
		player->socket->send(toSend);
		return true;
	}
	return false;
}

bool Game::removeItemFromPlayerStorage(Player *player, unsigned char index, unsigned int bpid)
{
	if(bpid == 20 && player->itemStorage->size() > index)
	{
		player->itemStorage->eraseItem(index);
		sf::Packet toSend;
		int pid = 107;
		toSend << pid << bpid << index;
		player->socket->send(toSend);
		return true;
	}
	return false;
}

bool Game::removeItemFromPlayerStorage(Player *player, Item *item)
{
	if(player->itemStorage->isHoldingItem(item))
	{
		player->itemStorage->eraseItem(item);
		sf::Packet toSend;
		int pid = 108;
		toSend << pid << item->mID << item->count;
		player->socket->send(toSend);
		return true;
	}
	return false;
}

bool Game::removeItemIdFromPlayerStorage(Player *player, unsigned int id, unsigned short count)
{
	if(player->itemStorage->isHoldingItemId(id, count))
	{
		player->itemStorage->eraseItemId(id, count);
		sf::Packet toSend;
		int pid = 108;
		toSend << pid << id << count;
		player->socket->send(toSend);
		return true;
	}
	return false;
}

void Game::createItemOnTile(unsigned int id, unsigned short count, Tile *tile)//NIE WYSY£A INFO DO LUDZI!!!!!!!!!!!!!!!!!
{
	if(BaseItems.getItemType(id)->isContainer())
	{
		Item* itemek;
		itemek = new Container(id);
		tile->addThing(itemek);
	}
	else
	{
		Item* itemek;
		itemek = new Item( id, count);
		tile->addThing(itemek);
	}
}

void Game::addThingToTile(Thing *thing, Tile *tile)
{
	if(tile && tile->canAddThing() == 1)
	{
		Item *item = dynamic_cast<Item*>(thing);
		if(item && item->getContainer())
		{
			tile->addThing(thing);

			Container *container = dynamic_cast<Container*>(thing);
			unsigned int id = 102;
			sf::Packet toSend;
			toSend << id << tile->pos.x << tile->pos.y << tile->pos.z << container->mID << container->count;
			this->sendPacketToPlayersInArea(toSend, tile->pos.x, tile->pos.y, tile->pos.z);
		}
		else if(item)
		{
			tile->addThing(thing);

			unsigned int id = 102;
			sf::Packet toSend;
			toSend << id << tile->pos.x << tile->pos.y << tile->pos.z << item->mID << item->count;
			this->sendPacketToPlayersInArea(toSend, tile->pos.x, tile->pos.y, tile->pos.z);
		}
	}
	else if(tile && tile->canAddThing() == TILE_TELEPORT)
	{
		Item *teleport = tile->getTopItem();

		if(teleport && teleport->abilities)
		{
			if(teleport->abilities->destPosX != 0 || teleport->abilities->destPosY != 0 || teleport->abilities->destPosZ != 0)
			{
				Tile *destTile = Map.getTile(teleport->abilities->destPosX, teleport->abilities->destPosY, teleport->abilities->destPosZ);
				addThingToTile(thing, destTile);
			}
		}
		else
		{
			addThingToTile(thing, tile);
		}
	}
	else if(tile && tile->canAddThing() == TILE_FLOOR_CHANGE_DOWN)
	{
		Tile *tile2 = Map.getTile(tile->pos.x+1, tile->pos.y+2, tile->pos.z-1);
		addThingToTile(thing, tile2);
	}
	else if(tile && tile->canAddThing() == TILE_FLOOR_CHANGE_DOWN_NORTH)
	{
		Tile *tile2 = Map.getTile(tile->pos.x+1, tile->pos.y-3, tile->pos.z-1);
		addThingToTile(thing, tile2);
	}
	else if(tile && tile->canAddThing() == TILE_FLOOR_CHANGE_DOWN_SOUTH)
	{
		Tile *tile2 = Map.getTile(tile->pos.x+1, tile->pos.y+2, tile->pos.z-1);
		addThingToTile(thing, tile2);
	}
	else if(tile && tile->canAddThing() == TILE_FLOOR_CHANGE_DOWN_EAST)
	{
		Tile *tile2 = Map.getTile(tile->pos.x+3, tile->pos.y+1, tile->pos.z-1);
		addThingToTile(thing, tile2);
	}
	else if(tile && tile->canAddThing() == TILE_FLOOR_CHANGE_DOWN_WEST)
	{
		Tile *tile2 = Map.getTile(tile->pos.x-2, tile->pos.y+1, tile->pos.z-1);
		addThingToTile(thing, tile2);
	}
	else if(tile && tile->canAddThing() == TILE_FLOOR_CHANGE_NORTH)
	{
		Tile *tile2 = Map.getTile(tile->pos.x-1, tile->pos.y-2, tile->pos.z+1);
		addThingToTile(thing, tile2);
	}
	else if(tile && tile->canAddThing() == TILE_FLOOR_CHANGE_SOUTH)
	{
		Tile *tile2 = Map.getTile(tile->pos.x-1, tile->pos.y+1, tile->pos.z-1);
		addThingToTile(thing, tile2);
	}
	else if(tile && tile->canAddThing() == TILE_FLOOR_CHANGE_EAST)
	{
		Tile *tile2 = Map.getTile(tile->pos.x+2, tile->pos.y-1, tile->pos.z-1);
		addThingToTile(thing, tile2);
	}
	else if(tile && tile->canAddThing() == TILE_FLOOR_CHANGE_WEST)
	{
		Tile *tile2 = Map.getTile(tile->pos.x-1, tile->pos.y-1, tile->pos.z-1);
		addThingToTile(thing, tile2);
	}
}
void Game::transformThingFromTile(unsigned char stackpos, Thing *thing, Tile *tile)
{
    tile->transformThing(stackpos, thing);

	Item *item = dynamic_cast<Item*>(thing);
	if(item && tile)
	{
		unsigned int id = 105;
		sf::Packet toSend;
		toSend << id << tile->pos.x << tile->pos.y << tile->pos.z << stackpos << item->mID << item->count;
		this->sendPacketToPlayersInArea(toSend, tile->pos.x, tile->pos.y, tile->pos.z);
		//cout << "Sended transformThingFromTile, itemID: " << (dynamic_cast<Item*>(tile->getThing(stackpos)))->mID << " to: " << item->mID << endl;
	}
}
bool Game::removeThingFromTile(unsigned char stackpos, Tile *tile)
{
	unsigned int tempid = (dynamic_cast<Item*>(tile->getThing(stackpos)))->mID;
    if(tile->removeThing(stackpos))
    {
		unsigned int id = 103;
		sf::Packet toSend;
		toSend << id << tile->pos.x << tile->pos.y << tile->pos.z << stackpos;
		this->sendPacketToPlayersInArea(toSend, tile->pos.x, tile->pos.y, tile->pos.z);
		//cout << "Sended removeThingFromTile, itemID: " << tempid << endl;
		return true;
    }
    return false;
}
bool Game::removeTopThingFromTile(Tile *tile, unsigned short count)
{
    if(tile->removeTopItem(count))
	{
		unsigned int id = 109;
		sf::Packet toSend;
		toSend << id << tile->pos.x << tile->pos.y << tile->pos.z << count;
		this->sendPacketToPlayersInArea(toSend, tile->pos.x, tile->pos.y, tile->pos.z);
		return true;
    }
    return false;
}
Thing* Game::getThingFromTile(unsigned char stackpos, Tile *tile)
{
	return tile->getThing(stackpos);
}
unsigned char Game::getIndexOfThingFromTile(Thing* thing, Tile *tile)
{
	return tile->getIndexOfThing(thing);
}
unsigned char Game::getTileType(unsigned int x, unsigned int y, unsigned char z)
{
	Tile *tile = Map.getTile(x,y,z);
	if(tile)
		return tile->type;
	else
		return 0;
}

bool Game::moveThing(unsigned char stackpos, Tile *fromTile, Tile *toTile)
{
   Thing* sth = this->getThingFromTile(stackpos, fromTile);
   if(sth && fromTile && toTile)
   {
	   Item* item = dynamic_cast<Item*>(sth);
       if(item)
	   {
		  this->removeThingFromTile(stackpos, fromTile);
		  this->addThingToTile(item, toTile);
		  return true;
	   }
	   Creature* creature = dynamic_cast<Creature*>(sth);
	   if(creature)
	   {
		  this->removeThingFromTile(stackpos, fromTile);
		  this->addThingToTile(creature, toTile);
		  return true;
	   }
   }
   return false;
}

bool Game::moveCreature(Creature* creature, char movex, char movey, unsigned char *special)
{
	unsigned char direction = 0;

	if(movex == -1 && movey == 0)
	  direction = 4;
	if(movex == 1 && movey == 0)
	  direction = 6;
	if(movex == 0 && movey == -1)
	  direction = 8;
	if(movex == 0 && movey == 1)
	  direction = 2;
	// DIAGONAL MOVEMENT
	if(movex == -1 && movey == -1)
	  direction = 7;
	if(movex == 1 && movey == -1)
	  direction = 9;
	if(movex == -1 && movey == 1)
	  direction = 1;
	if(movex == 1 && movey == 1)
	  direction = 3;

	return this->moveCreature(creature, direction, special);
}

bool Game::moveCreature(Creature* creature, unsigned char direction, unsigned char *special)
{
	if(direction >= 0 && direction <= 9)
	{
		int diff_x = 0, diff_y = 0;
		if(direction == 4)
		  diff_x = -1;
		else if(direction == 6)
		  diff_x = 1;
		else if(direction == 8)
		  diff_y = -1;
		else if(direction == 2)
		  diff_y = 1;
		// DIAGONAL MOVEMENT
		else if(direction == 7)
		{
		  diff_x = -1;
		  diff_y = -1;
		}
		else if(direction == 9)
		{
		  diff_x = 1;
		  diff_y = -1;
		}
		else if(direction == 1)
		{
		  diff_x = -1;
		  diff_y = 1;
		}
		else if(direction == 3)
		{
		  diff_x = 1;
		  diff_y = 1;
		}
		Tile *tile;
		tile = Map.getTile(creature->getPosX() + diff_x, creature->getPosY() + diff_y, char(creature->getPosZ()));
		if(tile && !tile->isCollision())
		{
			creature->setStartedWalking();
			Player *player = dynamic_cast<Player*>(creature);
			
			if(player || tile->canAddThing() == 1 || tile->getTopItem()->mID == 32 || tile->getTopItem()->mID == 34)
			{
				tile = Map.getTile(creature->getPosX(), creature->getPosY(), char(creature->getPosZ()));
				tile->removeCreatureByID(creature->getId());
				creature->setDirection(direction);

				tile = Map.getTile(creature->getPosX() + diff_x, creature->getPosY() + diff_y, char(creature->getPosZ()));
			}

			if(special)
			{
				*special = static_cast<unsigned char>(tile->canAddThing());
				if(tile->getTopItem()->mID == 32 || tile->getTopItem()->mID == 34) //if there's doors, behaviour as normal tile
					*special = 1;
			}

			if(player && tile->canAddThing() == TILE_TELEPORT)
			{
				Item *teleport = tile->getTopItem();

				if(teleport && teleport->abilities)
				{
					if(teleport->abilities->destPosX != 0 || teleport->abilities->destPosY != 0 || teleport->abilities->destPosZ != 0)
					{
						Tile *destTile = Map.getTile(teleport->abilities->destPosX, teleport->abilities->destPosY, teleport->abilities->destPosZ);

						std::deque<Player*> playersInArea;
						CreaturesManager.getPlayersInArea(playersInArea, creature->getPosX(), creature->getPosY(), creature->getPosZ());

						creature->setPosX(teleport->abilities->destPosX);
						creature->setPosY(teleport->abilities->destPosY);
						creature->setPosZ(teleport->abilities->destPosZ);
						destTile->addThing(creature);

						for (std::deque<Player*>::iterator it = playersInArea.begin(); it < playersInArea.end(); it++)
							this->sendPlayerCreature((*it),creature, true);
						  /////////////////////////////////////////////////////////////////////////////////////////////////////////
						 // biorê graczy z miejsca przed teleportem potwora, wysy³am im nowe wspó³rzêdne (po teleporcie) potwora
						//  by clienty usunê³y stary obiekt potwora ;)
						this->createEffect(teleport->abilities->destPosX, teleport->abilities->destPosY, teleport->abilities->destPosZ, 4);
					}
				}
				else
				{
					creature->setPosX(creature->getPosX()+diff_x);
					creature->setPosY(creature->getPosY()+diff_y);
					tile->addThing(creature);
				}
			}
			else if(player && tile->canAddThing() == TILE_FLOOR_CHANGE_DOWN_NORTH)
			{
				Tile *tile2 = Map.getTile(tile->pos.x+1, tile->pos.y, tile->pos.z-1);
				creature->setPosX(creature->getPosX()+1);
				creature->setPosY(creature->getPosY());
				creature->setPosZ(creature->getPosZ()-1);
				creature->setDirection(8);
				tile2->addThing(creature);
			}
			else if(player && tile->canAddThing() == TILE_FLOOR_CHANGE_DOWN_SOUTH)
			{
				Tile *tile2 = Map.getTile(tile->pos.x+1, tile->pos.y+2, tile->pos.z-1);
				creature->setPosX(creature->getPosX()+1);
				creature->setPosY(creature->getPosY()+2);
				creature->setPosZ(creature->getPosZ()-1);
				creature->setDirection(2);
				tile2->addThing(creature);
				this->sendPlayerCreature(player, creature, true);
			}
			else if(player && tile->canAddThing() == TILE_FLOOR_CHANGE_DOWN_EAST)
			{
				Tile *tile2 = Map.getTile(tile->pos.x+2, tile->pos.y+1, tile->pos.z-1);
				creature->setPosX(creature->getPosX()+2);
				creature->setPosY(creature->getPosY()+1);
				creature->setPosZ(creature->getPosZ()-1);
				creature->setDirection(6);
				tile2->addThing(creature);
			}
			else if(player && tile->canAddThing() == TILE_FLOOR_CHANGE_DOWN_WEST)
			{
				Tile *tile2 = Map.getTile(tile->pos.x, tile->pos.y+1, tile->pos.z-1);
				creature->setPosX(creature->getPosX());
				creature->setPosY(creature->getPosY()+1);
				creature->setPosZ(creature->getPosZ()-1);
				creature->setDirection(4);
				tile2->addThing(creature);
			}
			else if(player && tile->canAddThing() == TILE_FLOOR_CHANGE_DOWN)
			{
				Tile *tile2 = Map.getTile(tile->pos.x+1, tile->pos.y+1, tile->pos.z-1);
				creature->setPosX(creature->getPosX()+1);
				creature->setPosY(creature->getPosY()+1);
				creature->setPosZ(creature->getPosZ()-1);
				creature->setDirection(2);
				tile2->addThing(creature);
			}
			else if(player && tile->canAddThing() == TILE_FLOOR_CHANGE_NORTH)
			{
				Tile *tile2 = Map.getTile(tile->pos.x-1, tile->pos.y-2, tile->pos.z+1);
				creature->setPosX(creature->getPosX()-1);
				creature->setPosY(creature->getPosY()-2);
				creature->setPosZ(creature->getPosZ()+1);
				creature->setDirection(8);
				tile2->addThing(creature);
			}
			else if(player && tile->canAddThing() == TILE_FLOOR_CHANGE_SOUTH)
			{
				Tile *tile2 = Map.getTile(tile->pos.x-1, tile->pos.y, tile->pos.z+1);
				creature->setPosX(creature->getPosX()-1);
				creature->setPosY(creature->getPosY());
				creature->setPosZ(creature->getPosZ()+1);
				creature->setDirection(2);
				tile2->addThing(creature);
			}
			else if(player && tile->canAddThing() == TILE_FLOOR_CHANGE_EAST)
			{
				Tile *tile2 = Map.getTile(tile->pos.x, tile->pos.y-1, tile->pos.z+1);
				creature->setPosX(creature->getPosX());
				creature->setPosY(creature->getPosY()-1);
				creature->setPosZ(creature->getPosZ()+1);
				creature->setDirection(6);
				tile2->addThing(creature);
			}
			else if(player && tile->canAddThing() == TILE_FLOOR_CHANGE_WEST)
			{
				Tile *tile2 = Map.getTile(tile->pos.x-2, tile->pos.y-1, tile->pos.z+1);
				creature->setPosX(creature->getPosX()-2);
				creature->setPosY(creature->getPosY()-1);
				creature->setPosZ(creature->getPosZ()+1);
				creature->setDirection(4);
				tile2->addThing(creature);
			}
			else if(player || tile->canAddThing() == 1 || tile->getTopItem()->mID == 32 || tile->getTopItem()->mID == 34)
			{
				creature->setPosX(creature->getPosX()+diff_x);
				creature->setPosY(creature->getPosY()+diff_y);
				tile = Map.getTile(creature->getPosX(), creature->getPosY(), char(creature->getPosZ()));
				tile->addThing(creature);  
			}
			else
				return false;

			std::deque<Player*> playersInArea;
			CreaturesManager.getPlayersInArea(playersInArea, creature->getPosX(), creature->getPosY(), creature->getPosZ());

			for (std::deque<Player*>::iterator it = playersInArea.begin(); it < playersInArea.end(); it++)
			{
				this->sendPlayerCreature((*it),creature, true);
			}

			return true;
		}
	}
	return false;
}

void Game::internalMovePlayer(Player *player, unsigned char direction)
{
	if(player && player->getStartedWalking() <= (clock() - player->getSpeed())) 
	{   
		unsigned char special = 1;
		if(this->moveCreature(player, direction, &special))
		{  
			unsigned char zz1, zz2;
			if(player->getPosZ() >= 7)
			{
				zz1 = 6;
				zz2 = 14;
			}
			else
			{
				zz1 = player->getPosZ() - 2;
				zz2 = 7;
			}
			if(special == 1) //normal player movement
			{
				if(direction == 4)
				{
					for(zz1; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
					{
						Tile *tile = Map.getTile(player->getPosX() - 9, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
				}
				if(direction == 6)
				{
					for(zz1; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
					{
						Tile *tile = Map.getTile(player->getPosX() + 10, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile); 
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
				}
				if(direction == 8)
				{
					for(zz1; zz1 <= zz2; zz1++)                  
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
					{
						Tile *tile = Map.getTile(x, player->getPosY() - 7, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile); 
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
				}
				if(direction == 2)
				{
					for(zz1; zz1 <= zz2; zz1++)                
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
					{
						Tile *tile = Map.getTile(x, player->getPosY() + 8, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);  
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
				}
				// DIAGONAL MOVEMENT
				if(direction == 7)
				{
					for(unsigned char zz1_t = zz1; zz1_t <= zz2; zz1_t++)
					{
						for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
						{
							Tile *tile = Map.getTile(x, player->getPosY() - 7, zz1_t);
							if(tile)
							{
								this->sendTileToPlayer(player, tile); 
								std::deque<Creature*> creaturesDeque = tile->getCreatures();
								for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
									this->sendPlayerCreature(player, (*it));
							}
						}
					}
					for(zz1; zz1 <= zz2; zz1++)
					{
						for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
						{
							Tile *tile = Map.getTile(player->getPosX() - 9, y, zz1);
							if(tile)
							{
								this->sendTileToPlayer(player, tile);
								std::deque<Creature*> creaturesDeque = tile->getCreatures();
								for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
									this->sendPlayerCreature(player, (*it));
							}
						}
					}
					player->setDirection(8);
				}
				if(direction == 9)
				{
					for(unsigned char zz1_t = zz1; zz1_t <= zz2; zz1_t++)
					{
						for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
						{
							Tile *tile = Map.getTile(x, player->getPosY() - 7, zz1_t);
							if(tile)
							{
								this->sendTileToPlayer(player, tile); 
								std::deque<Creature*> creaturesDeque = tile->getCreatures();
								for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
									this->sendPlayerCreature(player, (*it));
							}
						}
					}
					for(zz1; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
					{
						Tile *tile = Map.getTile(player->getPosX() + 10, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile); 
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
					player->setDirection(8);
				}
				if(direction == 1)
				{
					for(unsigned char zz1_t = zz1; zz1_t <= zz2; zz1_t++)
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
					{
						Tile *tile = Map.getTile(x, player->getPosY() + 8, zz1_t);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);  
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
					for(zz1; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
					{
						Tile *tile = Map.getTile(player->getPosX() - 9, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
					player->setDirection(2);
				}
				if(direction == 3)
				{
					for(unsigned char zz1_t = zz1; zz1_t <= zz2; zz1_t++)
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
					{
						Tile *tile = Map.getTile(x, player->getPosY() + 8, zz1_t);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);  
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
					for(zz1; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
					{
						Tile *tile = Map.getTile(player->getPosX() + 10, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile); 
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
					player->setDirection(2);
				}
			}
			else if(special == TILE_TELEPORT)
			{
				for(zz1; zz1 <= zz2; zz1++)
				for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
				for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
				{
					Tile *tile = Map.getTile(x, y, zz1);
					if(tile)
					{
						this->sendTileToPlayer(player, tile);
						std::deque<Creature*> creaturesDeque = tile->getCreatures();
						for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
							this->sendPlayerCreature(player, (*it), false);
					}
				}
			}
			else if(special == TILE_FLOOR_CHANGE_DOWN)
			{
				if(player->getPosZ() == 6)
				{
					for(zz1; zz1 <= player->getPosZ()-1; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
					{
						Tile *tile = Map.getTile(x, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it), false);
						}
					}

				}
				//else
				{
					unsigned char temp = zz1;
					for(zz1; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() + 7; y <= player->getPosY() + 9; y++)
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
					{
						Tile *tile = Map.getTile(x, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
					for(zz1 = temp; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 6; y++)
					for(unsigned int x = player->getPosX() + 9; x <= player->getPosX() +10; x++)
					{
						Tile *tile = Map.getTile(x, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
				}
			}
			else if(special == TILE_FLOOR_CHANGE_DOWN_SOUTH)
			{
				if(player->getPosZ() == 6)
				{
					for(zz1; zz1 <= player->getPosZ()-1; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
					{
						Tile *tile = Map.getTile(x, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it), false);
						}
					}
				}
				//else
				{
					unsigned char temp = zz1;
					for(zz1; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() + 7; y <= player->getPosY() + 9; y++)
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 11; x++)
					{
						Tile *tile = Map.getTile(x, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
					for(zz1 = temp; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 9; y++)
					for(unsigned int x = player->getPosX() + 9; x <= player->getPosX() +11; x++)
					{
						Tile *tile = Map.getTile(x, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
				}
			}
			else if(special == TILE_FLOOR_CHANGE_NORTH)
			{
				if(player->getPosZ() == 7)
				{
					for(zz1 = player->getPosZ()+1; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
					{
						Tile *tile = Map.getTile(x, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it), false);
						}
					}
					zz1 = 6;
				}
				//else
				{
					unsigned char temp = zz1;
					for(zz1; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() - 5; y++)
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
					{
						Tile *tile = Map.getTile(x, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
					for(zz1 = temp; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() - 4; y <= player->getPosY() + 8; y++)
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() -7; x++)
					{
						Tile *tile = Map.getTile(x, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
				}
			}
		}
	}
}

void Game::doPlayerChangeDirection(Player *player, unsigned char direction)
{
	if( player && direction != player->getDirection() )
	{
		player->setDirection(direction);
		
		sf::Packet toSend;
		int pid = 142;
		toSend << pid << player->getId() << direction;

		this->sendPacketToPlayersInArea(toSend, player->getPosX(), player->getPosY(), player->getPosZ());
	}
}

void Game::doTeleportPlayer(Player *player, unsigned int destPosX, unsigned int destPosY, unsigned char destPosZ)
{
	Tile *destTile = Map.getTile(destPosX, destPosY, destPosZ);
	if(player && destTile)
	{
		Tile *oldTile = Map.getTile(player->getPosX(), player->getPosY(), player->getPosZ());
		oldTile->removeCreatureByID(player->getId());
		this->sendTileToPlayer(player,oldTile);

		std::deque<Player*> playersInArea;
		CreaturesManager.getPlayersInArea(playersInArea, player->getPosX(), player->getPosY(), player->getPosZ());

		player->setPosX(destPosX);
		player->setPosY(destPosY);
		player->setPosZ(destPosZ);
		destTile->addThing(player);
		
		unsigned char zz1, zz2;
		if(player->getPosZ() >= 7)
		{
			zz1 = 6;
			zz2 = 14;
		}
		else
		{
			zz1 = player->getPosZ() - 2;
			zz2 = 7;
		}
		for(zz1; zz1 <= zz2; zz1++)
		for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
		for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
		{
			Tile *tile = Map.getTile(x, y, zz1);
			if(tile)
			{
				this->sendTileToPlayer(player, tile);
				std::deque<Creature*> creaturesDeque = tile->getCreatures();
				for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
					this->sendPlayerCreature(player, (*it), false);
			}
		}
		for (std::deque<Player*>::iterator it = playersInArea.begin(); it < playersInArea.end(); it++)
			this->sendPlayerCreature((*it),player, true);

		CreaturesManager.getPlayersInArea(playersInArea, destPosX, destPosY, destPosZ);
		for (std::deque<Player*>::iterator it = playersInArea.begin(); it < playersInArea.end(); it++)
			this->sendPlayerCreature((*it),player, true);

		this->createEffect(destPosX, destPosY, destPosZ, 4);
	}
}

void Game::doCreatureTemporaryChangeSpeed(Creature *creature, float factor, unsigned int duration)
{
	if(creature)
	{
		if(creature->speedConditionExpires != 0)
			creature->setSpeed(creature->baseSpeed);

		creature->baseSpeed = creature->getSpeed();
		creature->setSpeed(float(creature->getSpeed())*factor);
		creature->speedConditionExpires = clock() + duration;
		this->sendCreatureUpdateSpeed(creature);
	}
}

void Game::receiveThrowItem(Player *player, sf::Packet toReceive)
{
	if(player && player->canDoActionTime())
	{
		unsigned int itemId, parentId;
		unsigned short count;
		unsigned char fromz, toz;
		unsigned int fromx, fromy, tox, toy;
		Tile* totile;
		Tile* fromtile;
		toReceive >> itemId >> count >> parentId;
		if(parentId == 0) //je¿ei przesuwamy item z pola na mapie do innego pola, tutaj dodatkowe pola ze Ÿród³em itemu
		{
			toReceive >> fromx >> fromy >> fromz;
			fromtile = Map.getTile(fromx, fromy, fromz);
		}
		toReceive >> tox >> toy >> toz;
		totile = Map.getTile(tox, toy, toz);

		if(parentId == 20 && totile && this->canThrow(player, player->itemStorage->getItemId(itemId, count), player->getPosX(), player->getPosY(), player->getPosZ(), tox, toy, toz))
		{
			this->addThingToTile(player->itemStorage->getItemId(itemId, count),totile);
			this->removeItemIdFromPlayerStorage(player, itemId, count);
		}
		if(parentId == 0 && fromtile && totile && this->canThrow(player, fromtile->getTopItem(), fromx, fromy, fromz, tox, toy, toz))
		{
			this->addThingToTile(fromtile->getTopItem(),totile);
			this->removeTopThingFromTile(fromtile);
			//this->moveThing(fromtile->getIndexOfThing(fromtile->getTopItem()),fromtile,totile);
		}
		player->setActionTime();
	}
}

void Game::receiveEquipItem(Player *player, sf::Packet toReceive)
{
	if(player && player->canDoActionTime())
	{
		unsigned char index;
		int parentId;
		bool containerFromMap;
		unsigned int bpid;
		unsigned char fromz;
		unsigned int fromx, fromy;
		unsigned char containerIndex;
		toReceive >> containerFromMap >> index >> parentId >> bpid >> containerIndex;
		if(parentId == 0) //je¿eli przesuwamy item z pola na mapie do plecaka, tutaj dodatkowe pola ze Ÿród³em itemu
		{
			toReceive >> fromx >> fromy >> fromz;
			Tile* tile = Map.getTile(fromx, fromy, fromz);

			if(tile && tile->getTopItem() && (player->itemStorage->getItemHoldingCount() < player->itemStorage->capacity()))
			{
				Container *container = dynamic_cast<Container*>(tile->getThing(index));

				if(container == NULL && BaseItems.getItemType(tile->getTopItem()->mID)->pickupable
				&& this->canThrow(player, tile->getTopItem(), fromx, fromy, fromz, player->getPosX(), player->getPosY(), player->getPosZ(), true))
				{
					Item *itemek = tile->getTopItem();
					if(bpid > 0 && bpid <= 15 && canEquipItem(player, itemek, bpid))
					{
						if(this->equipItem(player, bpid, itemek))
							this->removeTopThingFromTile(tile);
					}
					else
					{
						if(this->addItemToPlayerStorage(player, itemek, 20))
							this->removeTopThingFromTile(tile); //dodaj branie odpowiedniej iloœci danego przedmiotu
					}
				}
				if(container && containerFromMap && this->canThrow(player, container->getItem(containerIndex), fromx, fromy, fromz, player->getPosX(), player->getPosY(), player->getPosZ(), true))
				{
					Item *item = container->getItem(containerIndex);
					if(item)
					{
						if(bpid > 0 && bpid <= 15 && canEquipItem(player, item, bpid))
						{
							if(this->equipItem(player, bpid, item))
								this->removeItemFromContainer(container, item);
						}
						else
						{
							if(this->addItemToPlayerStorage(player, item, 20))
								this->removeItemFromContainer(container, item);
						}
					}
				}
			}
		}
		if(parentId == 20 && player->itemStorage->getItem(index)) //zamiana miejscami w plecaku lub equip z plecaka do equ
		{
			Item *itemek = player->itemStorage->getItem(index);
			if(bpid > 0 && bpid <= 15 && canEquipItem(player, itemek, bpid))
			{
				if(this->equipItem(player, bpid, itemek))
					this->removeItemFromPlayerStorage(player, index, 20);
			}
			else if(bpid == 0 || bpid == 20)
			{
				if(!itemek->baseItem->stackable)
				{
					if(this->addItemToPlayerStorage(player, itemek, 20))
						this->removeItemFromPlayerStorage(player, index, 20);
				}
				else ////////// POPRAW KIEDYŒ BO MO¯E BYÆ ABUSE!
				{
					this->removeItemFromPlayerStorage(player, index, 20);
					this->addItemToPlayerStorage(player, itemek, 20);
				}
			}
		}
		else if(parentId > 0 && parentId <= 15)
		{
			Item *itemek = NULL;

			if(parentId == 1 && player->equipment.lRingEar)
			{
				itemek = player->equipment.lRingEar;
			}
			else if(parentId == 2 && player->equipment.head) 
			{
				itemek = player->equipment.head;
			}
			else if(parentId == 3 && player->equipment.rRingEar)  
			{
				itemek = player->equipment.rRingEar;
			}
			else if(parentId == 4 && player->equipment.necklace)    
			{
				itemek = player->equipment.necklace;
			}
			else if(parentId == 5 && player->equipment.armor) 
			{
				itemek = player->equipment.armor;
			}
			else if(parentId == 6 && player->equipment.backpack)
			{
				itemek = player->equipment.backpack;
			}
			else if(parentId == 7 && player->equipment.weapon) 
			{
				itemek = player->equipment.weapon;
			}
			else if(parentId == 8 && player->equipment.belt)   
			{
				itemek = player->equipment.belt;
			}
			else if(parentId == 9 && player->equipment.shield) 
			{
				itemek = player->equipment.shield;
			}
			else if(parentId == 10 && player->equipment.lRing)    
			{
				itemek = player->equipment.lRing;
			}
			else if(parentId == 11 && player->equipment.legs)
			{
				itemek = player->equipment.legs;
			}
			else if(parentId == 12 && player->equipment.rRing)   
			{
				itemek = player->equipment.rRing;
			}
			else if(parentId == 13 && player->equipment.gloves)   
			{
				itemek = player->equipment.gloves;
			}
			else if(parentId == 14 && player->equipment.boots)
			{
				itemek = player->equipment.boots;
			}
			else if(parentId == 15 && player->equipment.arrows)  
			{
				itemek = player->equipment.arrows;
			}
			if(bpid == 20 && itemek)
			{
				if(this->addItemToPlayerStorage(player, itemek, 20))
					removeItemFromPlayerEquipment(player, parentId, 0);
			}
		}
		player->setActionTime();
	}
}

void Game::receiveRequestItemStats(Player *player, sf::Packet toReceive)
{
	if(player && player->canDoActionTime())
	{
		unsigned int bpid;
		unsigned char fromz, index;
		unsigned int fromx, fromy;
		toReceive >> bpid >> fromx >> fromy >> fromz >> index;
		Item *itemek = NULL;

		if(bpid == 0) //je¿eli sprawdzamy item na mapie, tutaj dodatkowe pola ze Ÿród³em itemu
		{
			Tile* tile = Map.getTile(fromx, fromy, fromz);

			if(tile && tile->getTopItem())
			{
				Container *container = dynamic_cast<Container*>(tile->getThing(index));

				if(container == NULL)
					itemek = tile->getTopItem();
				if(container)
					itemek = container->getItem(index);
			}
		}
		else if(bpid > 0 && bpid <= 15) //sprawdzenie w ekwipunku
		{
			if(bpid == 1 && player->equipment.lRingEar)
				itemek = player->equipment.lRingEar;
			else if(bpid == 2 && player->equipment.head) 
				itemek = player->equipment.head;
			else if(bpid == 3 && player->equipment.rRingEar)  
				itemek = player->equipment.rRingEar;
			else if(bpid == 4 && player->equipment.necklace)    
				itemek = player->equipment.necklace;
			else if(bpid == 5 && player->equipment.armor) 
				itemek = player->equipment.armor;
			else if(bpid == 6 && player->equipment.backpack)
				itemek = player->equipment.backpack;
			else if(bpid == 7 && player->equipment.weapon) 
				itemek = player->equipment.weapon;
			else if(bpid == 8 && player->equipment.belt)   
				itemek = player->equipment.belt;
			else if(bpid == 9 && player->equipment.shield) 
				itemek = player->equipment.shield;
			else if(bpid == 10 && player->equipment.lRing)    
				itemek = player->equipment.lRing;
			else if(bpid == 11 && player->equipment.legs)
				itemek = player->equipment.legs;
			else if(bpid == 12 && player->equipment.rRing)   
				itemek = player->equipment.rRing;
			else if(bpid == 13 && player->equipment.gloves)
				itemek = player->equipment.gloves;
			else if(bpid == 14 && player->equipment.boots)
				itemek = player->equipment.boots;
			else if(bpid == 15 && player->equipment.arrows)  
				itemek = player->equipment.arrows;
		}
		if(bpid == 20 && player->itemStorage->getItem(index)) //sprawdzenie w plecaku gracza
		{
			itemek = player->itemStorage->getItem(index);
		}
		if(itemek) //tutaj wype³nianie pakietu ze statystykami przedmiotu i wysy³anie go
		{
			/*
			sf::Packet toSend;
			int sendId = 175;
			toSend << sendId;
			toSend << itemek->baseItem->name << itemek->baseItem->pluralName << itemek->baseItem->armor << itemek->baseItem->attack << itemek->baseItem->defence;
			toSend << itemek->baseItem->description;
			toSend << itemek->baseItem->minReqDex << itemek->baseItem->minReqInt << itemek->baseItem->minReqMPW << itemek->baseItem->minReqStr;
			if(itemek->abilities)
			{
				toSend << bool(1);
				toSend << itemek->abilities->addHealth << itemek->abilities->addMana << itemek->abilities->addSkillDex << itemek->abilities->addSkillInt;
				toSend << itemek->abilities->addSkillMPW << itemek->abilities->addSkillStr << itemek->abilities->addSpeed << itemek->abilities->elementDamage;
				toSend << int(itemek->abilities->elementType) << itemek->abilities->upgradeLevel << itemek->abilities->text;
				toSend << itemek->abilities->resistance[0] << itemek->abilities->resistance[1] << itemek->abilities->resistance[2];
				toSend << itemek->abilities->resistance[3] << itemek->abilities->resistance[4] << itemek->abilities->resistance[5];
				toSend << itemek->abilities->resistance[6] << itemek->abilities->resistance[7] << itemek->abilities->resistance[8];
				toSend << itemek->abilities->resistance[9];
			}
			else
			{
				toSend << bool(0);
			}

			player->socket->send(toSend);
			*/
			string message = "You're looking at ";
			if(itemek->count > 1)
			{
				message += intToStr(itemek->count);
				message += " ";
				message += itemek->baseItem->pluralName;
			}
			else
				message += itemek->baseItem->name;

			message += ".";
			if(itemek->baseItem->attack != 0)
			{
				message += " Atk: ";
				message += intToStr(itemek->baseItem->attack);
			}
			if(itemek->baseItem->defence != 0)
			{
				message += " Def: ";
				message += intToStr(itemek->baseItem->defence);
			}
			if(itemek->baseItem->armor != 0)
			{
				message += " Arm: ";
				message += intToStr(itemek->baseItem->armor);
			}
			if(itemek->baseItem->shootRange > 1)
			{
				message += " Range: ";
				message += intToStr(itemek->baseItem->shootRange);
			}
			if(itemek->baseItem->id != 0)
			{
				message += " Id: ";
				message += intToStr(itemek->baseItem->id);
			}
			this->sendMessageToPlayer(player, 0, "", 0, message);
		}
		player->setActionTime();
	}
}

void Game::receiveUseItem(Player *player, sf::Packet toReceive)
{
	if(player && player->canDoActionTime())
	{
		unsigned int bpid;
		unsigned char fromz, toz;
		unsigned int fromx, fromy, tox, toy;
		toReceive >> bpid >> fromx >> fromy >> fromz >> tox >> toy >>toz;
		if(bpid == 0 && this->isNearPos(player, fromx, fromy, fromz)) //je¿eli u¿ywamy item z mapy
		{
			Tile *fromTile = Map.getTile(fromx, fromy, fromz);
			if(fromTile)
			{
				Item *item = fromTile->getTopItem();
				if(BaseItems.getItemType(item->mID)->useable) //item pozwala wybraæ miejsce na którym ma byæ u¿yty
				{
					//////////////////////////////////////////////////////////
				}
				else if(item->mID == 40) //drabina
				{
					Tile *tile;
					tile = Map.getTile(dynamic_cast<Tile*>(item->parent)->pos.x - 1, dynamic_cast<Tile*>(item->parent)->pos.y - 2, dynamic_cast<Tile*>(item->parent)->pos.z + 1);
					if(tile)
					{
						player->setStartedWalking();
						tile = Map.getTile(player->getPosX(), player->getPosY(), char(player->getPosZ()));
						tile->removeCreatureByID(player->getId());
						this->sendTileToPlayer(player,tile);
						player->setDirection(8);
						tile = Map.getTile(dynamic_cast<Tile*>(item->parent)->pos.x - 1, dynamic_cast<Tile*>(item->parent)->pos.y - 2, dynamic_cast<Tile*>(item->parent)->pos.z + 1);
						player->setPos(tile->pos.x, tile->pos.y, tile->pos.z);
						//this->sendPlayerCreature(player, player, false);
					}
					unsigned char zz1, zz2;
					if(player->getPosZ() >= 7)
					{
						zz1 = 7;
						zz2 = 14;
					}
					else
					{
						zz1 = player->getPosZ() - 2;
						zz2 = 6;
					}
					for(zz1; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
					{
						Tile *tile = Map.getTile(x, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								if((*it)->getId() != player->getId())
									this->sendPlayerCreature(player, (*it));
						}
					}
					this->sendPlayerCreature(player, player, false);////////////////////
				}
				else if(item->mID == 279) //zejœcie do œcieków
				{
					Tile *tile;
					tile = Map.getTile(dynamic_cast<Tile*>(item->parent)->pos.x + 1, dynamic_cast<Tile*>(item->parent)->pos.y + 1, dynamic_cast<Tile*>(item->parent)->pos.z - 1);
					if(tile)
					{
						player->setStartedWalking();
						tile = Map.getTile(player->getPosX(), player->getPosY(), char(player->getPosZ()));
						tile->removeCreatureByID(player->getId());
						this->sendTileToPlayer(player,tile);
						player->setDirection(2);
						tile = Map.getTile(dynamic_cast<Tile*>(item->parent)->pos.x + 1, dynamic_cast<Tile*>(item->parent)->pos.y + 1, dynamic_cast<Tile*>(item->parent)->pos.z - 1);
						player->setPos(dynamic_cast<Tile*>(item->parent)->pos.x + 1, dynamic_cast<Tile*>(item->parent)->pos.y + 1, dynamic_cast<Tile*>(item->parent)->pos.z - 1);
					}
					unsigned char zz1, zz2;
					if(player->getPosZ() >= 7)
					{
						zz1 = 7;
						zz2 = 14;
					}
					else
					{
						zz1 = player->getPosZ() - 2;
						zz2 = 6;
					}
					for(zz1; zz1 <= zz2; zz1++)
					for(unsigned int y = player->getPosY() - 7; y <= player->getPosY() + 8; y++)
					for(unsigned int x = player->getPosX() - 9; x <= player->getPosX() + 10; x++)
					{
						Tile *tile = Map.getTile(x, y, zz1);
						if(tile)
						{
							this->sendTileToPlayer(player, tile);
							std::deque<Creature*> creaturesDeque = tile->getCreatures();
							for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
								this->sendPlayerCreature(player, (*it));
						}
					}
					this->sendPlayerCreature(player, player, false);////////////////////
				}
				else if(item->mID == 31 || item->mID == 33) //transformacja drzwi zamkniête->otwarte
				{
					this->transformThingFromTile(fromTile->getIndexOfThing(item), new Item(item->mID+1), fromTile);
				}
				else if(item->mID == 32 || item->mID == 34) //transformacja drzwi otwarte->zamkniête
				{
					std::deque<Creature*> creaturesDeque = fromTile->getCreatures();
					if(!creaturesDeque.empty())
						for(std::deque<Creature*>::iterator it = creaturesDeque.begin(); it != creaturesDeque.end(); it++)
						{
							Player *player = dynamic_cast<Player*>((*it));
							if(item->mID == 32)
							{
								if(player)
									this->internalMovePlayer(player,2);
								else
									this->moveCreature((*it),2);
							}
							else if(item->mID == 33)
							{
								if(player)
									this->internalMovePlayer(player,6);
								else
									this->moveCreature((*it),6);
							}
						}
					this->transformThingFromTile(fromTile->getIndexOfThing(item), new Item(item->mID-1), fromTile);
				}
				else if(item->mID == 283 || item->mID == 284 || item->mID == 285 || item->mID == 286 || item->mID == 453) //usuniêcie itemu miêsa i wêgorza z pod³ogi
				{
					if(player->getHungryTicks() < 300)
					{
						if(item->count > 1)
							this->transformThingFromTile(fromTile->getIndexOfThing(item), new Item(item->mID, item->count - 1), fromTile);
						else
							this->removeTopThingFromTile(fromTile);

						player->addHungryTicks(60);
						this->sendMessageToPlayer(player, 0, "", 0, "That was delicous...");
					}
					else
						this->sendMessageToPlayer(player, 0, "", 0, "You shouldn't eat more for a while...");
				}
				else if(item->mID == 288) //switch lewo->prawo
				{
					if(item->abilities && item->abilities->uniqueid == 101) //switch w podziemiach zwi¹zanych z "piek³em"
					{
						Tile *tile = Map.getTile(52, 140, 4);
						if(tile && tile->getTopItem() && tile->getTopItem()->mID  != 152)
						{
							Item *portal = new Item(152,1);
							portal->abilities = new ItemAbilities();
							portal->abilities->destPosX = 76;
							portal->abilities->destPosY = 144;
							portal->abilities->destPosZ = 6;
							this->addThingToTile(portal, tile);
							this->sendMessageToPlayer(player, 0, "", 0, "Hmm... Strange portal appears, huh?");
						}
						else
							this->sendMessageToPlayer(player, 0, "", 0, "You cannot vanish this portal...?");
					}
					else if(item->abilities && item->abilities->uniqueid == 102) //switch w piekle, na dole z Angry Saint, odblokowywuj¹cy kamieñ na drodze
					{
						for(int ix = 0; ix <= 1; ix++)
						for(int iy = 0; iy <= 1; iy++)
						{
							Tile *tile = Map.getTile(114+ix, 128+iy, 5);
							if(tile && tile->getTopItem() && tile->getTopItem()->mID >= 393 && tile->getTopItem()->mID <= 396)
								this->removeTopThingFromTile(tile);
						}
						delete item->abilities;
						this->sendMessageToPlayer(player, 0, "", 0, "This strange feeling... Sounds like big rock being crushed.");
					}
					Item *newItem = new Item(item->mID+1);
					if(item->abilities)
						newItem->abilities = item->abilities;
					this->transformThingFromTile(fromTile->getIndexOfThing(item), newItem, fromTile);
				}
				else if(item->mID == 289) //switch prawo->lewo
				{
					Item *newItem = new Item(item->mID-1);
					if(item->abilities)
						newItem->abilities = item->abilities;
					this->transformThingFromTile(fromTile->getIndexOfThing(item), newItem, fromTile);
				}
				else if(item->mID == 37 || item->mID == 298 || item->mID == 300) //w³¹czenie lampy ulicznej i œwiec na œcianach
				{
					this->transformThingFromTile(fromTile->getIndexOfThing(item), new Item(item->mID+1), fromTile);
				}
				else if(item->mID == 38 || item->mID == 299 || item->mID == 301) //wy³¹czenie lampy ulicznej i œwiec na œcianach
				{
					this->transformThingFromTile(fromTile->getIndexOfThing(item), new Item(item->mID-1), fromTile);
				}
				else if(item->mID == 46) //skrzynka questowa id=46
				{
					if(item->abilities && item->abilities->uniqueid == 100) //skrzynka w podziemiach na lewo od piek³a, z black dragonami
					{
						if(player->getFlag(100) == -1)
							if(this->addItemToPlayerStorage(player, new Item(422,1), 20))
							{
								player->setFlag(100, 1);
								this->sendMessageToPlayer(player, 0, "", 0, "You collected 'Solid Shell Armor'.");
							}
							else
								this->sendMessageToPlayer(player, 0, "", 0, "You don't have enough capacity.");
						else
							this->sendMessageToPlayer(player, 0, "", 0, "You allready opened this box.");
					}
				}
			}
		}
		if(bpid == 20) //klikniêcie prawym klawiszem myszki na item w plecaku
		{
			Item *tempItem = player->itemStorage->getItem(char(fromx));
			if(tempItem && ( tempItem->mID == 283 || tempItem->mID == 284 || tempItem->mID == 285 || tempItem->mID == 286 || tempItem->mID == 453) ) 
			{
				if(player->getHungryTicks() < 300)
				{
					if(tempItem->count > 1)
						this->removeItemIdFromPlayerStorage(player, tempItem->mID, 1);
					else
						this->removeItemFromPlayerStorage(player, char(fromx), 20);
					
					player->addHungryTicks(60);
					this->sendMessageToPlayer(player, 0, "", 0, "That was delicous...");
				}
				else
					this->sendMessageToPlayer(player, 0, "", 0, "You shouldn't eat more for a while...");
			}
			if (tempItem && (tempItem->mID == 357 || tempItem->mID == 358 || tempItem->mID == 359 || tempItem->mID == 360 || tempItem->mID == 361 || tempItem->mID == 362) )
			{
				if(tempItem->count > 1)
					this->removeItemIdFromPlayerStorage(player, tempItem->mID, 1);
				else
					this->removeItemFromPlayerStorage(player, char(fromx), 20);

				unsigned int addCount = 0;

				if(tempItem->mID == 357) // big health potion
				{
					addCount = rand()%200 + 401;
					player->addHealth(addCount);
					this->sendCreatureUpdateHealth(player);
				}
				else if(tempItem->mID == 358) // big mana potion
				{
					addCount = rand()%200 + 401;
					player->addMana(addCount);
					this->sendPlayerUpdateMana(player);
				}
				else if(tempItem->mID == 359) // medium health potion
				{
					addCount = rand()%100 + 201;
					player->addHealth(addCount);
					this->sendCreatureUpdateHealth(player);
				}
				else if(tempItem->mID == 360) // medium mana potion
				{
					addCount = rand()%100 + 201;
					player->addMana(addCount);
					this->sendPlayerUpdateMana(player);
				}
				else if(tempItem->mID == 361) // small health potion
				{
					addCount = rand()%50 + 51;
					player->addHealth(addCount);
					this->sendCreatureUpdateHealth(player);
				}
				else if(tempItem->mID == 362) //small mana potion
				{
					addCount = rand()%50 + 51;
					player->addMana(addCount);
					this->sendPlayerUpdateMana(player);
				}
				this->createEffect(player->getPosX(), player->getPosY(), player->getPosZ(), 6); //energy storm effect
				this->createTextEffect(player->getPosX(), player->getPosY(), player->getPosZ(), 1, intToStr(addCount));
			}
		}
		player->setActionTime();
	}
}

void Game::receivePlayerAttackCreature(Player *player, sf::Packet toReceive)
{
	if(player && player->canDoActionTime())
	{
		unsigned int cid;
		bool cancel = false;
		toReceive >> cid >> cancel;

		Creature *creature = CreaturesManager.getCreature(cid);
		if(creature && player->getId() != creature->getId()) 
		{
			if((creature->getPosX() >= player->getPosX() -8) && (creature->getPosX() <= player->getPosX() +8) 
			&& (creature->getPosY() >= player->getPosY() -6) && (creature->getPosY() <= player->getPosY() +6) 
			&& (creature->getPosZ() == player->getPosZ()))
			{
				NPC *npc = dynamic_cast<NPC*>(creature);
				if(npc)
				{
					for(deque<NPC>::iterator it = CreaturesManager.NPCList.begin(); it < CreaturesManager.NPCList.end(); it++)
						it->playerDisappear(player->getId());

					if(this->isInRange(player, npc, 2))
					{
						player->npcTalked = npc;
						sf::Packet packet = npc->playerChoose(player->getId(), 0);
						player->socket->send(packet);
						this->sendPlayerNPCshop(player, npc);
					}
					else
						this->sendMessageToPlayer(player, 0, "", 0, "You should stand closer to your talking partner");
				}
				else if( dynamic_cast<Player*>(creature) || (dynamic_cast<Monster*>(creature) && !(dynamic_cast<Monster*>(creature)->death)) )
				{
					if(cancel) //anulowanie ataku
					{
						player->setTarget(NULL);
						this->sendPlayerTargetConfirmation(player, cid, true);
					}
					else if( (dynamic_cast<Player*>(creature) && ConfigManager.pvp == true) || dynamic_cast<Monster*>(creature) )//zaatakowanie
					{
						player->setTarget(creature);
						this->sendPlayerTargetConfirmation(player, cid, false);

						if( player->equipment.weapon && player->equipment.weapon->baseItem->weaponType != WEAPON_DIST && this->isNearPos(creature, player->getPosX(), player->getPosY(), player->getPosZ()) && 
							player->getLastMeleeAttack() + 1500 < clock() )
						{
							player->setLastMeleeAttack();
							this->attackCreature(creature, player, COMBAT_PHYSICALDAMAGE);
						}
						else if(player->equipment.weapon && player->equipment.weapon->baseItem->weaponType == WEAPON_DIST && player->getLastMeleeAttack() + 1500 < clock())
						{
							if(player->equipment.arrows != NULL && player->equipment.arrows->baseItem->ammoType == player->equipment.weapon->baseItem->ammoType 
							&& player->equipment.arrows->count >= 1 
							&& isInRange(player->getPosX(), player->getPosY(), player->getPosZ(), creature->getPosX(), creature->getPosY(), creature->getPosZ(), player->equipment.weapon->baseItem->shootRange) 
							&& canSeeThroughTiles(player->getPosX(), player->getPosY(), creature->getPosX(), creature->getPosY(), creature->getPosZ()))
							{
								unsigned char shootEffect;
								if(player->equipment.weapon->baseItem->ammoType == AMMO_ARROW)
									shootEffect = 1;
								else if(player->equipment.weapon->baseItem->ammoType == AMMO_BOLT)
									shootEffect = 2;
								else
									shootEffect = 0;

								this->removeItemFromPlayerEquipment(player, 15, 1);
								player->setLastMeleeAttack();
								this->createShootEffect(player->getPosX(), player->getPosY(), player->getPosZ(), creature->getPosX(), creature->getPosY(), creature->getPosZ(), shootEffect);
								this->attackCreature(creature, player, COMBAT_PHYSICALDAMAGE);
							}	
						}
					}
					else
						this->sendPlayerTargetConfirmation(player, cid, true);
					player->setActionTime();
				}
			}
		}
		if(ConfigManager.pvp == false && dynamic_cast<Player*>(creature))
		{
			player->setTarget(NULL);
			this->sendPlayerTargetConfirmation(player, cid, true);
		}
	}
}

int getPlayerFlag(Player *player, unsigned int _key)
{
	if(player)
		return player->getFlag(_key);
	return -1;
}

bool setPlayerFlag(Player *player, unsigned int _key, int _value)
{
	if(player)
		return player->setFlag(_key, _value);
	return false;
}

bool Game::canEquipItem(Player *player, Item *item, unsigned short slot)
{
	unsigned short slotPos = 0;

	if(slot == 1)
		slotPos = 1;
	else if(slot == 2)
		slotPos = 2;
	else if(slot == 3)
		slotPos = 4;
	else if(slot == 4)
		slotPos = 8;
	else if(slot == 5)
		slotPos = 16;
	else if(slot == 6)
		slotPos = 32;
	else if(slot == 7)
		slotPos = 64;
	else if(slot == 8)
		slotPos = 128;
	else if(slot == 9)
		slotPos = 256;
	else if(slot == 10)
		slotPos = 512;
	else if(slot == 11)
		slotPos = 1024;
	else if(slot == 12)
		slotPos = 2048;
	else if(slot == 13)
		slotPos = 4096;
	else if(slot == 14)
		slotPos = 8192;
	else if(slot == 15)
		slotPos = 16384;

	if(player && ((slotPos == item->baseItem->slotPosition) || (slotPos == 64 && item->baseItem->slotPosition == 32768 && player->equipment.shield == NULL) ))
	{
		if(slotPos == 256 && player->equipment.weapon && player->equipment.weapon->baseItem->slotPosition == 32768)
		{
			this->sendMessageToPlayer(player, 0, "", 0, "You can't hold shield while holding two-handed weapon.");
			return false;
		}
		if(item->baseItem->vocation == VOCATION_NONE || item->baseItem->vocation == player->getVocation())
		{
			if(item->baseItem->minReqDex < player->getDexterity() &&
				item->baseItem->minReqStr < player->getStrength() &&
				item->baseItem->minReqInt < player->getIntelligence() &&
				item->baseItem->minReqMPW < player->getMagicPower())
				return true;
		}
	}
	else if(player && slotPos == 64 && item->baseItem->slotPosition == 32768 && player->equipment.shield != NULL)
	{
		this->sendMessageToPlayer(player, 0, "", 0, "You can't hold two-handed weapon while holding shield.");
	}
	return false;
}

bool Game::canThrow(Player *player, Thing *thing, unsigned int fromx, unsigned int fromy, unsigned char fromz, unsigned int tox, unsigned int toy, unsigned char toz, bool playerEquip)
{
	Item *item = dynamic_cast<Item*>(thing);
	if(player && item && BaseItems.getItemType(item->mID)->moveable)
	{
		if((fromx >= player->getPosX() -1) && (fromx <= player->getPosX() +1) 
		&& (fromy >= player->getPosY() -1) && (fromy <= player->getPosY() +1) 
		&& (fromz == player->getPosZ()))
		{
			if((tox >= player->getPosX() -7) && (tox <= player->getPosX() +7) 
			&& (toy >= player->getPosY() -5) && (toy <= player->getPosY() +5) 
			&& (toz == player->getPosZ()))//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			{
				Tile *tile = Map.getTile(tox, toy, toz);
				if(tile->canAddThing() > 0 || playerEquip)
					return true;
			}
		}
	}
	return false;
}

bool Game::canSeeThroughTiles(unsigned int fromx, unsigned int fromy, unsigned int tox, unsigned int toy, unsigned char z)
//(int x0, int y0, int x1, int y1)
{
    int dx, dy;

	if(fromx > tox)
		dx = fromx - tox;
	else
		dx = tox - fromx;

	if(fromy > toy)
		dy = fromy - toy;
	else
		dy = toy - fromy;

	if(dx <= 1 && dy <= 1)
		return true;

    int x = fromx;
    int y = fromy;
    int n = 1 + dx + dy;
    int x_inc = (tox > fromx) ? 1 : -1;
    int y_inc = (toy > fromy) ? 1 : -1;
    int error = dx - dy;
    dx *= 2;
    dy *= 2;
    for (; n > 0; --n)
    {
        //if(currentLevel.getTile(int((x+tileSize/2)/tileSize), int((y+tileSize/2)/tileSize)) == 1) 
		if(Map.getTile(x, y, z) && !Map.getTile(x, y, z)->isProjectile())
			return false; 
        if (error > 0)
        {
            x += x_inc;
            error -= dy;
        }
        else
        {
            y += y_inc;
            error += dx;
        }
    }
    return true;
}

bool Game::canDoCombat(Creature *creature)
{
	if(creature)
	{
		Tile *tile = Map.getTile(creature->getPosX(), creature->getPosY(), creature->getPosZ());
		if(tile && tile->type == 0)
			return true;
	}
	return false;
}

bool Game::canDoCombat(unsigned int x, unsigned int y, unsigned char z)
{
	Tile *tile = Map.getTile(x, y, z);
	if(tile && tile->type == 0)
		return true;
	return false;
}

bool Game::isNearPos(Creature *creature, unsigned int x, unsigned int y, unsigned char z)
{
	if(creature && (x >= creature->getPosX() -1) && (x <= creature->getPosX() +1) 
	&& (y >= creature->getPosY() -1) && (y <= creature->getPosY() +1) 
	&& (z == creature->getPosZ()))
	{
		return true;
	}
	return false;
}

bool Game::isInRange(Creature *creature, Creature* creature2, unsigned char range)
{
	if(creature && creature2)
		return this->isInRange(creature->getPosX(), creature->getPosY(), creature->getPosZ(), creature2->getPosX(), creature2->getPosY(), creature2->getPosZ(), range);
	return false;
}

bool Game::isInRange(Creature *creature, unsigned int posx, unsigned int posy, unsigned char posz, unsigned char range)
{
	if(creature)
		return this->isInRange(creature->getPosX(), creature->getPosY(), creature->getPosZ(), posx, posy, posz, range);
	return false;
}

bool Game::isInRange(unsigned int fromx, unsigned int fromy, unsigned char fromz, unsigned int tox, unsigned int toy, unsigned char toz, unsigned char range)
{
	if(fromz != toz)
		return false;

	unsigned char diffx = 0;
	unsigned char diffy = 0;

	if(fromx > tox)
		diffx = fromx - tox;
	else
		diffx = tox - fromx;

	if(fromy > toy)
		diffy = fromy - toy;
	else
		diffy = toy - fromy;

	unsigned char rangeToCompare = diffx*diffx + diffy*diffy;
	unsigned char distance = static_cast<unsigned char>(sqrt(float(rangeToCompare)));

	if(distance <= range)
		return true;
	else
		return false;
}

void Game::sendPacketToPlayersInArea(sf::Packet packet, unsigned int x, unsigned int y, unsigned char z)
{
	deque<Player*> spectators;
	CreaturesManager.getPlayersInArea(spectators, x, y, z);


	for(deque<Player*>::iterator it = spectators.begin(); it != spectators.end(); it++)
	{
		if((*it)->socket)
		{
			(*it)->socket->send(packet);
		}  
	}
}

bool Game::sendMessageToPlayer(unsigned int id, unsigned char type, string sender, unsigned int senderLevel, string message)
{
	Player *player = this->getPlayerByID(id);
	return this->sendMessageToPlayer(player, type, sender, senderLevel, message);
}

bool Game::sendMessageToPlayer(Player *player, unsigned char type, string sender, unsigned int senderLevel, string message, string from)
{
	if(player)
	{
		int packetId = 200;
		sf::Packet toSend;
		toSend << packetId << type << sender << senderLevel << message;
		if(type == 5)
			toSend << from;
		player->socket->send(toSend);
		return true;
	}
	return false;
}

bool Game::sendMessageToAllPlayers(unsigned char type, string sender, unsigned int senderLevel, string message)
{
	for (deque<Player>::iterator it = CreaturesManager.playersList.begin(); it < CreaturesManager.playersList.end(); it++)
	{
		this->sendMessageToPlayer(&(*it), type ,sender ,senderLevel , message);
	}
	return true;
}

void Game::sendPacketToAllPlayers(sf::Packet toSend)
{
	for (deque<Player>::iterator it = CreaturesManager.playersList.begin(); it < CreaturesManager.playersList.end(); it++)
	{
		if(it->socket)
			it->socket->send(toSend);
	}
}

void Game::sendTileToPlayer(Player *player, Tile *tile)
{
	if(tile && player)
	{
		player->socket->send(tile->returnTilePacket());
		if(tile->hasContainer())
		{
			sf::Packet toSend;
			unsigned int pid = 190;

			std::deque<Item*> items;
			tile->getAllItems(items);

			if(!items.empty())
			{
				for(unsigned int i = 0; i < items.size(); i++)
				{
					Container *container = items[i]->getContainer();
					if(container)
					{
						for(unsigned int n = 0; n < container->itemList.size(); n++)
						{
							toSend.clear();
							toSend << pid << tile->pos.x << tile->pos.y << tile->pos.z << tile->getIndexOfThing(container) << container->getItem(n)->mID << container->getItem(n)->count;
							player->socket->send(toSend);
						}
					}
				}
			}
		}
	}
}

void Game::receivePlayerMessage(Player *player, sf::Packet toReceive)
{
	unsigned char type;
	string message;
	toReceive >> type >> message;

	int packetId = 200;
	sf::Packet toSend;

	if(player)
	{
		if(message == "exura vita" && player->hasSpell(1) && !player->isSpellAttacking(1))
			this->useSpell(player, 1); //testowy czar leczenia!
		if(message == "exori vis" && player->hasSpell(2) && !player->isSpellAttacking(2))
			this->useSpell(player, 2); //testowy czar atakuj¹cy konkretn¹ creature
		if(message == "exori" && player->hasSpell(3) && !player->isSpellAttacking(3))
			this->useSpell(player, 3); //testowy czar atakuj¹cy obszar
		if(message == "exori flam" && player->hasSpell(4) && !player->isSpellAttacking(4))
			this->useSpell(player, 4); //testowy czar atakuj¹cy konkretn¹ creature
		if(message == "exevo flam" && player->hasSpell(5) && !player->isSpellAttacking(5))
			this->useSpell(player, 5); //testowy czar atakuj¹cy konkretn¹ creature
		if(message == "exana flam" && player->hasSpell(6) && !player->isSpellAttacking(6))
			this->useSpell(player, 6); //testowy czar atakuj¹cy tam gdzie patrzymy tu¿ przy nas
		if(message == "exana flam hur" && player->hasSpell(7) && !player->isSpellAttacking(7))
			this->useSpell(player, 7); //testowy czar atakuj¹cy tam gdzie patrzymy na obszar
		if(message == "exana vis hur" && player->hasSpell(8) && !player->isSpellAttacking(8))
			this->useSpell(player, 8); //testowy czar atakuj¹cy tam gdzie patrzymy na obszar
		if(message == "exori gran vis" && player->hasSpell(9) && !player->isSpellAttacking(9))
			this->useSpell(player, 9); //testowy czar atakuj¹cy konkretn¹ creature
		if(message == "exevo gran mas vis" && player->hasSpell(10) && !player->isSpellAttacking(10))
			this->useSpell(player, 10); //testowy czar atakuj¹cy konkretn¹ creature
		if(message == "utani gran hur")
			this->useSpell(player, 11); //testowy czar daj¹cy graczowi przyspieszenie
		if(message == "exori stun")
			this->useSpell(player, 12); //testowy czar daj¹cy graczowi przyspieszenie
		if((type == 2 && player->access == 0) || (type == 2 && player->access > 0 && message[0] != '/'))
		{
			if(message[0] == '!' && message[1] == 'o' && message[2] == 'n' && message[3] == 'l' && message[4] == 'i' && message[5] == 'n' && message[6] == 'e')
			{                      
				message = "There are ";
				message += intToStr(CreaturesManager.playersList.size());
				if(CreaturesManager.playersList.size() > 1)
					message += " players online.";
				else
					message += " player online.";
				this->sendMessageToPlayer(player, 0, "System", 0, message);  
			}
			else if(message[0] == '!' && message[1] == 'b' && message[2] == 'u' && message[3] == 'g')
			{                      
				message.erase(0,5);
				coutCurrentTime(); 
				cout << "::Bug reported by " << player->getName().c_str() << ": " << message << std::endl;
				this->sendMessageToPlayer(player, 0, "System", 0, "Thank you for bug report!");  
			}
			if(message[0] == '!' && message[1] == 'c' && message[2] == 'h' && message[3] == 'a' && message[4] == 't')
			{       
				message.erase(0,6);
				string response;
				if(CreaturesManager.getPlayer(message))
				{
					if(message != player->getName())
						this->sendMessageToPlayer(player, 4, message, 0, "New conversation with me");
					else
						this->sendMessageToPlayer(player, 0, "", 0, "You can't talk with yourself");
				}
				else
				{
					response = "Player ";
					response += message;
					response += " is offline";
					this->sendMessageToPlayer(player, 0, "", 0, response);
				}
			}
			else if(message.empty() == false)
			{
				this->sendMessageToAllPlayers(type, player->getName(), player->getLevel(), message);
			}
		}
		else if(type == 2 && player->access > 0 && message[0] == '/')
		{
			string command = message;
			command.erase(0,3); //zostaje sama komenda, np z "/c 151" usuwamy znaki "/c " i zostaje "151"

			if(message[1] == 'i' && player->access >= 5) //tworzenie itemow na danym tile, count bêdzie równa³ siê 1 (/i 124)
			{
				unsigned int id = atoi(command.c_str());
				if( id > 0 && id < BaseItems.itemsCount)
				{
					Tile *tile = Map.getTile(player->getPosX(), player->getPosY(), player->getPosZ());
					if(!tile)
						tile = new Tile(player->getPosX(), player->getPosY(), player->getPosZ());

					this->addThingToTile(new Item(id), tile);
				}
			}
			if(message[1] == 'k' && player->access >= 5) //wyrzucanie gracza
			{
			   bool kicked = false;
        
			   Player* playerKick = CreaturesManager.getPlayer(command);
			   if(playerKick)
			   {                 
				   message = "Player: ";
				   message += playerKick->getName();
				   message += " kicked.";
				   int id = 0;
				   toSend << id << message;
				   playerKick->socket->send(toSend);
				   this->sendMessageToPlayer(player, 00, "System", 0, message);

				   this->onCreatureDisappear(playerKick);    
           
				   kicked = true;
			   }           
			   if(!kicked)
				   this->sendMessageToPlayer(player, 0, "System", 0, "Player is offline or doesn't exist.");           
			}
			if(message[1] == 'a' && player->access >= 5) //teleport o dan¹ iloœæ kratek
			{
				unsigned char direction = player->getDirection();
				unsigned int distance = atoi(command.c_str());
				if(distance > 10)
					distance = 10;

				if(direction == 4)
					this->doTeleportPlayer(player, player->getPosX() - distance, player->getPosY(), player->getPosZ());
				else if(direction == 6)
					this->doTeleportPlayer(player, player->getPosX() + distance, player->getPosY(), player->getPosZ());
				else if(direction == 8)
					this->doTeleportPlayer(player, player->getPosX(), player->getPosY() - distance, player->getPosZ());
				else if(direction == 2)
					this->doTeleportPlayer(player, player->getPosX(), player->getPosY() + distance, player->getPosZ());
			}
			if(message[1] == 'g' && player->access >= 5) //teleport do innego gracza
			{
				Player *target = this->getPlayerByName(command);

				if(target)
				{
					bool exit = false;
					for(int x = -1; x <= 1; x++)
					{
						if(exit)
							break;
						for(int y = -1; y <= 1; y++)
						{
							Tile *tile = Map.getTile(target->getPosX() + x, target->getPosY() + y, target->getPosZ());
							if(tile)
							{
								this->doTeleportPlayer(player, target->getPosX() + x, target->getPosY() + y, target->getPosZ());
								exit = true;
								break;
							}
						}
					}
				}
			}
			if(message[1] == 'm' && player->access >= 5) //tworzenie potwora obok nas
			{
				unsigned int crid = CreaturesManager.createMonster(command, player->getPosX(), player->getPosY()+1, player->getPosZ(), 0);

				if(crid != 0)
					this->moveCreature(CreaturesManager.getMonster(crid), 0);
				else
					this->sendMessageToPlayer(player, 0, "System", 0, "Wrong summoning monster name.");    
			}
			if(message[1] == 't' && player->access >= 5) //teleport do predefioniowanych lokacji
			{
				if(command == "temple1")
					this->doTeleportPlayer(player, 19, 94, 7);
				else if(command == "temple2")
					this->doTeleportPlayer(player, 230, 52, 10);
				else if(command == "hell")
					this->doTeleportPlayer(player, 85, 128, 6);
				else
					this->sendMessageToPlayer(player, 0, "System", 0, "Available locations: temple1, temple2, hell");  
			}
			if(message[1] == 'p' && player->access >= 5) //uzyskanie pozycji
			{
				std::ostringstream buff;
				buff << "Position X:" << player->getPosX() << " Y:" << player->getPosY() << " Z:" << int(player->getPosZ());
				this->sendMessageToPlayer(player, 0, "System", 0, buff.str());  
			}
		}
		if(type == 4)
		{
			string toPlayer;
			toReceive >> toPlayer;

			Player *receiver = CreaturesManager.getPlayer(toPlayer);
			if(receiver)
			{
				if(receiver == player)
					this->sendMessageToPlayer(player, 0, "", 0, "You can't talk with yourself");
				else
				{
					this->sendMessageToPlayer(player, 5, player->getName(), player->getLevel(), message, toPlayer);
					this->sendMessageToPlayer(receiver, 4, player->getName(), player->getLevel(), message);
				}
			}
			else
			{
				string message = "Player ";
				message += toPlayer;
				message += " is offline";
				this->sendMessageToPlayer(player, 0, "", 0, message);
			}
		}
	}
}

void Game::sendPlayerUpdateMana(Player *player)
{
	if(player)
	{
		sf::Packet toSend;
		int pid = 98;
		toSend << pid << player->getMana();
		player->socket->send(toSend);
	}
}

void Game::receivePlayerUseSpell(Player *player, sf::Packet toReceive)
{
	unsigned char spellID;
	toReceive >> spellID;
	
	if(player && !player->isSpellAttacking(spellID) && (player->hasSpell(spellID) || player->access >= 5))
	{
		if(spellID == 1 && !player->isDefensiveSpellsCooldown())
			this->useSpell(player, spellID);
		else if(spellID > 1 && !player->isOffensiveSpellsCooldown())
			this->useSpell(player, spellID);
	}
	else if(player && !player->hasSpell(spellID))
	{
		this->sendMessageToPlayer(player, 0, "", 0, "You don't know this talent.");
		if(!player->isDefensiveSpellsCooldown() && !player->isDefensiveSpellsCooldown())
			this->sendPlayerUseSpellCancel(player, spellID);
	}
}

void Game::sendPlayerUseSpellCancel(Player *player, unsigned char spellID)
{
		sf::Packet toSend;
		int sendId = 250;
		toSend << sendId << spellID;
		player->socket->send(toSend);
}

bool Game::useSpell(Creature *creature, unsigned char spellID)
{
	if(creature)
	{
		Player *player = dynamic_cast<Player*>(creature);
		Monster *monster = dynamic_cast<Monster*>(creature);
		bool canDoCombat = this->canDoCombat(creature);

		if(spellID == 1) //exura vita
		{
			if(player)
			{
				if(player->canDoActionTime() && player->getMana() >= 100 && !player->isDefensiveSpellsCooldown())
				{
					player->addMana(-100);
					this->sendPlayerUpdateMana(player);
					player->setLastSpellAttack(spellID);
					player->setDefensiveSpellsCooldown();
				}
				else
				{
					this->sendPlayerUseSpellCancel(player, spellID);
					return false;
				}
			}
			int healthToAdd = rand()%150 + 151;
			creature->addHealth(healthToAdd);
			this->sendCreatureUpdateHealth(creature);
			this->createEffect(creature->getPosX(), creature->getPosY(), creature->getPosZ(), 6);//energy storm effect
			this->createTextEffect(creature->getPosX(), creature->getPosY(), creature->getPosZ(), 1, intToStr(healthToAdd));
		}
		else if(spellID == 2 && canDoCombat) //exori vis
		{
			Creature *target = creature->getTarget();
			if(target && this->isInRange(creature, target, 5) && canSeeThroughTiles(creature->getPosX(), creature->getPosY(), target->getPosX(), target->getPosY(), target->getPosZ()))
			{
				if(player)
				{
					if(player->canDoActionTime() && player->getMana() >= 25 && !player->isOffensiveSpellsCooldown())
					{
						player->addMana(-25);
						this->sendPlayerUpdateMana(player);
						player->setLastSpellAttack(spellID);
						player->setOffensiveSpellsCooldown();
					}
					else
					{
						this->sendPlayerUseSpellCancel(player, spellID);
						return false;
					}
				}
				this->createShootEffect(creature->getPosX(), creature->getPosY(), creature->getPosZ(), target->getPosX(), target->getPosY(), target->getPosZ(), 4); //undef. effect
				this->createEffect(target->getPosX(), target->getPosY(), target->getPosZ(), 5);//energy ball effect
				this->attackCreature(target, creature, COMBAT_ENERGYDAMAGE, spellID);
			}
		}
		else if(spellID == 3 && canDoCombat) //exori
		{
			if(player)
			{
				if(player->canDoActionTime() && player->getMana() >= 150 && !player->isOffensiveSpellsCooldown())
				{
					player->addMana(-150);
					this->sendPlayerUpdateMana(player);
					player->setLastSpellAttack(spellID);
					player->setOffensiveSpellsCooldown();
				}
				else
				{
					this->sendPlayerUseSpellCancel(player, spellID);
					return false;
				}
			}
			for(int x = -1; x <= 1; x++)
				for(int y = -1; y <= 1; y++)
					if(y != 0 || x != 0)
						this->createEffect(creature->getPosX() + x, creature->getPosY() + y, creature->getPosZ(), 8);//shock wave effect

			std::deque<Creature*> spectators;
			CreaturesManager.getCreaturesInArea(spectators , creature->getPosX(), creature->getPosY(), creature->getPosZ());
			for(unsigned int it = 0; it < spectators.size(); it++)
			{
				if(spectators[it] != creature && this->isNearPos(spectators[it], creature->getPosX(), creature->getPosY(), creature->getPosZ()))
				{
					this->attackCreature(spectators[it], creature, COMBAT_PHYSICALDAMAGE, spellID);
				}
			}
		}
		else if(spellID == 4 && canDoCombat) //exori flam
		{
			Creature *target = creature->getTarget();
			if(target && this->isInRange(creature, target, 5) && canSeeThroughTiles(creature->getPosX(), creature->getPosY(), target->getPosX(), target->getPosY(), target->getPosZ()))
			{
				if(player)
				{
					if(player->canDoActionTime() && player->getMana() >= 25 && !player->isOffensiveSpellsCooldown())
					{
						player->addMana(-25);
						this->sendPlayerUpdateMana(player);
						player->setLastSpellAttack(spellID);
						player->setOffensiveSpellsCooldown();
					}
					else
					{
						this->sendPlayerUseSpellCancel(player, spellID);
						return false;
					}
				}
				this->createShootEffect(creature->getPosX(), creature->getPosY(), creature->getPosZ(), target->getPosX(), target->getPosY(), target->getPosZ(), 3); //fireball effect
				this->createEffect(target->getPosX(), target->getPosY(), target->getPosZ(), 1);//fire effect
				this->attackCreature(target, creature, COMBAT_ENERGYDAMAGE, spellID);
			}
		}
		else if(spellID == 5 && canDoCombat) //exevo flam
		{
			Creature *target = creature->getTarget();
			if(target && this->isInRange(creature, target, 5) && canSeeThroughTiles(creature->getPosX(), creature->getPosY(), target->getPosX(), target->getPosY(), target->getPosZ()))
			{
				if(player)
				{
					if(player->canDoActionTime() && player->getMana() >= 100 && !player->isOffensiveSpellsCooldown())
					{
						player->addMana(-100);
						this->sendPlayerUpdateMana(player);
						player->setLastSpellAttack(spellID);
						player->setOffensiveSpellsCooldown();
					}
					else
					{
						this->sendPlayerUseSpellCancel(player, spellID);
						return false;
					}
				}
				this->createShootEffect(creature->getPosX(), creature->getPosY(), creature->getPosZ(), target->getPosX(), target->getPosY(), target->getPosZ(), 3); //fireball effect
				for(int x = -1; x <= 1; x++)
				for(int y = -1; y <= 1; y++)
					this->createEffect(target->getPosX() + x, target->getPosY() + y, target->getPosZ(), 1);//fire effect

				std::deque<Creature*> spectators;
				CreaturesManager.getCreaturesInArea(spectators , target->getPosX(), target->getPosY(), target->getPosZ());
				for(unsigned int it = 0; it < spectators.size(); it++)
				{
					if(spectators[it] != creature && this->isNearPos(spectators[it], target->getPosX(), target->getPosY(), target->getPosZ()))
					{
						this->attackCreature(spectators[it], creature, COMBAT_PHYSICALDAMAGE, spellID);
					}
				}
			}
		}
		else if(spellID == 6 && canDoCombat) //exana flam
		{
			unsigned char direction = creature->getDirection();
			int diff_x = 0, diff_y = 0;

			if(direction == 4)
			  diff_x = -1;
			else if(direction == 6)
			  diff_x = 1;
			else if(direction == 8)
			  diff_y = -1;
			else if(direction == 2)
			  diff_y = 1;

			if(player)
			{
				if(player->canDoActionTime() && player->getMana() >= 20 && !player->isOffensiveSpellsCooldown())
				{
					player->addMana(-20);
					this->sendPlayerUpdateMana(player);
					player->setLastSpellAttack(spellID);
					player->setOffensiveSpellsCooldown();
				}
				else
				{
					this->sendPlayerUseSpellCancel(player, spellID);
					return false;
				}
			}
			this->createEffect(creature->getPosX() + diff_x, creature->getPosY() + diff_y, creature->getPosZ(), 1);//fire effect

			std::deque<Creature*> targets = Map.getTile(creature->getPosX() + diff_x, creature->getPosY() + diff_y, creature->getPosZ())->getCreatures();
			for(unsigned int it = 0; it < targets.size(); it++)
					this->attackCreature(targets[it], creature, COMBAT_FIREDAMAGE, spellID);
		}
		else if(spellID == 7 && canDoCombat) //exana flam hur
		{
			if(player)
			{
				if(player->canDoActionTime() && player->getMana() >= 80 && !player->isOffensiveSpellsCooldown())
				{
					player->addMana(-80);
					this->sendPlayerUpdateMana(player);
					player->setLastSpellAttack(spellID);
					player->setOffensiveSpellsCooldown();
				}
				else
				{
					this->sendPlayerUseSpellCancel(player, spellID);
					return false;
				}
			}
			unsigned char direction = creature->getDirection();
			int diff_x = 0, diff_y = 0;

			if(direction == 4)
			  diff_x = -2;
			else if(direction == 6)
			  diff_x = 2;
			else if(direction == 8)
			  diff_y = -2;
			else if(direction == 2)
			  diff_y = 2;

			for(int x = -1; x <= 1; x++)
			for(int y = -1; y <= 1; y++)
			{
				//this->createEffect(creature->getPosX() + diff_x + x, creature->getPosY() + diff_y + y, creature->getPosZ(), 1);//fire effect
				this->createEffect(creature->getPosX() + diff_x + x, creature->getPosY() + diff_y + y, creature->getPosZ(), 10);//great explosion effect
				std::deque<Creature*> targets;
				Tile *tile = Map.getTile(creature->getPosX() + diff_x + x, creature->getPosY() + diff_y + y, creature->getPosZ());
				if(tile)
				{
					targets = tile->getCreatures();
					if(!targets.empty())
						for(std::deque<Creature*>::iterator it = targets.begin(); it != targets.end(); it++)
							this->attackCreature(*it, creature, COMBAT_FIREDAMAGE, spellID);
				}
			}
		}
		else if(spellID == 8 && canDoCombat) //exana vis hur
		{
			if(player)
			{
				if(player->canDoActionTime() && player->getMana() >= 80 && !player->isOffensiveSpellsCooldown())
				{
					player->addMana(-80);
					this->sendPlayerUpdateMana(player);
					player->setLastSpellAttack(spellID);
					player->setOffensiveSpellsCooldown();
				}
				else
				{
					this->sendPlayerUseSpellCancel(player, spellID);
					return false;
				}
			}
			unsigned char direction = creature->getDirection();
			int diff_x = 0, diff_y = 0;

			if(direction == 4)
			  diff_x = -2;
			else if(direction == 6)
			  diff_x = 2;
			else if(direction == 8)
			  diff_y = -2;
			else if(direction == 2)
			  diff_y = 2;

			for(int x = -1; x <= 1; x++)
			for(int y = -1; y <= 1; y++)
			{
				//this->createEffect(creature->getPosX() + diff_x + x, creature->getPosY() + diff_y + y, creature->getPosZ(), 5);//energy ball effect
				this->createEffect(creature->getPosX() + diff_x + x, creature->getPosY() + diff_y + y, creature->getPosZ(), 9);//energy strike effect
				std::deque<Creature*> targets;
				Tile *tile = Map.getTile(creature->getPosX() + diff_x + x, creature->getPosY() + diff_y + y, creature->getPosZ());
				if(tile)
				{
					targets = tile->getCreatures();
					if(!targets.empty())
						for(std::deque<Creature*>::iterator it = targets.begin(); it != targets.end(); it++)
							this->attackCreature(*it, creature, COMBAT_FIREDAMAGE, spellID);
				}
			}
		}
		else if(spellID == 9 && canDoCombat) //exori gran vis
		{
			Creature *target = creature->getTarget();
			if(target && this->isInRange(creature, target, 5) && canSeeThroughTiles(creature->getPosX(), creature->getPosY(), target->getPosX(), target->getPosY(), target->getPosZ()))
			{
				if(player)
				{
					if(player->canDoActionTime() && player->getMana() >= 80 && !player->isOffensiveSpellsCooldown())
					{
						player->addMana(-80);
						this->sendPlayerUpdateMana(player);
						player->setLastSpellAttack(spellID);
						player->setOffensiveSpellsCooldown();
					}
					else
					{
						this->sendPlayerUseSpellCancel(player, spellID);
						return false;
					}
				}
				this->createEffect(target->getPosX(), target->getPosY(), target->getPosZ(), 7); //energy bomb effect
				this->createShootEffect(creature->getPosX(), creature->getPosY(), creature->getPosZ(), target->getPosX(), target->getPosY(), target->getPosZ(), 3); //fireball effect
				this->attackCreature(target, creature, COMBAT_ENERGYDAMAGE, spellID);
			}
		}
		else if(spellID == 10 && canDoCombat) //exevo gran mas vis
		{
			if(player)
			{
				if(player->canDoActionTime() && player->getMana() >= 400 && !player->isOffensiveSpellsCooldown())
				{
					player->addMana(-400);
					this->sendPlayerUpdateMana(player);
					player->setLastSpellAttack(spellID);
					player->setOffensiveSpellsCooldown();
				}
				else
				{
					this->sendPlayerUseSpellCancel(player, spellID);
					return false;
				}
			}
			for(int x = -5; x <= 5; x++)
				for(int y = -5; y <= 5; y++)
					if(this->isInRange(creature, creature->getPosX() + x, creature->getPosY() + y, creature->getPosZ(), 4))
						this->createEffect(creature->getPosX() + x, creature->getPosY() + y, creature->getPosZ(), 7); //energy bomb effect

			std::deque<Creature*> spectators;
			CreaturesManager.getCreaturesInArea(spectators , creature->getPosX(), creature->getPosY(), creature->getPosZ());
			for(unsigned int it = 0; it < spectators.size(); it++)
			{
				if(spectators[it] != creature && this->isInRange(creature, spectators[it], 4))
				{
					this->attackCreature(spectators[it], creature, COMBAT_ENERGYDAMAGE, spellID);
				}
			}
		}
		else if(spellID == 11) //utani gran hur
		{
			if(player)
			{
				if(player->canDoActionTime() && player->getMana() >= 10 && !player->isDefensiveSpellsCooldown())
				{
					player->addMana(-10);
					this->sendPlayerUpdateMana(player);
					player->setLastSpellAttack(spellID);
					player->setDefensiveSpellsCooldown();
				}
				else
				{
					this->sendPlayerUseSpellCancel(player, spellID);
					return false;
				}
			}
			this->doCreatureTemporaryChangeSpeed(creature, 0.7f, 5000);
			this->createEffect(creature->getPosX(), creature->getPosY(), creature->getPosZ(), 6);//energy storm effect
		}
		else if(spellID == 12 && canDoCombat) //exori stun
		{
			if(player)
			{
				if(player->canDoActionTime() && player->getMana() >= 300 && !player->isOffensiveSpellsCooldown())
				{
					player->addMana(-300);
					this->sendPlayerUpdateMana(player);
					player->setLastSpellAttack(spellID);
					player->setOffensiveSpellsCooldown();
				}
				else
				{
					this->sendPlayerUseSpellCancel(player, spellID);
					return false;
				}
			}
			for(int x = -1; x <= 1; x++)
				for(int y = -1; y <= 1; y++)
					if(y != 0 || x != 0)
						this->createEffect(creature->getPosX() + x, creature->getPosY() + y, creature->getPosZ(), 8);//shock wave effect

			std::deque<Creature*> spectators;
			CreaturesManager.getCreaturesInArea(spectators , creature->getPosX(), creature->getPosY(), creature->getPosZ());
			for(unsigned int it = 0; it < spectators.size(); it++)
			{
				if(spectators[it] != creature && this->isNearPos(spectators[it], creature->getPosX(), creature->getPosY(), creature->getPosZ()))
					this->doCreatureTemporaryChangeSpeed(spectators[it], 3.0f, 3000);
			}
		}
		else if(canDoCombat == false && spellID > 1)
		{
			this->sendPlayerUseSpellCancel(player, spellID);
		}
		if(player)
			player->setActionTime();

		creature->setLastSpellAttack(spellID);
		return true;
	}
	else
		return false;
}

bool Game::canPlayerPayMoney(Player *player, unsigned int money)
{
	return true; //!!!!!!!!!!!!!!!!!
}

void Game::sendPlayerNPCshop(Player *player, NPC *npc)
{
	if(player && npc)
	{
		int pid = 601;
		sf::Packet toSendSells;
		sf::Packet toSendBuys;
		deque<NPCshopEntry> shop;
		npc->getShopEntries(shop);
		toSendSells << pid;
		pid = 602;
		toSendBuys << pid;

		for(deque<NPCshopEntry>::iterator it = shop.begin(); it != shop.end(); it++)
		{
			if(it->type == SHOP_ACTION_SELL)
			{
				toSendSells << it->itemId << it->price;
			}
			else if(it->type == SHOP_ACTION_BUY)
			{
				toSendBuys << it->itemId << it->price;
			}
		}
		player->socket->send(toSendSells);
		player->socket->send(toSendBuys);
	}
}

bool Game::addPlayerExperience(Player *player, unsigned int amount)
{
	if(!player)
		return false;

	player->addExperience(amount);
	this->createTextEffect(player->getPosX(), player->getPosY(), player->getPosZ(), 1, intToStr(amount));
	while(player->getExperience() >= player->getNextLevelExperience())
	{
		player->onAdvance();

		string message = "Congratulations!! You advanced to level ";
		message += intToStr(player->getLevel());
		message += ".";
		this->sendMessageToPlayer(player, 0, "", 0, message);
		this->createEffect(player->getPosX(), player->getPosY(), player->getPosZ(), 6);

		player->setHealth(player->getMaxHealth());
		this->sendCreatureUpdateHealth(player);
		player->setMana(player->getMaxMana());

		#ifdef _DEBUG
			cout << "::" << player->getName() << " advanced from level " << player->getLevel()-1 << " to level " << player->getLevel() << endl;
		#endif
	}
	this->sendPlayerStatistics(player);
	return true;
}

void Game::processClient(ClientContext *clientContext)
{
	if(clientContext->socket->receive(clientContext->toReceive) == sf::Socket::Done)
	{
		clientContext->sendId = -1;
		clientContext->recvId = -1;
		clientContext->toReceive >> clientContext->recvId;

		if(clientContext->recvId == 1337) // klient wys³a³ id 1337 - kontrola wersji
		{
			clientContext->toReceive >> clientContext->recvMsg;

			#ifdef _DEBUG
			   cout << "         ::" << clientContext->recvMsg.c_str() << " - Client version" << endl;
			   cout << "         ::0.2.0 - Required version" <<endl;     
			#endif
			if(clientContext->recvMsg != this->serverVersion)
			{
			   #ifdef _DEBUG
				   coutCurrentTime();
				   std::cout << "::Client ip: " << clientContext->socket->getRemoteAddress() << " disconnected, wrong client version" << std::endl;
			   #endif
            
			   clientContext->sendId = 0;
			   clientContext->sendMsg = "Please download new client version from official website.";
			   clientContext->toSend.clear();							 
			   clientContext->toSend << clientContext->sendId << clientContext->sendMsg;
			   clientContext->socket->send(clientContext->toSend);

			   selector.remove(*(clientContext->socket));
			   clientContext->socket->disconnect();
			   clientContext->lostPlayer = true;
			}
			else
			{
			   clientContext->sendId = 1337;
			   clientContext->toSend.clear();
			   clientContext->toSend << clientContext->sendId;
			   clientContext->socket->send(clientContext->toSend);

			   #ifdef _DEBUG
				   coutCurrentTime();
				   cout << "::Sended WELCOME to client ID [" << clientContext->id << "]." << endl;
			   #endif
			}          
		}		
		if (clientContext->recvId == 0) // klient wys³a³ id 0 - nag³e zamkniêcie lub logout
		{
			coutCurrentTime();
			std::cout << "::Client id: " << clientContext->id << ", ip: " << clientContext->socket->getRemoteAddress() << " disconnected" << std::endl;
			this->onCreatureDisappear(clientContext->player);
			selector.remove(*(clientContext->socket));
			clientContext->socket->disconnect();
			clientContext->lostPlayer = true;
			clientContext->player = NULL;
		}
		else if(clientContext->recvId == 1) //Logowanie konta: 1   ACCOUNT   PASSWORD
		{
			string message = "Internal server error!";
			string name, password;
			clientContext->toReceive >> name >> password;

			string resultPassword;
			bool success = AccountsManager.getPassword(name, resultPassword);
			if(success && password == resultPassword)
				success = true;
			else
			{
				success = false;
				message = "Account name or password is incorrect!";
			}

			#ifdef _DEBUG
				coutCurrentTime();
				cout << "::Client ID " << clientContext->id << " try to login. " << endl;
				cout << "           Account: " << name << " Password: " << password << endl;
				cout << "           Success: " << success << " (password from file: " << resultPassword << ")" << endl;
			#endif

			if(success)
			{
				if(!BansManager.isAccountBanished(name))
				{
					Account acc;
					acc = AccountsManager.loadAccount(name);
					clientContext->accountName = name;

					clientContext->sendId = 1;
					clientContext->toSend.clear();
					clientContext->toSend << clientContext->sendId << success << acc.charList.size();

					for(unsigned int i = 0; i < acc.charList.size(); i++)
					{
						clientContext->toSend << acc.charList[i];
					}
					clientContext->socket->send(clientContext->toSend); //Logowanie siê uda³o, wysy³amy do clienta pakiet: 1	TRUE	ILOSC_POSTACI	NAZWA_POSTACI

					#ifdef _DEBUG
						cout << "           Characters: ";
						for(unsigned int i = 0; i < acc.charList.size(); i++)
						{
							cout << acc.charList[i] << ", ";
						}
						cout << endl;
					#endif
				}
				else
				{
					#ifdef _DEBUG
						cout << "           Account is Banished!\n";
					#endif
					message = "This account is banished.";
					clientContext->sendId = 1;
					clientContext->toSend.clear();
					clientContext->toSend << clientContext->sendId << success << message;
					clientContext->socket->send(clientContext->toSend);

					success = false;
				}
			}
			else
			{
				clientContext->sendId = 1;
				clientContext->toSend.clear();
				clientContext->toSend << clientContext->sendId << success << message;
				clientContext->socket->send(clientContext->toSend); //Logowanie siê nie uda³o, wysy³amy do clienta pakiet: 1	FALSE	MESSAGE
			}
		}
		else if(clientContext->recvId == 2) //Pakiet tworzenia konta: 2   ACCOUNT   PASSWORD	EMAIL
		{
			bool success = false;
			Account acc;
			clientContext->toReceive >> acc.name >> acc.password >> acc.email;

			Account tempAcc = AccountsManager.loadAccount(acc.name);
			if(tempAcc.name == "" && acc.email.find("@") != string::npos && acc.password.size() > 5 && acc.email.find(".") != string::npos && acc.email.size() >= 5 && 
				acc.name.size() > 5 && acc.name.size() <= 20 && acc.password.size() <= 20)
			{
				#ifdef _DEBUG
					coutCurrentTime();
					cout << "::Client ID " << clientContext->id << " created account." << endl;
					cout << "           Account: " << acc.name << " Password: " << acc.password << endl;
				#endif

				AccountsManager.saveAccount(acc);
				clientContext->accountName = acc.name;

				success = true;
			}
			else
			{
				#ifdef _DEBUG
					coutCurrentTime();
					cout << "::Client ID " << clientContext->id << " can't create account." << endl;
				#endif

				success = false;
			}
			clientContext->sendId = 2;
			clientContext->toSend.clear();
			clientContext->toSend << clientContext->sendId << success;
			clientContext->socket->send(clientContext->toSend); //Rezultat tworzenia konta: 2	CZY_SIE_UDALO
		}
		else if(clientContext->recvId == 3) //Logowanie postaci: 3   NAME
		{
			string message = "Internal server error!";
			bool proceed = false;
			if(clientContext->player == NULL && clientContext->accountName != "")
			{
				string name;
				clientContext->toReceive >> name;

				Account acc;
				acc = AccountsManager.loadAccount(clientContext->accountName);

				for(unsigned int i = 0; i < acc.charList.size(); i++)
				{
					if(acc.charList[i] == name)
					{
						if(ConfigManager.maxPlayers > CreaturesManager.getPlayersCount() || acc.access >= 3) 
						{
							for(unsigned int nn = 0; nn < acc.charList.size(); nn++)
							{
								if(CreaturesManager.getPlayer(acc.charList[nn]))
								{
									message = "Only one player per account can be online at same time.";
									break;
								}
							}
							proceed = CreaturesManager.loadPlayer(name);
							clientContext->player = CreaturesManager.getPlayer(name);
							clientContext->player->socket = clientContext->socket;
							if(proceed && clientContext->player->getAccountName() == acc.name)
							{
								#ifdef _DEBUG
									coutCurrentTime();
									cout << "::Client ID " << clientContext->id << " logged player: " << name << endl;
								#endif

								break;
							}
							clientContext->player = NULL;
							message = "Player allready logged!";
							proceed = false;
							break;
						}
						else
						  message = "Too many players online! Please try again.";
						break;
					}
				}
			}
			else
			{
				message = "Only 1 player online per account is allowed!";
				proceed = false;
			}

			clientContext->sendId = 3;
			clientContext->toSend.clear();
			clientContext->toSend << clientContext->sendId << proceed << message;
			clientContext->socket->send(clientContext->toSend); //Logowanie postacii: 3	CZY_SIE_UDALO	MESSAGE

			if(proceed)
			  this->processPlayerLogin(clientContext->player);
		}
		else if(clientContext->recvId == 4) //Tworzenie postacii: 4   NAME
		{
			bool proceed = false;
			if(clientContext->accountName != "") //sprawdzamy, czy client ma zalogowane konto
			{
				string name;
				clientContext->toReceive >> name;

				Account acc;
				acc = AccountsManager.loadAccount(clientContext->accountName);

				for(unsigned int i = 0; i < acc.charList.size(); i++)
				{
					if(acc.charList[i] == name) //sprawdzamy, czy na koncie nie istnieje ju¿ postaæ o zadanym nicku
					{
						proceed = false;
						break;
					}
				}

				if(this->isValidName(name, name) && acc.charList.size() < 5)
					proceed = true;
				else
					proceed = false;

				if(proceed && CreaturesManager.createPlayer(name) != 0) //sprawdzamy, czy postaæ o podanym nicku istnieje. Je¿eli nie - tworzymy j¹ i kontynuujemy.
				{
					CreaturesManager.getPlayer(name)->setAccountName(acc.name);  //przypisujemy postaci nazwe konta
					CreaturesManager.savePlayer(name); //zapisujemy postaæ
					CreaturesManager.removePlayer(name); //usuwamy j¹ z listy graczy

					acc.charList.push_back(name); //dodajemy nick postaci do listy na koncie
					AccountsManager.saveAccount(acc); //zapisujemu konto
				
					proceed = true;

					#ifdef _DEBUG
						coutCurrentTime();
						cout << "::Client ID " << clientContext->id << " created player: " << name << endl;
					#endif
				}
				else
					proceed = false;
			}
			clientContext->sendId = 4;
			clientContext->toSend.clear();
			clientContext->toSend << clientContext->sendId << proceed;
			clientContext->socket->send(clientContext->toSend); //Tworzenie postacii: 4	CZY_SIE_UDALO
		}
		else if(clientContext->recvId == 99) 
		{            
		   unsigned char index;
		   clientContext->toReceive >> index;
		   if(clientContext->player && clientContext->player->getStatisticsPoints() > 0)
		   {
			   if(index == 1)
			   {
				   clientContext->player->addStrength(1);
				   clientContext->player->setStatisticsPoints(clientContext->player->getStatisticsPoints() - 1);
			   }
			   else if(index == 2)
			   {
				   clientContext->player->addDexterity(1);
				   clientContext->player->setStatisticsPoints(clientContext->player->getStatisticsPoints() - 1);
			   }
			   else if(index == 3)
			   {
				   clientContext->player->addMagicPower(1);
				   clientContext->player->setStatisticsPoints(clientContext->player->getStatisticsPoints() - 1);
			   }

			   this->sendPlayerStatistics(clientContext->player);
		   }
		   else
			   this->sendMessageToPlayer(clientContext->player, 0, "", 0, "You don't have Statistics Points to distribute.");
		}
		else if(clientContext->recvId == 120) 
		{            
		   int direction;
		   clientContext->toReceive >> direction;
		   this->internalMovePlayer(clientContext->player, char(direction));
		}
		else if(clientContext->recvId == 142) //obrót postacii gracza w miejscu
		{           
		   int direction;
		   clientContext->toReceive >> direction;
		   this->doPlayerChangeDirection(clientContext->player, direction);
		}
		else if(clientContext->recvId == 170) 
		{            
		   this->receiveThrowItem(clientContext->player, clientContext->toReceive);
		}
		else if(clientContext->recvId == 171) 
		{            
		   this->receiveEquipItem(clientContext->player, clientContext->toReceive);
		}
		else if(clientContext->recvId == 175) 
		{            
		   this->receiveRequestItemStats(clientContext->player, clientContext->toReceive);
		}
		else if(clientContext->recvId == 180)
		{
		   this->receiveUseItem(clientContext->player, clientContext->toReceive);
		}
		else if(clientContext->recvId == 200)
		{
		   this->receivePlayerMessage(clientContext->player, clientContext->toReceive);
		}
		else if(clientContext->recvId == 220)
		{
		   this->receivePlayerAttackCreature(clientContext->player, clientContext->toReceive);
		}
		else if(clientContext->recvId == 250)
		{
		   this->receivePlayerUseSpell(clientContext->player, clientContext->toReceive);
		}
		else if(clientContext->recvId == 600 && clientContext->player)
		{
			unsigned char choice;
			clientContext->toReceive >> choice;
			NPC *npc = clientContext->player->npcTalked;
			if(npc)
			{
				if(this->isInRange(clientContext->player, npc, 2))
				{
					sf::Packet packet = npc->playerChoose(clientContext->player->getId(), choice-1);
					clientContext->socket->send(packet);
				}
				else
					this->sendMessageToPlayer(clientContext->player, 0, "", 0, "You should stand closer to your talking partner");
			}
		}
		else if(clientContext->recvId == 601 && clientContext->player)
		{
			unsigned int choice;
			clientContext->toReceive >> choice;
			NPC *npc = clientContext->player->npcTalked;
			if(npc)
			{
				if(this->isInRange(clientContext->player, npc, 2))
				{
					int price = npc->canBuyItem(choice);
					if(price != -1)
					{
						if(this->addItemToPlayerStorage(clientContext->player, new Item(choice, 1), 20))
						{
							if(this->removeItemIdFromPlayerStorage(clientContext->player, 294, price) == false)
							{
								this->removeItemIdFromPlayerStorage(clientContext->player, choice, 1);
								this->sendMessageToPlayer(clientContext->player, 0, "", 0, "You don't have enough money");
							}
						}
						else
							this->sendMessageToPlayer(clientContext->player, 0, "", 0, "You don't have enough space in your backpack");
					}
				}
				else
					this->sendMessageToPlayer(clientContext->player, 0, "", 0, "You should stand closer to your talking partner");
			}
		}
		else if(clientContext->recvId == 602 && clientContext->player)
		{
			unsigned int choice;
			clientContext->toReceive >> choice;
			NPC *npc = clientContext->player->npcTalked;
			if(npc)
			{
				if(this->isInRange(clientContext->player, npc, 2))
				{
					int price = npc->canSellItem(choice);
					if(price != -1)
					{
						if(this->removeItemIdFromPlayerStorage(clientContext->player, choice, 1))
							this->addItemToPlayerStorage(clientContext->player, new Item(294, price), 20);
						else
							this->sendMessageToPlayer(clientContext->player, 0, "", 0, "You don't have that item in your backpack");
					}
				}
				else
					this->sendMessageToPlayer(clientContext->player, 0, "", 0, "You should stand closer to your talking partner");
			}
		}
	}
}