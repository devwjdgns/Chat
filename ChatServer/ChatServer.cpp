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
            bool ret = dataManager.checkUser(j["account"], j["password"], id);
            nlohmann::json response;
            response["type"] = "login";
            response["status"] = ret;
            if (ret)
            {
                client->setID(id);
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