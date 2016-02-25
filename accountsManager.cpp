#include "accountsManager.h"
#include <deque>
#include <fstream>
#include <iostream>

using namespace std;

Account AccountsManager::loadAccount(std::string& name)
{
	Account acc;

	deque<std::string> lines;
	deque<std::string> names;
	deque<std::string> values;
	string tmp;

    std::string extension = ".acc";
    std::string filename = "Accounts\\";
    filename += name;
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
		return acc;

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
	for(unsigned int i = 0; i < names.size(); i++)
	{
		if(names[i] == "name")
			acc.name = values[i];	
		if(names[i] == "password")
			acc.password = values[i];
		if(names[i] == "email")
			acc.email = values[i];
		if(names[i] == "access")
			acc.access = atoi(values[i].c_str());
		if(names[i] == "premiumTime")
			acc.premiumTime = atoi(values[i].c_str());
		if(names[i] == "player")
			acc.charList.push_back(values[i]);
	}
	return acc;
}

bool AccountsManager::saveAccount(Account &account)
{
	std::string extension = ".acc";
	std::string filename = "Accounts\\";
	filename += account.name;
	filename += extension;

	std::ofstream plik_wy(filename.c_str(), ios_base::trunc);
	{
		plik_wy<<"name="<<account.name<<"\n";
		plik_wy<<"password="<<account.password<<"\n";
		plik_wy<<"email="<<account.email<<"\n";
		plik_wy<<"access="<<account.access<<"\n";
		plik_wy<<"premiumTime="<<account.premiumTime<<"\n";

		for (unsigned int i = 0; i < account.charList.size(); i++)
		{
			plik_wy<<"player="<<account.charList[i]<<"\n";
		}
		plik_wy.close();
	}
	return true;
}

bool AccountsManager::getPassword(std::string& name, std::string &password)
{
	deque<std::string> lines;
	deque<std::string> names;
	deque<std::string> values;
	string tmp;

    std::string extension = ".acc";
    std::string filename = "Accounts\\";
    filename += name;
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
	for(unsigned int i = 0; i < names.size(); i++)
	{
		if(names[i] == "password")
		{
			password = values[i];
			return true;
		}
	}
	return false;	
}
