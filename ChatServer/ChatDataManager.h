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

	bool createMessage(const int& client_id, const int& room_id, const std::string& message, const std::string& timestamp);
	bool searchMessage(const int& client_id, const int& room_id, std::vector<std::string>& names, std::vector<std::string>& messages, std::vector<std::string>& timestamps);

	int getUserID(const std::string& account);
	int getRoomID(const int& client_id, const int& friend_id);
	std::vector<int> getMembersID(const int& room_id);
	std::string getUserName(const int& client_id);

private:
	sqlite3* db;
	std::mutex db_mutex;
};