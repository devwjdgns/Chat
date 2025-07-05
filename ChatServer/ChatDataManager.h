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
	bool createFriendTable();
	bool createRoomTable();
	bool createMessageTable();

	bool createUser(const std::string& name, const std::string& account, const std::string& password);
	bool assertUser(const std::string& account, const std::string& password, int& client_id);
	bool searchUser(const int& client_id, const std::string& target, std::vector<std::string>& accounts, std::vector<std::string>& names);

	bool createFriend(const int& client_id, const int& friend_id);
	bool deleteFriend(const int& client_id, const int& friend_id);
	bool searchFriend(const int& client_id, std::vector<std::string>& accounts, std::vector<std::string>& names);

	bool createRoom(const std::string& name, const std::vector<int>& members, int& room_id);
	bool deleteRoom(const int& client_id, const int& room_id);
	bool searchRoom(const int& client_id, std::vector<int>& ids, std::vector<std::string>& names);

	int getUserID(const std::string& account);
private:
	sqlite3* db;
	std::mutex db_mutex;
};