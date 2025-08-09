#include "trading_engine.hpp"
#include <iostream>
#include <thread>
#include <random>
#include <string>

void runBenchmark(TradingEngine& engine, int totalOrders) {
    engine.enableBenchmarkMode(totalOrders);
    engine.start();

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> qtyDist(1, 100);
    std::uniform_real_distribution<double> priceDist(90.0, 110.0);

    for (int i = 0; i < totalOrders; ++i) {
        OrderType type = (i % 2 == 0) ? OrderType::BUY : OrderType::SELL;
        engine.addOrder(Order(i, "AAPL", type, qtyDist(rng), priceDist(rng)));
    }

    engine.stop();
}

void runInteractive(TradingEngine& engine) {
    engine.start();

    int orderId = 1;
    while (true) {
        std::cout << "\n1. Place BUY Order\n";
        std::cout << "2. Place SELL Order\n";
        std::cout << "3. Exit\n";
        std::cout << "Enter choice: ";

        int choice;
        std::cin >> choice;
        if (choice == 3) break;

        std::string symbol;
        int qty;
        double price;

        std::cout << "Enter symbol: ";
        std::cin >> symbol;
        std::cout << "Enter quantity: ";
        std::cin >> qty;
        std::cout << "Enter price: ";
        std::cin >> price;

        if (choice == 1) {
            engine.addOrder(Order(orderId++, symbol, OrderType::BUY, qty, price));
        } else if (choice == 2) {
            engine.addOrder(Order(orderId++, symbol, OrderType::SELL, qty, price));
        }
    }

    engine.stop();
}

int main(int argc, char* argv[]) {
    TradingEngine engine;

    if (argc > 1 && std::string(argv[1]) == "--benchmark") {
        runBenchmark(engine, 200000); // Example: 200k orders for testing
    } else {
        runInteractive(engine);
    }
}
