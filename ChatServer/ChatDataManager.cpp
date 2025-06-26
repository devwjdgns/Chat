#include "ChatDataManager.h"

#include <format>
#include <iostream>

ChatDataManager::ChatDataManager()
{
    if (sqlite3_open("chat.db", &db) != SQLITE_OK) 
    {
        std::cerr << "database open fail...\n";
    }

    createUserTable();
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

bool ChatDataManager::checkUser(const std::string& account, const std::string& password, int& id)
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
        id = sqlite3_column_int(stmt, 0);
        result = true;
    }
    sqlite3_finalize(stmt);
    return result;
}

bool ChatDataManager::searchUser(const std::string& account, std::vector<std::string>& accounts, std::vector<std::string>& names)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    const char* SQL = "SELECT account, name FROM users WHERE account LIKE ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, SQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Search failed\n";
        return false;
    }

    std::string pattern = "%" + std::string(account.c_str()) + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);

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