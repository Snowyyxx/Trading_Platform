#ifndef TRADING_ENGINE_HPP
#define TRADING_ENGINE_HPP

#include "order_book.hpp"
#include "database.hpp"
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <memory>

class TradingEngine {
private:
    Database& db;
    std::atomic<int>& orderIdCounter;
    
    // A map from a stock symbol (string) to a unique pointer to its OrderBook.
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> order_books_;
    
    // A single mutex to protect access to the map of order books.
    std::mutex engineMutex;

    // Helper function to get or create an order book for a symbol.
    OrderBook& getOrderBook(const std::string& symbol);

public:
    explicit TradingEngine(Database& database, std::atomic<int>& counter);
    std::string processCommand(const std::string& command);
};

#endif