#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include <mutex>

class ChatDataManager
{
public:
	ChatDataManager();
	~ChatDataManager();

	bool createUserTable();
	bool createUser(const std::string& name, const std::string& account, const std::string& password);
	bool checkUser(const std::string& account, const std::string& password, int& id);
	bool searchUser(const std::string& account, std::vector<std::string>& accounts, std::vector<std::string>& names);

private:
	sqlite3* db;
	std::mutex db_mutex;
};