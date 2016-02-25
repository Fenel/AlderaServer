#ifndef __accountsManager_H__
#define __accountsManager_H__

#include <string>
#include "account.h"


class AccountsManager
{
 public:
	AccountsManager() {}
	~AccountsManager() {}

	Account loadAccount(std::string& name);
	bool saveAccount(Account &account);
	bool getPassword(std::string& name, std::string& password);
};

#endif
