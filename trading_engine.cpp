#include "trading_engine.hpp"
#include <iostream>

TradingEngine::TradingEngine()
    : db("stock_exchange.db"), incomingOrders(1024),
      running(false), newOrderFlag(false),
      benchmarkMode(false), benchmarkTarget(0) {}

TradingEngine::~TradingEngine() { stop(); }

void TradingEngine::start() {
    running = true;
    matcherThread = std::thread(&TradingEngine::matcherLoop, this);
}

void TradingEngine::stop() {
    running = false;
    cv.notify_all();
    if (matcherThread.joinable()) matcherThread.join();
}

void TradingEngine::enableBenchmarkMode(int totalOrders) {
    benchmarkMode = true;
    benchmarkTarget = totalOrders;
}

void TradingEngine::addOrder(const Order& order) {
    incomingOrders.push(order);
    {
        std::lock_guard<std::mutex> lock(cv_m);
        newOrderFlag = true;
    }
    cv.notify_one();
}

void TradingEngine::matcherLoop() {
    int processedOrders = 0;
    auto startTime = std::chrono::steady_clock::now();

    while (running) {
        std::unique_lock<std::mutex> lock(cv_m);
        cv.wait(lock, [this] { return newOrderFlag || !running; });
        newOrderFlag = false;
        lock.unlock();

        Order order;
        while (incomingOrders.pop(order)) {
            auto& book = books.try_emplace(order.symbol, order.symbol).first->second;
            book.addOrder(order);

            Order trade;
            while (book.match(trade)) {
                db.insertTrade(trade.symbol, trade.quantity, trade.price);
            }
            processedOrders++;

            if (benchmarkMode && processedOrders >= benchmarkTarget) {
                auto endTime = std::chrono::steady_clock::now();
                double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;
                std::cout << "Processed " << processedOrders << " orders in " << seconds
                          << " seconds (" << (processedOrders / seconds) << " orders/sec)\n";
                running = false;
                cv.notify_all();
                return;
            }
        }
    }
}
