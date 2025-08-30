#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include "order.hpp"
#include <queue>
#include <mutex>
#include <vector>
#include <string>

class OrderBook {
private:
    using OrderComp = bool(*)(const Order&, const Order&);
    std::priority_queue<Order, std::vector<Order>, OrderComp> buyOrders;
    std::priority_queue<Order, std::vector<Order>, OrderComp> sellOrders;
    mutable std::mutex mtx;

public:
    OrderBook();
    void addOrder(const Order& order);
    std::string matchOrders();
    std::string getDisplayString() const;
};

#endif