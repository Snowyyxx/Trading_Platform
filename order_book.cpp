#include "order_book.hpp"

// add a new order to the correct queue
void OrderBook::addOrder(const Order &o) {
    std::lock_guard<std::mutex> lk(mtx);  // lock so threads don’t collide

    if (o.is_buy)
        buy_q.push(o);    // push into buy queue
    else
        sell_q.push(o);   // push into sell queue
}

// try to match one buy and sell order
bool OrderBook::matchOrder(Order &buy, Order &sell) {
    std::lock_guard<std::mutex> lk(mtx);  // lock both queues

    // if either side is empty, no match is possible
    if (buy_q.empty() || sell_q.empty())
        return false;

    // no match if buyer isn't willing to pay enough
    if (buy_q.top().price < sell_q.top().price)
        return false;

    // take the top buy and sell orders out of the queues
    buy = buy_q.top();   buy_q.pop();
    sell = sell_q.top(); sell_q.pop();

    return true;  // match was successful
}

// return the best bid (highest buy price)
int OrderBook::bestBid() const {
    std::lock_guard<std::mutex> lk(mtx);
    return buy_q.empty() ? -1 : buy_q.top().price;
}

// return the best ask (lowest sell price)
int OrderBook::bestAsk() const {
    std::lock_guard<std::mutex> lk(mtx);
    return sell_q.empty() ? -1 : sell_q.top().price;
}
