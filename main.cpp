#include "trading_engine.hpp"
#include <iostream>
#include <chrono>

int main() {
    TradingEngine engine;
    engine.start();

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 1; i <= 2000; ++i) {
        Order o1{i, OrderType::BUY, 100.0 + (i % 10), 10};
        Order o2{i + 10000, OrderType::SELL, 100.0 + (i % 10), 10};
        engine.submitOrder(o1);
        engine.submitOrder(o2);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = endTime - startTime;
    std::cout << "Submitted 4000 orders in " << diff.count() << " seconds\n";
    std::cout << "Throughput: " << 4000 / diff.count() << " orders/sec\n";

    engine.stop();
    return 0;
}
