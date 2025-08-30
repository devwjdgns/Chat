#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <winsock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "ClientSession.h"
#include "ChatDataManager.h"

class ChatServer {
public:
    ChatServer();
    ~ChatServer();
    void run(std::string ip, int port);

private:
    void acceptClients();
    void handleClient(ClientSession* client);

private:
    SOCKET serverSocket;
    std::vector<std::unique_ptr<ClientSession>> clients;
    std::mutex clientsMutex;
    ChatDataManager dataManager;
    SSL_CTX* ctx;
};