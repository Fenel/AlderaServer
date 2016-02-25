#ifndef _CONFIG_MANAGER_H
#define _CONFIG_MANAGER_H

#include <string>
#include "enums.h"

class ConfigManager 
{
public:
	ConfigManager();
	~ConfigManager(){};
	bool loadFile();

    // CONNECTION
	uint16_t port;

	//PVP & PVM SYSTEM
	bool pvp;
	uint16_t pzLock;
	uint16_t huntingLock;
	uint16_t protectionLevel;
	bool noDamageToPartyMembers;

	//SAVE SYSTEM
	bool save;
    uint32_t saveInterval;

	//OTHER
	uint16_t maxPlayers;
	uint16_t playerActionInterval;
	uint16_t checkConnectionInterval;
private:
	bool m_isLoaded;
};


#endif /* _CONFIG_MANAGER_H */