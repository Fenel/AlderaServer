#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__

#include <deque>
#include <string>

class Account
{
public:
	Account();
	~Account();

	std::string name;
	std::string password;
	std::string email;
	unsigned int access;
	unsigned int premiumTime;

	std::deque<std::string> charList;
};

#endif 
