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

	void searchUser(CString account);
	void searchUserAct(std::string str);

	void addFriend(CString account);
	void addFriendAct(std::string str);

	void deleteFriend(CString account);
	void deleteFriendAct(std::string str);

	void searchFriend();
	void searchFriendAct(std::string str);

	void sendMessage(CString time, CString chat);
	void receiveMessage(std::string str);
	
	void receive();

private:
	std::thread receiver;
	std::atomic<bool> state{ true };

	SOCKET sock;
	CChatDlg* dlg;
};