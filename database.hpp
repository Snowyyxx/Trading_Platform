#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <sqlite3.h>
#include <string>

class Database {
public:
    explicit Database(const std::string& filename);
    ~Database();
    void insertTrade(const std::string& symbol, int quantity, double price);

private:
    sqlite3* db;
};

#endif
