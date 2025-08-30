#include "order_book.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

static bool buyComp(const Order& a, const Order& b) { return a.price < b.price; }
static bool sellComp(const Order& a, const Order& b) { return a.price > b.price; }

OrderBook::OrderBook() : buyOrders(buyComp), sellOrders(sellComp) {}

void OrderBook::addOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(mtx);
    if (order.type == OrderType::BUY) buyOrders.push(order);
    else sellOrders.push(order);
}

std::string OrderBook::matchOrders() {
    std::lock_guard<std::mutex> lock(mtx);
    std::stringstream trade_stream;
    while (!buyOrders.empty() && !sellOrders.empty() && buyOrders.top().price >= sellOrders.top().price) {
        Order buy = buyOrders.top(); buyOrders.pop();
        Order sell = sellOrders.top(); sellOrders.pop();
        int tradeQty = std::min(buy.quantity, sell.quantity);
        trade_stream << "[TRADE] " << tradeQty << " units matched at price " << sell.price
                     << " (Buy ID: " << buy.id << ", Sell ID: " << sell.id << ")\n";
        buy.quantity -= tradeQty;
        sell.quantity -= tradeQty;
        if (buy.quantity > 0) buyOrders.push(buy);
        if (sell.quantity > 0) sellOrders.push(sell);
    }
    return trade_stream.str();
}

std::string OrderBook::getDisplayString() const {
    std::lock_guard<std::mutex> lock(mtx);
    std::stringstream ss;
    auto buys_copy = buyOrders;
    auto sells_copy = sellOrders;
    std::vector<Order> buys, sells;
    while (!buys_copy.empty()) { buys.push_back(buys_copy.top()); buys_copy.pop(); }
    while (!sells_copy.empty()) { sells.push_back(sells_copy.top()); sells_copy.pop(); }
    std::reverse(sells.begin(), sells.end());

    ss << "\n==================================================\n";
    ss << "                      ORDER BOOK\n";
    ss << "==================================================\n";
    ss << "SELLS (ASKS)           | BUYS (BIDS)\n";
    ss << "------------------------|-------------------------\n";
    ss << std::left << std::setw(11) << "Price" << "| " << std::setw(11) << "Qty"
              << "| " << std::setw(11) << "Price" << "| " << "Qty\n";
    ss << "-----------|------------|-----------|------------\n";
    size_t max_rows = std::max(sells.size(), buys.size());
    for(size_t i = 0; i < max_rows; ++i) {
        if (i < sells.size()) {
            ss << std::left << std::fixed << std::setprecision(2) << std::setw(11) << sells[i].price << "| " << std::setw(11) << sells[i].quantity;
        } else {
            ss << std::string(24, ' ');
        }
        ss << "| ";
        if (i < buys.size()) {
            ss << std::left << std::fixed << std::setprecision(2) << std::setw(11) << buys[i].price << "| " << buys[i].quantity;
        }
        ss << "\n";
    }
    ss << "==================================================\n";
    return ss.str();
}