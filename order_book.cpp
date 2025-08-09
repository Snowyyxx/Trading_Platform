#include "order_book.hpp"

OrderBook::OrderBook(const std::string& sym) : symbol(sym) {}

void OrderBook::addOrder(const Order& order) {
    if (order.type == OrderType::BUY) {
        buyOrders.push(order);
    } else {
        sellOrders.push(order);
    }
}

bool OrderBook::match(Order& tradeResult) {
    if (!buyOrders.empty() && !sellOrders.empty()) {
        Order topBuy = buyOrders.top();
        Order topSell = sellOrders.top();

        if (topBuy.price >= topSell.price) {
            int tradeQty = std::min(topBuy.quantity, topSell.quantity);
            double tradePrice = (topBuy.price + topSell.price) / 2.0;

            tradeResult = Order(-1, symbol, OrderType::BUY, tradeQty, tradePrice);

            topBuy.quantity -= tradeQty;
            topSell.quantity -= tradeQty;
            buyOrders.pop();
            sellOrders.pop();

            if (topBuy.quantity > 0) buyOrders.push(topBuy);
            if (topSell.quantity > 0) sellOrders.push(topSell);

            return true;
        }
    }
    return false;
}
