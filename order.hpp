#ifndef ORDER_HPP
#define ORDER_HPP

#include <string>

enum class OrderType { BUY, SELL };

struct Order {
    int id;
    std::string symbol; // <-- NEW FIELD
    OrderType type;
    double price;
    int quantity;
};

#endif