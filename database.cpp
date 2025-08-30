#include "database.hpp"
#include <iostream>
#include <stdexcept>

Database::Database(const std::string& dbName) {
    if (sqlite3_open(dbName.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Error opening DB: " + std::string(sqlite3_errmsg(db)));
    }
    // Add the 'symbol' column to the table definition.
    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS orders ("
        "id INTEGER PRIMARY KEY, "
        "symbol TEXT NOT NULL, "
        "type TEXT, "
        "price REAL, "
        "quantity INTEGER);";
    char* errMsgPtr = nullptr;
    if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsgPtr) != SQLITE_OK) {
        std::string err = "Error creating table: " + std::string(errMsgPtr);
        sqlite3_free(errMsgPtr);
        throw std::runtime_error(err);
    }
}

Database::~Database() { if (db) sqlite3_close(db); }

void Database::saveOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(dbMutex);
    const char* sql = "INSERT INTO orders (id, symbol, type, price, quantity) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }
    // Bind the new 'symbol' value.
    sqlite3_bind_int(stmt, 1, order.id);
    sqlite3_bind_text(stmt, 2, order.symbol.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, (order.type == OrderType::BUY ? "BUY" : "SELL"), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, order.price);
    sqlite3_bind_int(stmt, 5, order.quantity);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = "Error inserting order: " + std::string(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        throw std::runtime_error(err);
    }
    sqlite3_finalize(stmt);
}

int Database::getMaxOrderId() {
    std::lock_guard<std::mutex> lock(dbMutex);
    int maxId = 0;
    const char* sql = "SELECT MAX(id) FROM orders;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
                maxId = sqlite3_column_int(stmt, 0);
            }
        }
    }
    sqlite3_finalize(stmt);
    return maxId;
}