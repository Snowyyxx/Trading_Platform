#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include "order.hpp"
#include <queue>
#include <string>

class OrderBook {
public:
    explicit OrderBook(const std::string& symbol);
    void addOrder(const Order& order);
    bool match(Order& tradeResult);

private:
    std::string symbol;
    std::priority_queue<Order, std::vector<Order>, BuyOrderComparator> buyOrders;
    std::priority_queue<Order, std::vector<Order>, SellOrderComparator> sellOrders;
};

#endif
