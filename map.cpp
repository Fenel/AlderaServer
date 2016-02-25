#include <iostream>
#include <time.h>
#include <string>
#include <fstream>

#include <SFML/Network.hpp>

#include "map.h"
#include "container.h"
#include "player.h"
#include "game.h"

extern Game Game;
extern void coutCurrentTime();

int MAP_MAX_DIMENSIONS = 640;

Map::Map()
{

}

unsigned int Map::posToIndex(unsigned int x, unsigned int y, unsigned char z)
{
   unsigned int R = 0, multiplier = 1;

   R += multiplier * x;
   multiplier *= MAP_MAX_DIMENSIONS;

   R += multiplier * y;
   multiplier *= MAP_MAX_DIMENSIONS;

   R += multiplier * int(z);
   multiplier *= 15;  
              
   return R;
}

void Map::indexToPos(unsigned int &x, unsigned int &y, unsigned char &z, unsigned int index)
{
   unsigned int R = 1;

   x = index % MAP_MAX_DIMENSIONS;
   index /= MAP_MAX_DIMENSIONS;
      
   y = index % MAP_MAX_DIMENSIONS;
   index /= MAP_MAX_DIMENSIONS;
      
   z = char(index%15);            
}

void Map::setTile(Position pos, Tile *tile)
{
    setTile(pos.x,pos.y,pos.z,tile);
}

void Map::setTile(unsigned short x, unsigned short y, unsigned char z, Tile *tile)
{
    if(tiles_tab[posToIndex(x, y, z)])
    {
       delete tiles_tab[posToIndex(x, y, z)];
    }
	tiles_tab[posToIndex(x, y, z)] = tile; 
}

Tile* Map::getTile(Position pos)
{
    return getTile(pos.x,pos.y,pos.z);
}

Tile* Map::getTile(unsigned short x, unsigned short y, unsigned char z)
{
    if(tiles_tab[posToIndex(x, y, z)])
    {
       return tiles_tab[posToIndex(x, y, z)];
    }
    return NULL;
}

void Map::load()
{
    clock_t time_started = clock(); 
    coutCurrentTime();
    
    std::string current_tile;
    
    std::cout<<"::Loading world.map...";
    std::ifstream plik_we("Map\\world.map");
	if (plik_we.is_open())
    {
        for(int i = 0; i < 6144000 - 1; i++)
        {
	       if(tiles_tab[i])
	       {
              delete tiles_tab[i];     
           }          
        }        
        while(!plik_we.eof())
        {
            std::string x, y, z;
            current_tile.clear();

			std::getline(plik_we,current_tile);
                    
            current_tile.erase(0, 1);        

            x = current_tile.substr(0, current_tile.find_first_of("Y"));           
            current_tile.erase(0, current_tile.find_first_of("Y")+1);
                       
            y = current_tile.substr(0, current_tile.find_first_of("Z"));                   
            current_tile.erase(0, current_tile.find_first_of("Z")+1);

            z = current_tile.substr(0, current_tile.find_first_of("="));                       
            current_tile.erase(0, current_tile.find_first_of("=")+1);
         
            tiles_tab[posToIndex(atoi(x.c_str()), atoi(y.c_str()), atoi(z.c_str()))] = new Tile(atoi(x.c_str()), atoi(y.c_str()), char(atoi(z.c_str())));
                       
            std::string token, charek;
            token.clear();

			unsigned int itemID = 0;
			unsigned short count = 1;
			bool abilities = false;

            for(unsigned int i = 0; i < current_tile.length(); i++)
            {
                charek = current_tile[i];
                if(charek != "|" && charek != ".")
                {
                    token += charek;
                }
                if(charek == ".")
                {
                    itemID = atoi(token.c_str());
                    token.clear();
                }
                if(charek == "`")
                {
					abilities = true;
					count = atoi(token.c_str());
                    token.clear();
                }
                if(charek == "|")
                {    
					if(abilities)
					{
						Game.createItemOnTile(itemID, count, tiles_tab[posToIndex(atoi(x.c_str()), atoi(y.c_str()), atoi(z.c_str()))]);
						tiles_tab[posToIndex(atoi(x.c_str()), atoi(y.c_str()), atoi(z.c_str()))]->getTopItem()->abilities = new ItemAbilities;	
						tiles_tab[posToIndex(atoi(x.c_str()), atoi(y.c_str()), atoi(z.c_str()))]->getTopItem()->abilities->parse(token);
					}
					else
						Game.createItemOnTile(itemID, char(atoi(token.c_str())), tiles_tab[posToIndex(atoi(x.c_str()), atoi(y.c_str()), atoi(z.c_str()))]);

                    token.clear();                    
                }    
            }
        }
        std::cout<<" Done!";
    }
    else
    {
        coutCurrentTime();
        std::cout << "\n::Error while opening world.map\n";
    }
	plik_we.close();
	std::cout<<" ( " << clock() - time_started << " ms )\n";

	time_started = clock(); 
    coutCurrentTime();
    std::cout<<"::Loading tilesProperties.txt...";
    std::ifstream tilesProperties("Map\\tilesProperties.txt");
	if (tilesProperties.is_open())
    {
		std::string current_tile;
        while(!tilesProperties.eof())
        {
            std::string x, y, z;
            current_tile.clear();

			std::getline(tilesProperties,current_tile);
                    
            current_tile.erase(0, 1);        

            x = current_tile.substr(0, current_tile.find_first_of("Y"));           
            current_tile.erase(0, current_tile.find_first_of("Y")+1);
                       
            y = current_tile.substr(0, current_tile.find_first_of("Z"));                   
            current_tile.erase(0, current_tile.find_first_of("Z")+1);

            z = current_tile.substr(0, current_tile.find_first_of("="));                       
            current_tile.erase(0, current_tile.find_first_of("=")+1);
         
			Tile *tile = tiles_tab[posToIndex(atoi(x.c_str()), atoi(y.c_str()), atoi(z.c_str()))];
                       
			if(tile && current_tile.size() > 0)
			{
				tile->type = atoi(current_tile.c_str());
			}
		}
        std::cout<<" Done!";
    }
    else
    {
        coutCurrentTime();
        std::cout << "\n::Error while opening tilesProperties.txt\n";
    }
	tilesProperties.close();
	
    std::cout<<" ( " << clock() - time_started << " ms )\n";
}

