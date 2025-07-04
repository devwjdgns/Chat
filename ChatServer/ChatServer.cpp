#include "ChatServer.h"
#include <iostream>
#include <algorithm>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

ChatServer::ChatServer() 
{
    WSAStartup(MAKEWORD(2, 2), new WSADATA);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
}

ChatServer::~ChatServer() 
{
    closesocket(serverSocket);
    WSACleanup();

    for (auto client : clients) 
    {
        delete client;
    }
}

void ChatServer::run(int port) 
{
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&addr, sizeof(addr));
    listen(serverSocket, SOMAXCONN);

    std::cout << "Server started on port " << port << "\n";
    acceptClients();
}

void ChatServer::acceptClients() 
{
    while (true) 
    {
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrLen);
        if (clientSocket != INVALID_SOCKET) 
        {
            auto* session = new ClientSession(clientSocket);
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clients.push_back(session);
            }
            std::thread(&ChatServer::handleClient, this, session).detach();
            std::cout << "Client connected.\n";
        }
    }
}

void ChatServer::handleClient(ClientSession* client) 
{
    nlohmann::json j;
    while (client->receiveJson(j)) 
    {
        std::string type = j.value("type", "");
        if (type == "register") 
        {
            bool ret = dataManager.createUser(j["name"], j["account"], j["password"]);
            nlohmann::json response;
            response["type"] = "register";
            response["status"] = ret;
            client->sendJson(response);
        }
        else if (type == "login")
        {
            int id;
            bool ret = dataManager.assertUser(j["account"], j["password"], id);
            nlohmann::json response;
            response["type"] = "login";
            response["status"] = ret;
            if (ret)
            {
                client->setID(id);
            }
            client->sendJson(response);
        }
        else if (type == "search_user")
        {
            std::vector<std::string> accounts;
            std::vector<std::string> names;
            bool ret = dataManager.searchUser(client->getID(), j["account"], accounts, names);
            nlohmann::json response;
            response["type"] = "search_user";
            response["users"] = nlohmann::json::array();
            for (int i = 0; i < accounts.size(); i++)
            {
                response["users"].push_back({
                    {"account", accounts[i]},
                    {"name", names[i]}
                    });
            }
            client->sendJson(response);
        }
        else if (type == "add_friend")
        {
            int id = dataManager.getUserID(j["account"]);
            bool ret = dataManager.createFriend(client->getID(), id);
            nlohmann::json response;
            response["type"] = "add_friend";
            response["status"] = ret;
            client->sendJson(response);
        }
        else if (type == "delete_friend")
        {
            int id = dataManager.getUserID(j["account"]);
            bool ret = dataManager.deleteFriend(client->getID(), id);
            nlohmann::json response;
            response["type"] = "delete_friend";
            response["status"] = ret;
            client->sendJson(response);
        }
        else if (type == "search_friend")
        {
            std::vector<std::string> accounts;
            std::vector<std::string> names;
            bool ret = dataManager.searchFriend(client->getID(), accounts, names);
            nlohmann::json response;
            response["type"] = "search_friend";
            response["friends"] = nlohmann::json::array();
            for (int i = 0; i < accounts.size(); i++)
            {
                response["friends"].push_back({
                    {"account", accounts[i]},
                    {"name", names[i]}
                    });
            }
            client->sendJson(response);
        }
        else if (type == "create_room")
        {
            std::vector<std::string> accounts = j["accounts"];
            std::vector<int> members;
            members.push_back(client->getID());
            for (auto account : accounts)
            {
                members.push_back(dataManager.getUserID(account));
            }
            int id = -1;
            bool ret = dataManager.createRoom(j["name"], members, id);
            nlohmann::json response;
            response["type"] = "create_room";
            response["id"] = id;
            response["status"] = ret;
            client->sendJson(response);
        }
        else if (type == "delete_room")
        {
            bool ret = dataManager.deleteRoom(client->getID(), j["id"]);
            nlohmann::json response;
            response["type"] = "delete_room";
            response["status"] = ret;
            client->sendJson(response);
        }
        else if (type == "search_room")
        {
            std::vector<int> ids;
            std::vector<std::string> names;
            bool ret = dataManager.searchRoom(client->getID(), ids, names);
            nlohmann::json response;
            response["type"] = "search_room";
            response["rooms"] = nlohmann::json::array();
            for (int i = 0; i < ids.size(); i++)
            {
                response["rooms"].push_back({
                    {"id", ids[i]},
                    {"name", names[i]}
                    });
            }
            client->sendJson(response);
        }
        else
        {
            broadcast(j, client->getSocket());
        }
    }

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove_if(clients.begin(), clients.end(),
            [client](ClientSession* c) 
            {
                return c->getSocket() == client->getSocket();
            }), clients.end());
    }

    delete client;
    std::cout << "Client disconnected.\n";
}

void ChatServer::broadcast(const nlohmann::json& message, SOCKET excludeSocket) 
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto client : clients)
    {
        if (client->getSocket() != excludeSocket)
        {
            client->sendJson(message);
        }
    }
}