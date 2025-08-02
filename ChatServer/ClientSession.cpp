#include "ClientSession.h"
#include <iostream>

ClientSession::ClientSession(SOCKET socket) : clientSocket(socket), clientID(-1) {}

ClientSession::~ClientSession() 
{
    closesocket(clientSocket);
}

bool ClientSession::receiveJson(nlohmann::json& outJson) 
{
    char buffer[1024];
    int len = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) return false;
    buffer[len] = '\0';

    try 
    {
        outJson = nlohmann::json::parse(buffer);
        return true;
    }
    catch (std::exception& e) 
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        return false;
    }
}

bool ClientSession::sendJson(const nlohmann::json& json) 
{
    std::string message = json.dump();
    int sent = send(clientSocket, message.c_str(), message.size(), 0);
    return sent != SOCKET_ERROR;
}

SOCKET ClientSession::getSocket() const 
{
    return clientSocket;
}