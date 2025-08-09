#include "trading_engine.hpp"
#include <iostream>

TradingEngine::TradingEngine()
    : db("orders.db"), orderQueue(1024), running(false), guard(nullptr) {}

TradingEngine::~TradingEngine() {
    stop();
}

void TradingEngine::start() {
    running = true;
    matcherThread = std::thread(&TradingEngine::matchLoop, this);
    guard = new ThreadGuard(matcherThread);
}

void TradingEngine::stop() {
    running = false;
    cv.notify_all();
    delete guard; // joins thread
}

void TradingEngine::submitOrder(const Order& order) {
    orderQueue.push(order);
    cv.notify_one();
}

void TradingEngine::matchLoop() {
    while (running) {
        std::unique_lock<std::mutex> lock(cvMutex);
        cv.wait(lock, [this]{ return !orderQueue.empty() || !running; });

        Order order;
        while (orderQueue.pop(order)) {
            orderBook.addOrder(order);
            db.saveOrder(order);
            orderBook.matchOrders();
        }
    }
}
