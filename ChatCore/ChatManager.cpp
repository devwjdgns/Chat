#include "ChatManager.h"

#pragma comment(lib, "ws2_32.lib")

ChatManager::ChatManager()
{
    std::string ip = "127.0.0.1";
    int port = 12345;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    const SSL_METHOD* method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) 
    {
        std::cerr << "SSL_CTX_new failed\n";
        return;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) 
    {
        std::cerr << "Socket creation failed\n";
        return;
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        int err = WSAGetLastError();
        std::cerr << "Connect failed. WSA error code: " << err << "\n";
        closesocket(sock);
        sock = INVALID_SOCKET;
        return;
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, (int)sock);

    if (SSL_connect(ssl) <= 0) 
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        ssl = nullptr;
        return;
    }

    receiver = std::thread([this]() {
        this->receive();
        });
}

ChatManager::~ChatManager()
{
    state = false;

    if (ssl) 
        SSL_shutdown(ssl);
    
    if (sock != INVALID_SOCKET) 
        shutdown(sock, SD_BOTH);

    if (receiver.joinable())
        receiver.join();

    if (ssl) 
        SSL_free(ssl);

    if (sock != INVALID_SOCKET) 
        closesocket(sock);

    if (ctx) 
        SSL_CTX_free(ctx);

    ERR_free_strings();
    EVP_cleanup();
    WSACleanup();
}

void ChatManager::registerAccount(std::string name, std::string account, std::string password)
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "register";
    j["name"] = name;
    j["account"] = account;
    j["password"] = password;
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
}

