#include "account.h"

Account::Account()
{
	name = "";
	password = "";
	email = "";
	access = 0;
	premiumTime = 0;
}

Account::~Account()
{
	charList.clear();
}
