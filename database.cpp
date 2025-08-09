#include "database.hpp"
#include <iostream>

Database::Database(const std::string& filename) {
    if (sqlite3_open(filename.c_str(), &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        db = nullptr;
    }
}

Database::~Database() {
    if (db) sqlite3_close(db);
}

void Database::insertTrade(const std::string& symbol, int quantity, double price) {
    if (!db) return;
    std::string sql = "INSERT INTO trades (symbol, quantity, price) VALUES ('" +
                      symbol + "'," + std::to_string(quantity) + "," + std::to_string(price) + ");";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}
