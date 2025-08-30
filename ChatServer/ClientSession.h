#pragma once
#include <winsock2.h>
#include <string>
#include <nlohmann/json.hpp>

#include <openssl/ssl.h>
#include <openssl/err.h>

class ClientSession {
public:
    ClientSession(SOCKET socket, SSL* ssl);
    ~ClientSession();

    void setID(int id) { clientID = id; }
    int getID() { return clientID; }

    bool receiveJson(nlohmann::json& json);
    bool sendJson(const nlohmann::json& json);
    SOCKET getSocket() const;

private:
    int clientID;
    SSL* clientSsl;
    SOCKET clientSocket;
};