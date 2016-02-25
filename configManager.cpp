#include <deque>
#include <string>
#include <iostream>
#include <fstream>
#include "configManager.h"

using namespace std;

bool stringToBool(string i)
{
	if(i == "enabled")
		return true;
	else
		return false;
}

ConfigManager::ConfigManager()
{
	this->m_isLoaded = this->loadFile();
}

bool ConfigManager::loadFile()
{
	if(!this->m_isLoaded)
	{
		deque<std::string> lines;
		deque<std::string> names;
		deque<std::string> values;
		string tmp;

		std::string filename = "config.cfg";

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
				if(name.find_first_of(" ") != string::npos)
				{
					name.erase(name.find_first_of(" "), name.size()-1);
				}
				names.push_back(name);

				string value = lines[i];
				value.erase(0,value.find_first_of("'")+1);
				value.erase(value.find_first_of("'"), value.size()-1);
				values.push_back(value);
			}
		}

		for(unsigned int i = 0; i < names.size(); i++)
		{
			if(names[i] == "port")
				this->port = atoi(values[i].data());	
			if(names[i] == "pvp")
				this->pvp = stringToBool(values[i].data());	
			if(names[i] == "pzLock")
				this->pzLock = atoi(values[i].data());
			if(names[i] == "huntingLock")
				this->huntingLock = atoi(values[i].data());
			if(names[i] == "protectionLevel")
				this->protectionLevel = atoi(values[i].data());
			if(names[i] == "noDamageToPartyMembers")
				this->noDamageToPartyMembers = stringToBool(values[i].data());	
			if(names[i] == "save")
				this->save = stringToBool(values[i].data());
			if(names[i] == "saveInterval")
				this->saveInterval = atoi(values[i].data());
			if(names[i] == "maxPlayers")
				this->maxPlayers = atoi(values[i].data());
			if(names[i] == "playerActionInterval")
				this->playerActionInterval = atoi(values[i].data());
			if(names[i] == "checkConnectionInterval")
				this->checkConnectionInterval = atoi(values[i].data());
		}
		this->m_isLoaded = true;	
		return true;
	}
	return false;
}