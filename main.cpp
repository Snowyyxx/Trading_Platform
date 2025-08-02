#pragma once
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sqlite3.h>

#include "order.hpp"
#include "order_book.hpp"
#include "database.hpp"
#include "trading_engine.hpp"

// global db mutex (defined in database.cpp)
extern std::mutex db_mutex;

// global flag used by trading_engine.cpp (not needed in this file)
std::atomic<bool> input_active{false};

namespace ModeHelpers {

    inline void sleep_ms(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    // ---------- monitor best bid / ask ----------
    void runMonitorMode() {
        while (true) {
            auto arr = displayOrders();
            struct Stat { int bestBid = -1, bestAsk = -1; };
            std::unordered_map<int, Stat> stats;

            for (auto& o : arr) {
                int sid   = o["stock_id"];
                int price = o["price"];
                auto& S = stats[sid];

                if (o["status"] == "Placed" || o["status"] == "Partially Executed") {
                    if (o["order_type"] == "BUY")
                        S.bestBid = std::max(S.bestBid, price);
                    else {
                        if (S.bestAsk < 0 || price < S.bestAsk)
                            S.bestAsk = price;
                    }
                }
            }

            std::cout << "\n=== monitor (best bid / ask) ===\n";
            for (auto& kv : stats) {
                std::cout << "stock " << kv.first
                          << " | bid " << (kv.second.bestBid < 0 ? 0 : kv.second.bestBid)
                          << " | ask " << (kv.second.bestAsk < 0 ? 0 : kv.second.bestAsk)
                          << "\n";
            }
            sleep_ms(2000);
        }
    }

    // ---------- live transaction feed ----------
    void runTransactionsMode() {
        int last_id = 0;
        while (true) {
            {
                std::lock_guard<std::mutex> lk(db_mutex);
                sqlite3* db = nullptr;
                if (sqlite3_open("stock_exchange.db", &db) == SQLITE_OK) {
                    const char* sql =
                        "select id, buy_order_id, sell_order_id, stock_id, units, sell_order_price "
                        "from transactions where id > ? order by id;";
                    sqlite3_stmt* stmt = nullptr;
                    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
                    sqlite3_bind_int(stmt, 1, last_id);

                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        last_id = sqlite3_column_int(stmt, 0);
                        std::cout << "[tx " << last_id << "] "
                                  << "stock " << sqlite3_column_int(stmt, 3)
                                  << " | qty " << sqlite3_column_int(stmt, 4)
                                  << " @ "    << sqlite3_column_int(stmt, 5)
                                  << " (b#"   << sqlite3_column_int(stmt, 1)
                                  << " / s#"  << sqlite3_column_int(stmt, 2)
                                  << ")\n";
                    }
                    sqlite3_finalize(stmt);
                    sqlite3_close(db);
                }
            }
            sleep_ms(1000);
        }
    }

