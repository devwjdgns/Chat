#include "ChatDataManager.h"

#include <format>
#include <iostream>

ChatDataManager::ChatDataManager()
{
    if (sqlite3_open("chat.db", &db) != SQLITE_OK) 
    {
        std::cerr << "database open fail...\n";
    }
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    createUserTable();
    createFriendTable();
    createRoomTable();
    createMessageTable();
}

ChatDataManager::~ChatDataManager()
{
    sqlite3_close(db);
}

bool ChatDataManager::createUserTable() 
{
    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "account TEXT NOT NULL UNIQUE, "
        "password TEXT NOT NULL);";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) 
    {
        std::cerr << "Create table error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool ChatDataManager::createFriendTable()
{
    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS friends("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER NOT NULL, "
        "friend_id INTEGER NOT NULL, "
        "UNIQUE(user_id, friend_id), "
        "FOREIGN KEY(user_id) REFERENCES users(id), "
        "FOREIGN KEY(friend_id) REFERENCES users(id));";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Create table error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool ChatDataManager::createRoomTable()
{
    {
        const char* createTableSQL =
            "CREATE TABLE IF NOT EXISTS rooms("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT);";

        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Create table error: " << errMsg << "\n";
            sqlite3_free(errMsg);
            return false;
        }
    }
    {
        const char* createTableSQL =
            "CREATE TABLE IF NOT EXISTS room_members("
            "room_id INTEGER NOT NULL, "
            "user_id INTEGER NOT NULL, "
            "PRIMARY KEY(room_id, user_id), "
            "FOREIGN KEY(room_id) REFERENCES rooms(id) ON DELETE CASCADE, "
            "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE);";

        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Create table error: " << errMsg << "\n";
            sqlite3_free(errMsg);
            return false;
        }
    }
    return true;
}

bool ChatDataManager::createMessageTable()
{
    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS messages("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "room_id INTEGER NOT NULL, "
        "sender_id INTEGER NOT NULL, "
        "message TEXT NOT NULL, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY(room_id) REFERENCES rooms(id) ON DELETE CASCADE, "
        "FOREIGN KEY(sender_id) REFERENCES users(id) ON DELETE SET NULL);";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Create table error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool ChatDataManager::createUser(const std::string& name, const std::string& account, const std::string& password)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL = "INSERT INTO users (name, account, password) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK) 
    {
        std::cerr << "Prepare failed\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, account.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, password.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "Insert failed\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool ChatDataManager::assertUser(const std::string& account, const std::string& password, int& client_id)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL = "SELECT id FROM users WHERE account = ? AND password = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK) 
    {
        std::cerr << "Prepare failed\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, account.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        client_id = sqlite3_column_int(stmt, 0);
        result = true;
    }
    sqlite3_finalize(stmt);
    return result;
}

bool ChatDataManager::searchUser(const int& client_id, const std::string& target, std::vector<std::string>& accounts, std::vector<std::string>& names)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL = "SELECT account, name FROM users WHERE id != ? "
        "AND id NOT IN (SELECT friend_id FROM friends WHERE user_id = ?) " 
        "AND account LIKE ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Search failed\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);
    sqlite3_bind_int(stmt, 2, client_id);
    std::string pattern = "%" + std::string(target.c_str()) + "%";
    sqlite3_bind_text(stmt, 3, pattern.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char* found_account = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* found_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (found_account && found_name)
        {
            accounts.push_back(reinterpret_cast<const char*>(found_account));
            names.push_back(reinterpret_cast<const char*>(found_name));
        }
    }
    sqlite3_finalize(stmt);
    return true;
}

bool ChatDataManager::createFriend(const int& client_id, const int& friend_id)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL = "INSERT INTO friends (user_id, friend_id) VALUES (?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Prepare failed\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);
    sqlite3_bind_int(stmt, 2, friend_id);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "Insert failed\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_reset(stmt);

    sqlite3_bind_int(stmt, 1, friend_id);
    sqlite3_bind_int(stmt, 2, client_id);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "Insert failed\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool ChatDataManager::deleteFriend(const int& client_id, const int& friend_id)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL = "DELETE FROM friends WHERE user_id = ? AND friend_id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Prepare failed\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);
    sqlite3_bind_int(stmt, 2, friend_id);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "Insert failed\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_reset(stmt);

    sqlite3_bind_int(stmt, 1, friend_id);
    sqlite3_bind_int(stmt, 2, client_id);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "Insert failed\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool ChatDataManager::searchFriend(const int& client_id, std::vector<std::string>& accounts, std::vector<std::string>& names)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL = "SELECT account, name FROM users WHERE id IN (SELECT friend_id FROM friends WHERE user_id = ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Search failed\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char* found_account = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* found_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (found_account && found_name)
        {
            accounts.push_back(reinterpret_cast<const char*>(found_account));
            names.push_back(reinterpret_cast<const char*>(found_name));
        }
    }
    sqlite3_finalize(stmt);
    return true;
}