void ChatManager::registerAccountAct(std::string str)
{
    try 
    {
        nlohmann::json response = nlohmann::json::parse(str);
        if (response["status"].get<bool>())
        {
            auto data = std::make_shared<StringData>("Account created!");
            if (eventHandler) eventHandler("register", true, data.get());
        }
        else
        {
            auto data = std::make_shared<StringData>("The account already exists!");
            if (eventHandler) eventHandler("register", false, data.get());
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::loginAccount(std::string account, std::string password)
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "login";
    j["account"] = account;
    j["password"] = password;
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
}

void ChatManager::loginAccountAct(std::string str)
{
    try 
    {
        nlohmann::json response = nlohmann::json::parse(str);
        if (response["status"].get<bool>())
        {
            auto data = std::make_shared<StringData>("");
            if (eventHandler) eventHandler("login", true, data.get());
        }
        else
        {
            auto data = std::make_shared<StringData>("Login failed!");
            if (eventHandler) eventHandler("login", false, data.get());
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::logoutAccount()
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "logout";
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
}

void ChatManager::searchUser(std::string account)
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "search_user";
    j["account"] = account;
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
}

void ChatManager::searchUserAct(std::string str)
{
    try
    {
        nlohmann::json response = nlohmann::json::parse(str);
        std::vector<std::string> resultList;
        if (response.contains("users") && response["users"].is_array())
        {
            for (const auto& user : response["users"])
            {
                std::string name = user.value("name", "");
                std::string account = user.value("account", "");
                std::string formatted = name + "(" + account + ")";
                resultList.push_back(formatted);
            }
        }
        auto data = std::make_shared<ListData>(resultList);
        if (eventHandler) eventHandler("search_user", false, data.get());
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::addFriend(std::string account)
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "add_friend";
    j["account"] = account;
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
}

void ChatManager::addFriendAct(std::string str)
{
    try
    {
        nlohmann::json response = nlohmann::json::parse(str);
        if (response["status"].get<bool>())
        {
            auto data = std::make_shared<StringData>("");
            if (eventHandler) eventHandler("add_friend", true, data.get());
        }
        else
        {
            auto data = std::make_shared<StringData>("Add friend failed!");
            if (eventHandler) eventHandler("add_friend", false, data.get());
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::deleteFriend(std::string account)
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "delete_friend";
    j["account"] = account;
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
}

void ChatManager::deleteFriendAct(std::string str)
{
    try
    {
        nlohmann::json response = nlohmann::json::parse(str);
        if (response["status"].get<bool>())
        {
            auto data = std::make_shared<StringData>("");
            if (eventHandler) eventHandler("delete_friend", true, data.get());
        }
        else
        {
            auto data = std::make_shared<StringData>("Delete friend failed!");
            if (eventHandler) eventHandler("delete_friend", false, data.get());
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::searchFriend()
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "search_friend";
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
}

void ChatManager::searchFriendAct(std::string str)
{
    try
    {
        nlohmann::json response = nlohmann::json::parse(str);
        std::vector<std::string> resultList;
        if (response.contains("friends") && response["friends"].is_array())
        {
            for (const auto& user : response["friends"])
            {
                std::string name = user.value("name", "");
                std::string account = user.value("account", "");

                std::string formatted = name + "(" + account + ")";
                resultList.push_back(formatted);
            }
        }
        auto data = std::make_shared<ListData>(resultList);
        if (eventHandler) eventHandler("search_friend", false, data.get());
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::createRoom(std::string name, std::vector<std::string>& accounts)
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "create_room";
    j["name"] = name;
    j["accounts"] = accounts;
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
}

void ChatManager::createRoomAct(std::string str)
{
    try
    {
        nlohmann::json response = nlohmann::json::parse(str);
        if (response["status"].get<bool>())
        {
            auto data = std::make_shared<StringData>(std::to_string(response["id"].get<int>()));
            if (eventHandler) eventHandler("create_room", true, data.get());
        }
        else
        {
            auto data = std::make_shared<StringData>("Create Room failed!");
            if (eventHandler) eventHandler("create_room", false, data.get());
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::deleteRoom(int id)
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "delete_room";
    j["id"] = id;
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
}

void ChatManager::deleteRoomAct(std::string str)
{
    try
    {
        nlohmann::json response = nlohmann::json::parse(str);
        if (response["status"].get<bool>())
        {
            auto data = std::make_shared<StringData>("");
            if (eventHandler) eventHandler("delete_room", true, data.get());
        }
        else
        {
            auto data = std::make_shared<StringData>("Delete Room failed!");
            if (eventHandler) eventHandler("delete_room", false, data.get());
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::searchRoom()
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "search_room";
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
}

void ChatManager::searchRoomAct(std::string str)
{
    try
    {
        nlohmann::json response = nlohmann::json::parse(str);
        std::vector<std::string> resultList;
        if (response.contains("rooms") && response["rooms"].is_array())
        {
            for (const auto& rooms : response["rooms"])
            {
                int id = rooms.value("id", 0);
                std::string name = rooms.value("name", "");

                std::string formatted = "(" + std::to_string(id) + ")" + name;
                resultList.push_back(formatted);
            }
        }
        auto data = std::make_shared<ListData>(resultList);
        if (eventHandler) eventHandler("search_room", false, data.get());
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::searchMessage(int id)
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "search_message";
    j["id"] = id;
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
}

void ChatManager::searchMessage(std::string account)
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "search_message";
    j["account"] = account;
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
}

void ChatManager::searchMessageAct(std::string str)
{
    try
    {
        nlohmann::json response = nlohmann::json::parse(str);
        if (eventHandler) eventHandler("room_info", response["id"], nullptr);
        if (response.contains("messages") && response["messages"].is_array())
        {
            int count = response["messages"].size();
            if (count == 0)
            {
                MessageData* resultList = new MessageData[1];
                if (eventHandler) eventHandler("search_message", count, resultList);
            }
            else
            {
                MessageData* resultList = new MessageData[count];
                for (int i = 0; i < count; i++) {
                    const auto& messages = response["messages"][i];
                    MessageData data(messages.value("name", ""), messages.value("message", ""), messages.value("timestamp", ""));
                    resultList[i] = data;
                }
                if (eventHandler) eventHandler("search_message", count, resultList);
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::sendMessage(int id, std::string message, std::string timestamp)
{
    if (!ssl) return;

    nlohmann::json j;
    j["type"] = "update_message";
    j["id"] = id;
    j["message"] = message;
    j["timestamp"] = timestamp;
    std::string json = j.dump() + "\n";
    SSL_write(ssl, json.c_str(), (int)json.length());
    MessageData* result = new MessageData("", message, timestamp);
    if (eventHandler) eventHandler("update_message", id, result);
}

void ChatManager::receiveMessage(std::string str)
{
    try 
    {
        nlohmann::json response = nlohmann::json::parse(str);
        MessageData* result = new MessageData(response["name"], response["message"], response["timestamp"]);
        if (eventHandler) eventHandler("update_message", response["id"], result);
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << "\n";
    }
}

void ChatManager::receive()
{
    char buffer[1024];
    std::string message;
    while (state && ssl)
    {
        int recvLen = SSL_read(ssl, buffer, sizeof(buffer));
        if (!state) break;

        if (recvLen <= 0) 
        {
            std::cerr << "SSL connection lost\n";
            auto data = std::make_shared<StringData>("Disconnected...");
            if (eventHandler) eventHandler("logout", true, data.get());
            break;
        }
        message.append(buffer, recvLen);

        size_t pos;
        while ((pos = message.find('\n')) != std::string::npos)
        {
            std::string oneMessage = message.substr(0, pos);
            message.erase(0, pos + 1);

            try 
            {
                nlohmann::json response = nlohmann::json::parse(oneMessage);

                if (response.contains("type") && response["type"] == "register") 
                {
                    registerAccountAct(oneMessage);
                }
                else if (response.contains("type") && response["type"] == "login") 
                {
                    loginAccountAct(oneMessage);
                }
                else if (response.contains("type") && response["type"] == "search_user")
                {
                    searchUserAct(oneMessage);
                }
                else if (response.contains("type") && response["type"] == "add_friend")
                {
                    addFriendAct(oneMessage);
                }
                else if (response.contains("type") && response["type"] == "delete_friend") 
                {
                    deleteFriendAct(oneMessage);
                }
                else if (response.contains("type") && response["type"] == "search_friend") 
                {
                    searchFriendAct(oneMessage);
                }
                else if (response.contains("type") && response["type"] == "create_room") 
                {
                    createRoomAct(oneMessage);
                }
                else if (response.contains("type") && response["type"] == "delete_room") 
                {
                    deleteRoomAct(oneMessage);
                }
                else if (response.contains("type") && response["type"] == "search_room") 
                {
                    searchRoomAct(oneMessage);
                }
                else if (response.contains("type") && response["type"] == "search_message") 
                {
                    searchMessageAct(oneMessage);
                }
                else if (response.contains("type") && response["type"] == "update_message")
                {
                    receiveMessage(oneMessage);
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "JSON parse error: " << e.what() << "\n";
            }
        }
    }
}