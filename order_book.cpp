#include "order_book.hpp"
#include <iostream>

static bool buyComp(const Order& a, const Order& b) {
    return a.price < b.price;
}
static bool sellComp(const Order& a, const Order& b) {
    return a.price > b.price;
}

OrderBook::OrderBook()
    : buyOrders(buyComp), sellOrders(sellComp) {}

void OrderBook::addOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(mtx);
    if (order.type == OrderType::BUY) {
        buyOrders.push(order);
    } else {
        sellOrders.push(order);
    }
}

bool OrderBook::matchOrders() {
    std::lock_guard<std::mutex> lock(mtx);
    bool matched = false;
    while (!buyOrders.empty() && !sellOrders.empty() &&
           buyOrders.top().price >= sellOrders.top().price) {
        Order buy = buyOrders.top(); buyOrders.pop();
        Order sell = sellOrders.top(); sellOrders.pop();
        int tradeQty = std::min(buy.quantity, sell.quantity);
        std::cout << "Matched Order: BuyID=" << buy.id 
                  << " SellID=" << sell.id
                  << " Qty=" << tradeQty << " Price=" << sell.price << "\n";
        matched = true;
    }
    return matched;
}