bool ChatDataManager::createRoom(const std::string& name, const std::vector<int>& members, int& room_id)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL = "INSERT INTO rooms (name) VALUES (?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Prepare failed\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "Insert failed\n";
        sqlite3_finalize(stmt);
        return false;
    }

    room_id = (int)sqlite3_last_insert_rowid(db);

    sqlite3_finalize(stmt);

    const char* insertSQL = "INSERT INTO room_members (room_id, user_id) VALUES (?, ?);";

    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Prepare insert member failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    char* errMsg = nullptr;
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "Failed to begin transaction: " << errMsg << "\n";
        sqlite3_free(errMsg);
        sqlite3_finalize(stmt);
        return false;
    }

    for (const auto& memberId : members)
    {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);

        sqlite3_bind_int(stmt, 1, room_id);
        sqlite3_bind_int(stmt, 2, memberId);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            std::cerr << "Insert member failed\n";

            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            sqlite3_finalize(stmt);
            return false;
        }
    }

    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "Failed to commit transaction: " << errMsg << "\n";
        sqlite3_free(errMsg);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    return true;
}

bool ChatDataManager::deleteRoom(const int& client_id, const int& room_id)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt;

    const char* deleteMemberSQL = "DELETE FROM room_members WHERE room_id = ? AND user_id = ?;";
    if (sqlite3_prepare_v2(db, deleteMemberSQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Prepare delete member failed\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, room_id);
    sqlite3_bind_int(stmt, 2, client_id);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "Delete member failed\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    const char* countSQL = "SELECT COUNT(*) FROM room_members WHERE room_id = ?;";
    if (sqlite3_prepare_v2(db, countSQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Prepare count failed\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, room_id);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        count = sqlite3_column_int(stmt, 0);
    }
    else
    {
        std::cerr << "Count query failed\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    if (count == 0)
    {
        const char* deleteRoomSQL = "DELETE FROM rooms WHERE id = ?;";
        if (sqlite3_prepare_v2(db, deleteRoomSQL, -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "Prepare delete room failed\n";
            return false;
        }

        sqlite3_bind_int(stmt, 1, room_id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            std::cerr << "Delete room failed\n";
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);
    }

    return true;
}

bool ChatDataManager::searchRoom(const int& client_id, std::vector<int>& ids, std::vector<std::string>& names)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL = "SELECT r.id, r.name FROM room_members rm "
        "JOIN rooms r ON rm.room_id = r.id "
        "WHERE rm.user_id = ? AND r.name IS NOT NULL AND r.name != '';";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK) 
    {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        if (name)
        {
            ids.push_back(id);
            names.push_back(reinterpret_cast<const char*>(name));
        }
    }

    sqlite3_finalize(stmt);

    return true;
}

bool ChatDataManager::createMessage(const int& client_id, const int& room_id, const std::string& message, const std::string& timestamp)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL = "INSERT INTO messages (room_id, sender_id, message, created_at) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK) 
    {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, room_id);
    sqlite3_bind_int(stmt, 2, client_id);
    sqlite3_bind_text(stmt, 3, message.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, timestamp.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) 
    {
        std::cerr << "Insert failed: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool ChatDataManager::searchMessage(const int& client_id, const int& room_id, std::vector<std::string>& names, std::vector<std::string>& messages, std::vector<std::string>& timestamps)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL =
        "SELECT CASE WHEN users.id = ? THEN '' ELSE users.name END AS name, "
        "messages.message, messages.created_at "
        "FROM messages "
        "JOIN users ON messages.sender_id = users.id "
        "WHERE messages.room_id = ? "
        "ORDER BY messages.created_at ASC;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, client_id);
    sqlite3_bind_int(stmt, 2, room_id);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char* name = sqlite3_column_text(stmt, 0);
        const unsigned char* message = sqlite3_column_text(stmt, 1);
        const unsigned char* created = sqlite3_column_text(stmt, 2);

        if (name && message && created)
        {
            names.push_back(reinterpret_cast<const char*>(name));
            messages.push_back(reinterpret_cast<const char*>(message));
            timestamps.push_back(reinterpret_cast<const char*>(created));
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

int ChatDataManager::getUserID(const std::string& account)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL = "SELECT id FROM users WHERE account = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Prepare failed\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, account.c_str(), -1, SQLITE_TRANSIENT);

    int result = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        result = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return result;
}

int ChatDataManager::getRoomID(const int& client_id, const int& friend_id)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL =
        "SELECT room_id "
        "FROM room_members "
        "GROUP BY room_id "
        "HAVING COUNT(*) = 2 AND "
        "SUM(user_id = ?) = 1 AND "
        "SUM(user_id = ?) = 1;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return -1;
    }

    sqlite3_bind_int(stmt, 1, client_id);
    sqlite3_bind_int(stmt, 2, friend_id);

    int room_id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        room_id = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return room_id;
}

std::vector<int> ChatDataManager::getMembersID(const int& room_id)
{
    std::lock_guard<std::mutex> lock(db_mutex);

    const char* sql = "SELECT user_id FROM room_members WHERE room_id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) 
    {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return std::vector<int>();
    }

    sqlite3_bind_int(stmt, 1, room_id);

    std::vector<int> members;
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        int id = sqlite3_column_int(stmt, 0);
        members.push_back(id);
    }

    sqlite3_finalize(stmt);
    return members;
}

std::string ChatDataManager::getUserName(const int& client_id)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL = "SELECT name FROM users WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Prepare failed\n";
        return "";
    }

    sqlite3_bind_int(stmt, 1, client_id);

    std::string result;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char* name = sqlite3_column_text(stmt, 0);

        if (name)
        {
            result = reinterpret_cast<const char*>(name);
        }
    }
    sqlite3_finalize(stmt);
    return result;
}
