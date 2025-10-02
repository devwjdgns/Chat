#include "ChatServer.h"
#include <iostream>
#include <algorithm>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

ChatServer::ChatServer() 
{
    WSAStartup(MAKEWORD(2, 2), new WSADATA);

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    const SSL_METHOD* method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) 
    {
        std::cerr << "SSL_CTX_new failed\n";
        return;
    }

    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);

    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) 
    {
        std::cerr << "Load certificate failed\n";
        ERR_print_errors_fp(stderr);
        return;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) 
    {
        std::cerr << "Load private key failed\n";
        ERR_print_errors_fp(stderr);
        return;
    }
    if (!SSL_CTX_check_private_key(ctx)) 
    {
        std::cerr << "Private key does not match the certificate public key\n";
        return;
    }

    serverSocket = INVALID_SOCKET;
}

ChatServer::~ChatServer() 
{
    if (serverSocket != INVALID_SOCKET) 
    {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    if (ctx)
    {
        SSL_CTX_free(ctx);
        ctx = nullptr;
    }
    EVP_cleanup();
    WSACleanup();
}

void ChatServer::run(std::string ip, int port)
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) 
    {
        std::cerr << "socket() failed\n";
        return;
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid ip address\n";
        return;
    }

    if (bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) 
    {
        std::cerr << "bind() failed\n";
        return;
    }
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) 
    {
        std::cerr << "listen() failed\n";
        return;
    }

    std::cout << "Server started on " << ip << ":" << port << "\n";
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
            SSL* ssl = SSL_new(ctx);
            if (!ssl)
            {
                closesocket(clientSocket);
                continue;
            }
            SSL_set_fd(ssl, (int)clientSocket);
            
            if (SSL_accept(ssl) <= 0)
            {
                std::cerr << "SSL_accept failed\n";
                ERR_print_errors_fp(stderr);
                SSL_free(ssl);
                closesocket(clientSocket);
                continue;
            }

            std::cout << "Client connected with " << SSL_get_cipher(ssl) << " encryption\n";


            auto session = std::make_unique<ClientSession>(clientSocket, ssl);
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clients.push_back(std::move(session));
            }
            std::thread(&ChatServer::handleClient, this, clients.back().get()).detach();
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
        else if (type == "logout")
        {
            client->setID(-1);
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

            std::lock_guard<std::mutex> lock(clientsMutex);
            for (const auto& c : clients)
            {
                if (c->getID() == id)
                {
                    std::vector<std::string> accounts;
                    std::vector<std::string> names;
                    bool ret = dataManager.searchFriend(c->getID(), accounts, names);
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
                    c->sendJson(response);
                    break;
                }
            }
        }
        else if (type == "delete_friend")
        {
            int id = dataManager.getUserID(j["account"]);
            bool ret = dataManager.deleteFriend(client->getID(), id);
            nlohmann::json response;
            response["type"] = "delete_friend";
            response["status"] = ret;
            client->sendJson(response);

            std::lock_guard<std::mutex> lock(clientsMutex);
            for (const auto& c : clients)
            {
                if (c->getID() == id)
                {
                    std::vector<std::string> accounts;
                    std::vector<std::string> names;
                    bool ret = dataManager.searchFriend(c->getID(), accounts, names);
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
                    c->sendJson(response);
                    break;
                }
            }
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
            members.erase(std::remove(members.begin(), members.end(), client->getID()), members.end());

            std::lock_guard<std::mutex> lock(clientsMutex);
            for (const auto& c : clients)
            {
                auto it = std::find(members.begin(), members.end(), c->getID());
                if (it != members.end())
                {
                    std::vector<int> ids;
                    std::vector<std::string> names;
                    bool ret = dataManager.searchRoom(c->getID(), ids, names);
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
                    c->sendJson(response);
                }
            }
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
        else if (type == "update_message")
        {
            j["name"] = dataManager.getUserName(client->getID());
            std::vector<int> members;
            bool ret = dataManager.createMessage(client->getID(), j["id"], j["message"], j["timestamp"]);
            members = dataManager.getMembersID(j["id"]);
            members.erase(std::remove(members.begin(), members.end(), client->getID()), members.end());

            std::lock_guard<std::mutex> lock(clientsMutex);
            for (const auto& c : clients)
            {
                auto it = std::find(members.begin(), members.end(), c->getID());
                if (it != members.end()) 
                {
                    c->sendJson(j);
                }
            }
        }
        else if (type == "search_message")
        {
            std::vector<std::string> names;
            std::vector<std::string> messages;
            std::vector<std::string> timestamps;
            int id = -1;
            if (j.contains("id"))
            {
                id = j["id"];
            }
            else if (j.contains("account"))
            {
                id = dataManager.getRoomID(client->getID(), dataManager.getUserID(j["account"]));
                if (id == -1)
                {
                    dataManager.createRoom("", { client->getID(), dataManager.getUserID(j["account"]) }, id);
                }
            }
            bool ret = dataManager.searchMessage(client->getID(), id, names, messages, timestamps);

            nlohmann::json response;
            response["type"] = "search_message";
            response["id"] = id;
            response["messages"] = nlohmann::json::array();
            for (int i = 0; i < names.size(); i++)
            {
                response["messages"].push_back({
                    {"name", names[i]},
                    {"message", messages[i]},
                    {"timestamp", timestamps[i]}
                    });
            }
            client->sendJson(response);
        }
    }
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove_if(clients.begin(), clients.end(),
            [client](const std::unique_ptr<ClientSession>& c)
            {
                return c->getSocket() == client->getSocket();
            }), clients.end());
    }

    std::cout << "Client disconnected.\n";
}