#include "trading_engine.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

TradingEngine::TradingEngine(Database& database, std::atomic<int>& counter)
    : db(database), orderIdCounter(counter) {}

// Private helper to manage the map of order books safely.
OrderBook& TradingEngine::getOrderBook(const std::string& symbol) {
    if (order_books_.find(symbol) == order_books_.end()) {
        std::cout << "[Engine] Creating new order book for symbol: " << symbol << std::endl;
        order_books_[symbol] = std::make_unique<OrderBook>();
    }
    return *order_books_[symbol];
}

std::string TradingEngine::processCommand(const std::string& command) {
    std::lock_guard<std::mutex> lock(engineMutex);
    std::stringstream ss(command);
    std::string cmd_type;
    std::getline(ss, cmd_type, ';');

    if (cmd_type == "ADD") {
        std::string symbol, typeStr, priceStr, qtyStr;
        std::getline(ss, symbol, ';');
        std::getline(ss, typeStr, ';');
        std::getline(ss, priceStr, ';');
        std::getline(ss, qtyStr, ';');
        
        // Convert symbol to uppercase for consistency
        std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);

        try {
            OrderType type = (typeStr == "BUY") ? OrderType::BUY : OrderType::SELL;
            double price = std::stod(priceStr);
            int quantity = std::stoi(qtyStr);
            int orderId = orderIdCounter++;

            Order newOrder{orderId, symbol, type, price, quantity};
            
            // Get the correct order book for the symbol.
            OrderBook& book = getOrderBook(symbol);

            book.addOrder(newOrder);
            db.saveOrder(newOrder);

            std::string trades = book.matchOrders();
            std::string response = "ACK;Order " + std::to_string(orderId) + " for " + symbol + " accepted.\n";
            if (!trades.empty()) {
                response += trades;
            }
            return response;
        } catch (const std::exception& e) {
            return "NACK;Error processing order: " + std::string(e.what()) + "\n";
        }
    } else if (cmd_type == "VIEW") {
        std::string symbol;
        std::getline(ss, symbol, ';');
        std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);

        if (order_books_.find(symbol) == order_books_.end()) {
            return "BOOK;Order book for " + symbol + " is empty.\n";
        }
        OrderBook& book = getOrderBook(symbol);
        return "BOOK;" + book.getDisplayString();
    }
    return "NACK;Unknown command.\n";
}