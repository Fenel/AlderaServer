#include <deque>
#include <string>
#include <time.h>
#include <iostream>
#include <fstream>

#include <SFML/Network.hpp>

#include "bansManager.h"

#include "creaturesManager.h"

extern CreaturesManager CreaturesManager;

BansManager::BansManager()
{
	loadBans();
}

BansManager::~BansManager()
{
}

void BansManager::saveBans()
{
	std::string filename = "Bans.txt";

	std::ofstream file(filename.c_str());
	{
		file.clear();
		file.close();
	}	
	
	for(unsigned int it = 0; it < bans.size(); it++)
	{
		std::ofstream file(filename.c_str(),std::ios_base::ate);
		{
			file << int(bans.at(it).type) << "|" << bans.at(it).banished_ident.c_str() << "|" << static_cast<unsigned int>(bans.at(it).added) << "|" << static_cast<unsigned int>(bans.at(it).expires) << "|" << bans.at(it).admin_ident.c_str() << "|" << bans.at(it).comment.c_str() << "|";
			file.close();
		}
	}
}

void BansManager::loadBans()
{
	std::deque<std::string> ban_strings;
	std::string tmp;

	std::ifstream file ("Bans.txt");
	if(file.is_open())
	{
		while(file)
		{
		   getline(file, tmp,'\n');

		   ban_strings.push_back(tmp);
		}   
		file.close();

		for(unsigned int it = 0; it < ban_strings.size(); it++)
		{
			Ban banik;

			std::string c, ss;
			int n = ban_strings[it].length();
			int licznik = 0;
			for(int i = 0; i < n; i++)
			{ 
			   c = ban_strings[it].at(i);
			   if(c != "|")
			   {
				  ss += c;
			   }
			   if(c == "|")
			   {
				  if( licznik == 0)
				  {
					  banik.type = BanType_t(atoi(ss.c_str()));
				  }
				  if( licznik == 1)
				  {
					  banik.banished_ident = ss;
				  }
				  if( licznik == 2)
				  {
					  banik.added = time_t(atoi(ss.c_str()));
				  }
				  if( licznik == 3)
				  {
					  banik.expires = time_t(atoi(ss.c_str()));
				  }
				  if( licznik == 4)
				  {
					  banik.admin_ident = ss;
				  }
				  if( licznik == 5)
				  {
					  banik.comment = ss;
				  }
				  licznik++;
				  ss = "";
			   }
			}
			if(banik.expires > time(NULL))
			{
				bans.push_back(banik);
			}
		}
	}
}

bool BansManager::isIpBanished(std::string ip)
{
   for (std::deque<Ban>::iterator it = bans.begin(); it < bans.end(); it++)
   {
      if (it->type == BAN_IPADDRESS && it->banished_ident == ip)
	  {
	     if(it->added < it->expires)
		 {
			 return true;
		 }
		 else
		 {
			 bans.erase(it);
		 }
	  }
   }
   return false;
}


bool BansManager::isPlayerBanished(std::string name)
{
   for (std::deque<Ban>::iterator it = bans.begin(); it < bans.end(); it++)
   {
      if (it->type == BAN_PLAYER && it->banished_ident == name)
	  {
	     if(it->added < it->expires)
		 {
			 return true;
		 }
		 else
		 {
			 bans.erase(it);
		 }
	  }
   }
   return false;
}

bool BansManager::isPlayerBanished(unsigned int id)
{
	if (CreaturesManager.getPlayer(id))
	{
		return(isPlayerBanished(CreaturesManager.getPlayer(id)->getName()));
	}
	return false;
}

bool BansManager::isAccountBanished(std::string account)
{
   for (std::deque<Ban>::iterator it = bans.begin(); it < bans.end(); it++)
   {
      if (it->type == BAN_ACCOUNT && it->banished_ident == account)
	  {
	     if(it->added < it->expires)
		 {
			 return true;
		 }
		 else
		 {
			 bans.erase(it);
		 }
	  }
   }
   return false;
}

