#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <string>

class CChatDlg;

class ChatManager
{
public:
	ChatManager(CChatDlg* dlg);
	~ChatManager();


	void registerAccount(CString name, CString account, CString password);
	void registerAccountAct(std::string str);

	void loginAccount(CString account, CString password);
	void loginAccountAct(std::string str);

	void logoutAccount();

	void searchUser(CString account);
	void searchUserAct(std::string str);

	void addFriend(CString account);
	void addFriendAct(std::string str);

	void deleteFriend(CString account);
	void deleteFriendAct(std::string str);

	void searchFriend();
	void searchFriendAct(std::string str);

	void createRoom(CString name, CArray<CString, CString>& friends);
	void createRoomAct(std::string str);

	void deleteRoom(int id);
	void deleteRoomAct(std::string str);

	void searchRoom();
	void searchRoomAct(std::string str);

	void searchMessage(int id);
	void searchMessage(CString account);
	void searchMessageAct(std::string str);

	void sendMessage(int id, CString message, CString timestamp);
	void receiveMessage(std::string str);
	
	void receive();

private:
	std::thread receiver;
	std::atomic<bool> state{ true };

	SOCKET sock;
	CChatDlg* dlg;
};