void Map::getSpectators(unsigned int posx, unsigned int posy, unsigned char posz, std::deque<Creature*> &monsters)
{ 	
   unsigned char z1, z2;
   
   if(posz >= 7)
   {
      z1 = 6;
      z2 = 14;
   }
   else
   {
      z1 = posz - 1;
      z2 = posz + 2;
      if(z2>7)
	  {
		  z2=7;
	  }
   }
      
   for(z1;z1<=z2;z1++)
   {
      for(unsigned int x = posx - 9; x <= posx + 10; x++)
	  {
         for(unsigned int y = posy - 7; y <= posy + 8; y++)
         {
            if(getTile(x,y,z1) != NULL)
	        {
				if(getTile(x,y,z1)->getCreatures().size() > 0)
				{
					unsigned int i = 0;
					while( i < getTile(x,y,z1)->getCreatures().size())
					{
						monsters.push_back(getTile(x,y,z1)->getCreatures().at(0));
						i++;
					}
				}
			}
		}
	  }
   }
}

void Map::getFloorAreaPlayers(unsigned int posx, unsigned int posy, unsigned char posz, std::deque<Player*> &players)
{ 	     
	for(unsigned int x = posx - 9; x <= posx + 10; x++)
	{
		for(unsigned int y = posy - 7; y <= posy + 8; y++)
		{
			if(getTile(x, y, posz) != NULL)
			{
				if(getTile(x, y, posz)->getCreatures().size() > 0)
				{
					unsigned int i = 0;
					while( i < getTile(x, y, posz)->getCreatures().size())
					{
						if(dynamic_cast<Player*>(getTile(x, y, posz)->getCreatures().at(i)))
							players.push_back(dynamic_cast<Player*>(getTile(x, y, posz)->getCreatures().at(i)));
						i++;
					}
				}
			}
		}
	}
}

void Map::getSpectators(Position& centerPos, std::deque<Creature*> &monsters)
{ 	
	this->getSpectators(centerPos.x, centerPos.y, centerPos.z, monsters);
}
