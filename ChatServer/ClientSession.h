#pragma once
#include <winsock2.h>
#include <string>
#include <nlohmann/json.hpp>

class ClientSession {
public:
    ClientSession(SOCKET socket);
    ~ClientSession();

    void setID(int id) { clientID = id; }
    int getID() { return clientID; }

    bool receiveJson(nlohmann::json& outJson);
    bool sendJson(const nlohmann::json& json);
    SOCKET getSocket() const;

private:
    int clientID;
    SOCKET clientSocket;
};