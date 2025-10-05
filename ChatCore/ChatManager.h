#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <string>
#include <memory>
#include <functional>

#include <nlohmann/json.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "IData.h"

class __declspec(dllexport) ChatManager
{
public:
	ChatManager();
	~ChatManager();

	void registerAccount(std::string name, std::string account, std::string password);
	void registerAccountAct(std::string str);

	void loginAccount(std::string account, std::string password);
	void loginAccountAct(std::string str);

	void logoutAccount();

	void searchUser(std::string account);
	void searchUserAct(std::string str);

	void addFriend(std::string account);
	void addFriendAct(std::string str);

	void deleteFriend(std::string account);
	void deleteFriendAct(std::string str);

	void searchFriend();
	void searchFriendAct(std::string str);

	void createRoom(std::string name, std::vector<std::string>& friends);
	void createRoomAct(std::string str);

	void deleteRoom(int id);
	void deleteRoomAct(std::string str);

	void searchRoom();
	void searchRoomAct(std::string str);

	void searchMessage(int id);
	void searchMessage(std::string account);
	void searchMessageAct(std::string str);

	void sendMessage(int id, std::string message, std::string timestamp);
	void receiveMessage(std::string str);
	
	void receive();

	using EventHandler = std::function<void(const std::string& type, const int& state, const IData* data)>;
	void setEventHandler(EventHandler handler)
	{
		this->eventHandler = handler;
	}

private:
	std::thread receiver;
	std::atomic<bool> state{ true };

	SSL_CTX* ctx;
	SSL* ssl;
	SOCKET sock;
	EventHandler eventHandler;
};