void BansManager::addIpBan(std::string ip, unsigned int _time, std::string admin_ident, std::string comment)
{
	Ban banik;
	banik.type = BAN_IPADDRESS;
	banik.added = time(NULL);
	banik.admin_ident = admin_ident;
	banik.banished_ident = ip;
	banik.expires = banik.added + static_cast<time_t>(_time);
	banik.comment = comment;
	bans.push_back(banik);

	std::string filename = "Bans.txt";
	std::ofstream file(filename.c_str(),std::ios_base::ate);
	{
		file << int(bans[bans.size() -1].type) << "|" << bans[bans.size() -1].banished_ident.c_str() << "|" << static_cast<unsigned int>(bans[bans.size() -1].added) << "|" << static_cast<unsigned int>(bans[bans.size() -1].expires) << "|" << bans[bans.size() -1].admin_ident.c_str() << "|" << bans[bans.size() -1].comment.c_str() << "|";
		file.close();
	}
}

void BansManager::addPlayerBan(std::string playerName, unsigned int _time, std::string admin_ident, std::string comment)
{
	Ban banik;
	banik.type = BAN_PLAYER;
	banik.added = time(NULL);
	banik.admin_ident = admin_ident;
	banik.banished_ident = playerName;
	banik.expires = banik.added + static_cast<time_t>(_time);
	banik.comment = comment;
	bans.push_back(banik);

	std::string filename = "Bans.txt";
	std::ofstream file(filename.c_str(),std::ios_base::ate);
	{
		file << int(bans[bans.size() -1].type) << "|" << bans[bans.size() -1].banished_ident.c_str() << "|" << static_cast<unsigned int>(bans[bans.size() -1].added) << "|" << static_cast<unsigned int>(bans[bans.size() -1].expires) << "|" << bans[bans.size() -1].admin_ident.c_str() << "|" << bans[bans.size() -1].comment.c_str() << "|";
		file.close();
	}
}

void BansManager::addAccountBan(std::string accountName, unsigned int _time, std::string admin_ident, std::string comment)
{
	Ban banik;
	banik.type = BAN_ACCOUNT;
	banik.added = time(NULL);
	banik.admin_ident = admin_ident;
	banik.banished_ident = accountName;
	banik.expires = banik.added + static_cast<time_t>(_time);
	banik.comment = comment;
	bans.push_back(banik);

	std::string filename = "Bans.txt";
	std::ofstream file(filename.c_str(),std::ios_base::ate);
	{
		file << int(bans[bans.size() -1].type) << "|" << bans[bans.size() -1].banished_ident.c_str() << "|" << static_cast<unsigned int>(bans[bans.size() -1].added) << "|" << static_cast<unsigned int>(bans[bans.size() -1].expires) << "|" << bans[bans.size() -1].admin_ident.c_str() << "|" << bans[bans.size() -1].comment.c_str() << "|";
		file.close();
	}
}

bool BansManager::removeIpBans(std::string ip)
{
   bool result = false;  

   for (std::deque<Ban>::iterator it = bans.begin(); it < bans.end(); it++)
   {
      if (it->type == BAN_IPADDRESS && it->banished_ident == ip)
	  {
		bans.erase(it);
		result = true;
	  }
   }	
   return result;
}

bool BansManager::removePlayerBans(unsigned int id)
{
	bool result = false;

	if (CreaturesManager.getPlayer(id))
	{
		result = removePlayerBans(CreaturesManager.getPlayer(id)->getName());
	}	
    return result;
}

bool BansManager::removePlayerBans(std::string playerName)
{
   bool result = false;

   for (std::deque<Ban>::iterator it = bans.begin(); it < bans.end(); it++)
   {
      if (it->type == BAN_PLAYER && it->banished_ident == playerName)
	  {
		bans.erase(it);
		result = true;
	  }
   }
   return result;
}

bool BansManager::removeAccountBans(std::string accountName)
{
   bool result = false;

   for (std::deque<Ban>::iterator it = bans.begin(); it < bans.end(); it++)
   {
      if (it->type == BAN_ACCOUNT && it->banished_ident == accountName)
	  {
		bans.erase(it);
		result = true;
	  }
   }
   return result;
}
