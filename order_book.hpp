#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include "order.hpp"
#include <queue>
#include <mutex>

class OrderBook {
private:
    // Max-heap for buy orders (highest price first)
    std::priority_queue<Order, std::vector<Order>, 
        bool(*)(const Order&, const Order&)> buyOrders;

    // Min-heap for sell orders (lowest price first)
    std::priority_queue<Order, std::vector<Order>, 
        bool(*)(const Order&, const Order&)> sellOrders;

    std::mutex mtx;

public:
    OrderBook();
    void addOrder(const Order& order);
    bool matchOrders();
};

#endif