    // ---------- dump whole db periodically ----------
    void runDbViewMode() {
        while (true) {
            {
                auto orders = displayOrders();
                std::cout << "\n--- order_records ---\n";
                for (auto& o : orders) {
                    std::cout << "id#"  << o["order_id"]
                              << " | s" << o["stock_id"]
                              << " | u" << o["user_id"]
                              << " | "  << o["units"]
                              << " @ "  << o["price"]
                              << " | "  << o["status"]
                              << " / "  << o["order_type"]
                              << "\n";
                }
            }
            {
                std::lock_guard<std::mutex> lk(db_mutex);
                sqlite3* db = nullptr;
                sqlite3_open("stock_exchange.db", &db);

                std::cout << "\n--- transactions ---\n";
                const char* sql =
                    "select id, buy_order_id, sell_order_id, stock_id, units, sell_order_price "
                    "from transactions order by id;";
                sqlite3_stmt* stmt = nullptr;
                sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    std::cout << "tx#" << sqlite3_column_int(stmt, 0)
                              << " | b#" << sqlite3_column_int(stmt, 1)
                              << " / s#" << sqlite3_column_int(stmt, 2)
                              << " | s"  << sqlite3_column_int(stmt, 3)
                              << " | "   << sqlite3_column_int(stmt, 4)
                              << " @ "   << sqlite3_column_int(stmt, 5)
                              << "\n";
                }
                sqlite3_finalize(stmt);
                sqlite3_close(db);
            }
            sleep_ms(3000);
        }
    }

    // ---------- latest traded price per stock ----------
    void runPricesMode() {
        while (true) {
            struct P { int sid, price; };
            std::vector<P> v;

            {
                std::lock_guard<std::mutex> lk(db_mutex);
                sqlite3* db = nullptr;
                sqlite3_open("stock_exchange.db", &db);

                const char* sql =
                    "select stock_id, sell_order_price "
                    "from ( "
                    "  select *, row_number() over (partition by stock_id order by id desc) rn "
                    "  from transactions "
                    ") where rn = 1;";
                sqlite3_stmt* stmt = nullptr;
                sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    v.push_back({ sqlite3_column_int(stmt, 0),
                                  sqlite3_column_int(stmt, 1) });
                }
                sqlite3_finalize(stmt);
                sqlite3_close(db);
            }

            std::sort(v.begin(), v.end(),
                      [](auto& a, auto& b){ return a.price > b.price; });

            std::cout << "\n*** stock prices (latest) ***\n";
            for (auto& p : v) {
                std::cout << "stock " << p.sid
                          << " → "   << p.price << "\n";
            }
            sleep_ms(3000);
        }
    }

    // ---------- self-contained demo: feeder + matcher ----------
    void runDemoMode() {
        TradingEngine engine;            // owns its order books
        std::atomic<bool> running{true};

        // matcher keeps pulling matches
        std::thread matcher([&]{
            engine.runMatcher();
        });

        // feeder pushes random orders every 100 ms
        std::thread feeder([&]{
            int uid = 1;
            while (running) {
                Order o;
                o.stock_id = 1;
                o.units    = 1;
                o.price    = 95 + (std::rand() % 11); // 95-105
                o.user_id  = uid++;
                o.is_buy   = (uid % 2 == 0);
                o.status   = "Placed";
                insert_order(o);         // db insert is mutex-guarded
                sleep_ms(100);
            }
        });

        // run demo for 10 s
        std::this_thread::sleep_for(std::chrono::seconds(10));
        running = false;
        feeder.join();
        matcher.detach();               // leave matcher running or exit
    }
} // namespace ModeHelpers

int main(int argc, char* argv[]) {

    // seed rand for demo mode
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    if (argc < 2) {
        std::cerr << "usage: " << argv[0]
                  << " <mode>\nmodes: engine | buy | sell | monitor | "
                     "transactions | db | prices | demo\n";
        return 1;
    }

    std::string mode(argv[1]);

    if (mode == "engine") {
        TradingEngine engine;
        engine.runMatcher();
    }
    else if (mode == "buy" || mode == "sell") {
        bool is_buy = (mode == "buy");
        while (true) {
            int stock_id, units, price, user_id;
            std::cout << "[" << (is_buy ? "buy" : "sell") << "] "
                      << "enter: stock_id units price user_id → " << std::flush;
            if (!(std::cin >> stock_id >> units >> price >> user_id))
                break;

            Order o;
            o.stock_id = stock_id;
            o.units    = units;
            o.price    = price;
            o.user_id  = user_id;
            o.is_buy   = is_buy;
            o.status   = "Placed";

            int assigned_id = insert_order(o);
            if (assigned_id > 0) {
                std::cout << (is_buy ? "\033[32m" : "\033[31m")
                          << (is_buy ? "buy " : "sell ") << "order #"
                          << assigned_id << "\033[0m\n";
            }
        }
    }
    else if (mode == "monitor")       ModeHelpers::runMonitorMode();
    else if (mode == "transactions")  ModeHelpers::runTransactionsMode();
    else if (mode == "db")            ModeHelpers::runDbViewMode();
    else if (mode == "prices")        ModeHelpers::runPricesMode();
    else if (mode == "demo")          ModeHelpers::runDemoMode();
    else {
        std::cerr << "unknown mode: " << mode << "\n";
        return 1;
    }

    return 0;
}
