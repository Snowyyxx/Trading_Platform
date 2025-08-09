#include "database.hpp"
#include <iostream>
#include <fstream>

Database::Database(const std::string& dbName) {
    if (sqlite3_open(dbName.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Error opening DB: " << sqlite3_errmsg(db) << "\n";
    }
    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS orders ("
        "id INTEGER PRIMARY KEY, "
        "type TEXT, "
        "price REAL, "
        "quantity INTEGER);";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error creating table: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}

Database::~Database() {
    sqlite3_close(db);
}

void Database::saveOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(dbMutex);

    // Persist to SQLite
    std::string sql = "INSERT INTO orders (id, type, price, quantity) VALUES (" +
        std::to_string(order.id) + ", '" +
        (order.type == OrderType::BUY ? "BUY" : "SELL") + "', " +
        std::to_string(order.price) + ", " +
        std::to_string(order.quantity) + ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error inserting order: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }

    // Crash-safe append-only log
    std::ofstream logFile("order_log.txt", std::ios::app);
    logFile << order.id << "," 
            << (order.type == OrderType::BUY ? "BUY" : "SELL") << ","
            << order.price << "," 
            << order.quantity << "\n";
    logFile.flush();
}
