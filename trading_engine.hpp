#ifndef TRADING_ENGINE_HPP
#define TRADING_ENGINE_HPP

#include "order.hpp"
#include "order_book.hpp"
#include "database.hpp"
#include <boost/lockfree/queue.hpp>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class TradingEngine {
public:
    TradingEngine();
    ~TradingEngine();
    void start();
    void stop();
    void addOrder(const Order& order);
    void enableBenchmarkMode(int totalOrders);

private:
    void matcherLoop();

    std::unordered_map<std::string, OrderBook> books;
    Database db;

    boost::lockfree::queue<Order> incomingOrders;
    std::thread matcherThread;
    std::mutex cv_m;
    std::condition_variable cv;
    std::atomic<bool> running;
    bool newOrderFlag;

    bool benchmarkMode;
    int benchmarkTarget;
};

#endif
