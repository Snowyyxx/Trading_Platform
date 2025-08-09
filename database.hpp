#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "order.hpp"
#include <sqlite3.h>
#include <string>
#include <mutex>

class Database {
private:
    sqlite3* db;
    std::mutex dbMutex;

public:
    Database(const std::string& dbName);
    ~Database();
    void saveOrder(const Order& order);
};

#endif
