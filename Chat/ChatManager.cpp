#include "pch.h"
#include "framework.h"
#include "ChatManager.h"
#include "ChatDlg.h"
#include "FriendDlg.h"
#include "utility.h"

#pragma comment(lib, "ws2_32.lib")

ChatManager::ChatManager(CChatDlg* dlg): dlg(dlg)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    receiver = std::thread([this]() {
        this->receive();
        });
}

ChatManager::~ChatManager()
{
    state = false;

    closesocket(sock);

    if (receiver.joinable())
        receiver.join();

    WSACleanup();
}

void ChatManager::registerAccount(CString name, CString account, CString password)
{
    nlohmann::json j;
    j["type"] = "register";
    j["name"] = convertString(name);
    j["account"] = convertString(account);
    j["password"] = convertString(password);
    std::string json = j.dump();
    send(sock, json.c_str(), json.length(), 0);
}

void ChatManager::registerAccountAct(std::string str)
{
    try 
    {
        nlohmann::json response = nlohmann::json::parse(str);
        if (response["status"].get<bool>())
        {
            CString* result = new CString(_T("Account created!"));
            ::PostMessage(dlg->GetSafeHwnd(), WM_REGISTER_ACTION, TRUE, reinterpret_cast<LPARAM>(result));
            return;
        }
        else
        {
            CString* result = new CString(_T("The account already exists!"));
            ::PostMessage(dlg->GetSafeHwnd(), WM_REGISTER_ACTION, FALSE, reinterpret_cast<LPARAM>(result));
            return;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::loginAccount(CString account, CString password)
{
    nlohmann::json j;
    j["type"] = "login";
    j["account"] = convertString(account);
    j["password"] = convertString(password);
    std::string json = j.dump();
    send(sock, json.c_str(), json.length(), 0);
}

void ChatManager::loginAccountAct(std::string str)
{
    try 
    {
        nlohmann::json response = nlohmann::json::parse(str);
        if (response["status"].get<bool>())
        {
            CString* result = new CString(_T(""));
            ::PostMessage(dlg->GetSafeHwnd(), WM_LOGIN_ACTION, TRUE, reinterpret_cast<LPARAM>(result));
            return;
        }
        else
        {
            CString* result = new CString(_T("Login failed!"));
            ::PostMessage(dlg->GetSafeHwnd(), WM_LOGIN_ACTION, FALSE, reinterpret_cast<LPARAM>(result));
            return;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::searchUser(CString account)
{
    nlohmann::json j;
    j["type"] = "search_user";
    j["account"] = convertString(account);
    std::string json = j.dump();
    send(sock, json.c_str(), json.length(), 0);
}

void ChatManager::searchUserAct(std::string str)
{
    try
    {
        nlohmann::json response = nlohmann::json::parse(str);
        std::vector<CString> resultList;
        if (response.contains("users") && response["users"].is_array())
        {
            for (const auto& user : response["users"])
            {
                std::string name = user.value("name", "");
                std::string account = user.value("account", "");

                CString formatted;
                formatted.Format(_T("%s(%s)"), convertString(name), convertString(account));
                resultList.push_back(formatted);
            }
        }

        int count = static_cast<int>(resultList.size());
        CString* result = new CString[count];

        for (int i = 0; i < count; ++i)
        {
            result[i] = resultList[i];
        }

        ::PostMessage(dlg->GetFriendDlg()->GetSafeHwnd(), WM_SEARCH_USER_ACTION, (WPARAM)count, reinterpret_cast<LPARAM>(result));
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::addFriend(CString account)
{
    nlohmann::json j;
    j["type"] = "add_friend";
    j["account"] = convertString(account);
    std::string json = j.dump();
    send(sock, json.c_str(), json.length(), 0);
}

void ChatManager::addFriendAct(std::string str)
{
    try
    {
        nlohmann::json response = nlohmann::json::parse(str);
        if (response["status"].get<bool>())
        {
            CString* result = new CString(_T(""));
            ::PostMessage(dlg->GetFriendDlg()->GetSafeHwnd(), WM_ADD_FRIEND_ACTION, TRUE, reinterpret_cast<LPARAM>(result));
            return;
        }
        else
        {
            CString* result = new CString(_T("Add friend failed!"));
            ::PostMessage(dlg->GetFriendDlg()->GetSafeHwnd(), WM_ADD_FRIEND_ACTION, FALSE, reinterpret_cast<LPARAM>(result));
            return;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::deleteFriend(CString account)
{
    nlohmann::json j;
    j["type"] = "delete_friend";
    j["account"] = convertString(account);
    std::string json = j.dump();
    send(sock, json.c_str(), json.length(), 0);
}

void ChatManager::deleteFriendAct(std::string str)
{
    try
    {
        nlohmann::json response = nlohmann::json::parse(str);
        if (response["status"].get<bool>())
        {
            CString* result = new CString(_T(""));
            ::PostMessage(dlg->GetSafeHwnd(), WM_DELETE_FRIEND_ACTION, TRUE, reinterpret_cast<LPARAM>(result));
            return;
        }
        else
        {
            CString* result = new CString(_T("Delete friend failed!"));
            ::PostMessage(dlg->GetSafeHwnd(), WM_DELETE_FRIEND_ACTION, FALSE, reinterpret_cast<LPARAM>(result));
            return;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::searchFriend()
{
    nlohmann::json j;
    j["type"] = "search_friend";
    std::string json = j.dump();
    send(sock, json.c_str(), json.length(), 0);
}

void ChatManager::searchFriendAct(std::string str)
{
    try
    {
        nlohmann::json response = nlohmann::json::parse(str);
        std::vector<CString> resultList;
        if (response.contains("users") && response["users"].is_array())
        {
            for (const auto& user : response["users"])
            {
                std::string name = user.value("name", "");
                std::string account = user.value("account", "");

                CString formatted;
                formatted.Format(_T("%s(%s)"), convertString(name), convertString(account));
                resultList.push_back(formatted);
            }
        }

        int count = static_cast<int>(resultList.size());
        CString* result = new CString[count];

        for (int i = 0; i < count; ++i)
        {
            result[i] = resultList[i];
        }
        ::PostMessage(dlg->GetSafeHwnd(), WM_SEARCH_FRIEND_ACTION, (WPARAM)count, reinterpret_cast<LPARAM>(result));
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::sendMessage(CString time, CString chat)
{
    nlohmann::json j;
    j["type"] = "message";
    j["time"] = convertString(time);
    j["chat"] = convertString(chat);
    std::string json = j.dump();
    send(sock, json.c_str(), json.length(), 0);
    dlg->SendChatMessage(time, chat);
}

void ChatManager::receiveMessage(std::string str)
{
    try 
    {
        nlohmann::json response = nlohmann::json::parse(str);
        CString* time = new CString(convertString(response["time"]));
        CString* chat = new CString(convertString(response["chat"]));
        ::PostMessage(dlg->GetSafeHwnd(), WM_MESSAGE_RECEIVED, reinterpret_cast<LPARAM>(time), reinterpret_cast<LPARAM>(chat));
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::receive()
{
    char buffer[1024];
    while (state) 
    {
        int recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (recvLen <= 0)
        {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET)
            {
                std::cerr << "Socket creation failed\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            sockaddr_in serverAddr = {};
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(12345);
            inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

            if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
            {
                closesocket(sock);
                sock = INVALID_SOCKET;
                std::cerr << "Connect failed, retrying...\n";
                CString* result = new CString(_T("Disconnected..."));
                ::PostMessage(dlg->GetSafeHwnd(), WM_LOGOUT_ACTION, TRUE, reinterpret_cast<LPARAM>(result));
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            continue;
        }
        buffer[recvLen] = '\0';
        std::string message(buffer);
        try 
        {
            nlohmann::json response = nlohmann::json::parse(message);
            if (response.contains("type") && response["type"] == "register")
            {
                registerAccountAct(message);
            }
            else if (response.contains("type") && response["type"] == "login")
            {
                loginAccountAct(message);
            }
            else if (response.contains("type") && response["type"] == "message")
            {
                receiveMessage(message);
            }
            else if (response.contains("type") && response["type"] == "search_user")
            {
                searchUserAct(message);
            }
            else if (response.contains("type") && response["type"] == "add_friend")
            {
                addFriendAct(message);
            }
            else if (response.contains("type") && response["type"] == "delete_friend")
            {
                deleteFriendAct(message);
            }
            else if (response.contains("type") && response["type"] == "search_friend")
            {
                searchFriendAct(message);
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "JSON parse error: " << e.what() << "\n";
        }
    }
}