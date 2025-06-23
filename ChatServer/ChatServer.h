#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include "ClientSession.h"
#include "ChatDataManager.h"

class ChatServer {
public:
    ChatServer();
    ~ChatServer();
    void run(int port);

private:
    SOCKET serverSocket;
    std::vector<ClientSession*> clients;
    std::mutex clientsMutex;
    ChatDataManager dataManager;

    void acceptClients();
    void handleClient(ClientSession* client);
    void broadcast(const nlohmann::json& message, SOCKET excludeSocket);
};