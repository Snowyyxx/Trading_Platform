#pragma once
#include <queue>
#include <mutex>
#include "order.hpp"

// ── priority-queue comparators ────────────────────────────
// buy orders: we want the highest price on top
struct BuyCmp {
    bool operator()(const Order &a, const Order &b) const {
        return a.price < b.price;   // higher price wins
    }
};

// sell orders: we want the lowest price on top
struct SellCmp {
    bool operator()(const Order &a, const Order &b) const {
        return a.price > b.price;   // lower price wins
    }
};

// ── order book managing one stock id ──────────────────────
class OrderBook {
public:
    // create an empty order book for a single stock symbol
    explicit OrderBook(int sid = 0) : stock_id(sid) {}

    // push a new order into the right queue
    void addOrder(const Order &o);

    // try to find one buy-sell pair that crosses
    // returns true if a match was found and fills 'buy' and 'sell'
    bool matchOrder(Order &buy, Order &sell);

    // quick helpers for dashboards / logging
    // highest buy price, or -1 if no buys
    int bestBid() const;

    // lowest sell price, or -1 if no sells
    int bestAsk() const;

private:
    int stock_id{0};  // which stock this book belongs to

    // max-heap for buys (top = best bid)
    std::priority_queue<Order, std::vector<Order>, BuyCmp>  buy_q;

    // min-heap for sells (top = best ask)
    std::priority_queue<Order, std::vector<Order>, SellCmp> sell_q;

    // guards both queues so addOrder / matchOrder are thread-safe
    mutable std::mutex mtx;
};
