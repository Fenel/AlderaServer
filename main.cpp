#include <iostream>
#include <windows.h>
#include <time.h>

#include <SFML/Network.hpp>

#include "configManager.h"
#include "game.h"
#include "baseItems.h"
#include "creaturesManager.h"
#include "accountsManager.h"
#include "bansManager.h"
#include "map.h"
#include "pathFind.h"

ConfigManager ConfigManager;
BaseItems BaseItems;
CreaturesManager CreaturesManager;
AccountsManager AccountsManager;
BansManager BansManager;
Map Map;
PathFind PathFind;

Game Game;

std::string intToStr(int n)
{
     std::string tmp;
     if(n < 0) {
          tmp = "-";
          n = -n;
     }
     if(n > 9)
          tmp += intToStr(n / 10);
     tmp += n % 10 + 48;
     return tmp;
}

bool isLettersOnly(char ch) 
{
        return ( ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) || 
                 (ch == ' ') || (ch == 39));
}

bool isLettersOnly(std::string str)
{
        for (std::string::iterator it = str.begin(); it != str.end(); it++) {
                if (!isLettersOnly((char)*it))
                        return false;
        }        
        return true;
}

void coutCurrentTime()
{
    string temp;
    SYSTEMTIME st;
    GetLocalTime(&st);
    if(st.wHour < 10)
    {
        cout << "0" << st.wHour << ":";
    }
    else cout << st.wHour << ":";
    
    if(st.wMinute < 10)
    {
        cout << "0" << st.wMinute << ":";
    }
    else cout << st.wMinute << ":";
    
    if(st.wSecond < 10)
    {
        cout << "0" << st.wSecond << " ";
    }
    else cout << st.wSecond << " ";
}

int main()
{

	system("PAUSE");
	return 0;
}

