#ifndef ORDER_HPP
#define ORDER_HPP

#include <string>
#include <chrono>

enum class OrderType { BUY, SELL };

struct Order {
    int id;
    std::string symbol;
    OrderType type;
    int quantity;
    double price;
    std::chrono::steady_clock::time_point timestamp;

    Order() = default;
    Order(int id_, const std::string& sym, OrderType t, int qty, double p)
        : id(id_), symbol(sym), type(t), quantity(qty), price(p),
          timestamp(std::chrono::steady_clock::now()) {}
};

struct BuyOrderComparator {
    bool operator()(const Order& a, const Order& b) const {
        return a.price < b.price; // max-heap for buys
    }
};

struct SellOrderComparator {
    bool operator()(const Order& a, const Order& b) const {
        return a.price > b.price; // min-heap for sells
    }
};

#endif
