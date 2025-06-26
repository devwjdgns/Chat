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

	void searchFriend(CString account);
	void searchFriendAct(std::string str);

	void sendMessage(CString time, CString chat);
	void receiveMessage(std::string str);
	
	void receive();

private:
	SOCKET sock;
	CChatDlg* dlg;
};