#ifndef TRADING_ENGINE_HPP
#define TRADING_ENGINE_HPP

#include "order_book.hpp"
#include "database.hpp"
#include <boost/lockfree/queue.hpp>
#include <condition_variable>
#include <thread>
#include <atomic>

class ThreadGuard {
private:
    std::thread& t;
public:
    explicit ThreadGuard(std::thread& t_) : t(t_) {}
    ~ThreadGuard() {
        if (t.joinable()) {
            t.join();
        }
    }
};

class TradingEngine {
private:
    OrderBook orderBook;
    Database db;
    boost::lockfree::queue<Order> orderQueue;
    std::mutex cvMutex;
    std::condition_variable cv;
    std::atomic<bool> running;
    std::thread matcherThread;
    ThreadGuard* guard;

    void matchLoop();

public:
    TradingEngine();
    ~TradingEngine();
    void start();
    void stop();
    void submitOrder(const Order& order);
};

#endif
