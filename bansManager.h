#ifndef __BANSMANAGER_H__
#define __BANSMANAGER_H__

#include "enums.h"

struct Ban 
{
	BanType_t type;
	std::string banished_ident;
	time_t added;
	time_t expires;
	std::string admin_ident;
	std::string comment;
};

class BansManager 
{
public:
	BansManager();
	~BansManager();

	void loadBans();
	void saveBans();

	bool isIpBanished(std::string ip) ;
	bool isPlayerBanished(std::string name);
	bool isPlayerBanished(unsigned int id);
	bool isAccountBanished(std::string account);

	void addIpBan(std::string ip, unsigned int _time, std::string admin_ident, std::string comment);
	void addPlayerBan(std::string playerName, unsigned int _time, std::string admin_ident, std::string comment);
	void addAccountBan(std::string accountName, unsigned int _time, std::string admin_ident, std::string comment);

	bool removeIpBans(std::string ip);
	bool removePlayerBans(unsigned int id);
	bool removePlayerBans(std::string playerName);
	bool removeAccountBans(std::string accountName);
private:
	std::deque<Ban> bans;
};

#endif
