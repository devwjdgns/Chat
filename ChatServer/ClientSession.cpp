#include "ClientSession.h"
#include <iostream>

ClientSession::ClientSession(SOCKET socket, SSL* ssl) : clientSocket(socket), clientSsl(ssl), clientID(-1) {}

ClientSession::~ClientSession() 
{
    if (clientSsl) 
    {
        SSL_shutdown(clientSsl);
        SSL_free(clientSsl);
        clientSsl = nullptr;
    }

    if (clientSocket != INVALID_SOCKET) 
    {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
}

bool ClientSession::receiveJson(nlohmann::json& json)
{
    std::string message;
    char buffer[1024];
    while (true) 
    {
        int n = SSL_read(clientSsl, buffer, sizeof(buffer));
        if (n <= 0) 
        {
            int err = SSL_get_error(clientSsl, n);
            if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) continue;
            return false;
        }
        message.append(buffer, buffer + n);
        auto pos = message.find('\n');
        if (pos != std::string::npos) 
        {
            std::string line = message.substr(0, pos);
            message.erase(0, pos + 1);
            try 
            {
                json = nlohmann::json::parse(line);
                return true;
            }
            catch (...) 
            {
                continue;
            }
        }
    }
}

bool ClientSession::sendJson(const nlohmann::json& json) 
{
    std::string message = json.dump() + "\n";
    int left = (int)message.size();
    const char* p = message.c_str();

    while (left > 0) 
    {
        int n = SSL_write(clientSsl, p, left);
        if (n <= 0) 
        {
            int err = SSL_get_error(clientSsl, n);
            if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) continue;
            return false;
        }
        left -= n;
        p += n;
    }
    return true;
}

SOCKET ClientSession::getSocket() const 
{
    return clientSocket;
}