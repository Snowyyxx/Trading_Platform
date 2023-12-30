#include <sqlite3.h>
#include <iostream>
#include <string>
#include"header.h"

// Assuming the 'order' class/struct is defined elsewhere with the appropriate fields
void insert_order(order order_obj) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *db_path = "stock_exchange.db"; // Path to your database file
    int rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char *sql = "INSERT INTO order_records (order_id, stock_id, user_id, units, price, status, order_type) VALUES (?, ?, ?, ?, ?, ?, ?)";
    std::string order_type = order_obj.buy_flag ? "BUY" : "SELL";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    // Binding the parameters (starting from index 1 as order_id is NULL)
    sqlite3_bind_int(stmt,1,order_obj.order_id);
    sqlite3_bind_int(stmt, 2, order_obj.stock_id);
    sqlite3_bind_int(stmt, 3, order_obj.user_id);
    sqlite3_bind_int(stmt, 4, order_obj.units);
    sqlite3_bind_int(stmt, 5, order_obj.price);
    sqlite3_bind_text(stmt, 6, order_obj.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, order_type.c_str(), -1, SQLITE_TRANSIENT);

    // Executing the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Error executing statement: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Order inserted successfully." << std::endl;
    }

    // Cleaning up
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}



void update_status(order order_obj){
    sqlite3*db;
    sqlite3_stmt*stmt;
    sqlite3_open("stock_exchange.db",&db);
    const char* sql = "update order_records set status = ? where order_id = ?";
    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    sqlite3_bind_text(stmt,1,order_obj.status.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,2,order_obj.order_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}



void transaction_insert(order buy_order, order sell_order){
    int stock_id = buy_order.stock_id;
    int units;
    if(buy_order.units==sell_order.units)
        units = buy_order.units;
    else
        buy_order.units>sell_order.units?units=sell_order.units:units=buy_order.units;
    
    sqlite3*db;
    sqlite3_stmt *stmt;
    sqlite3_open("stock_exchange.db",&db);
    const char * sql = "insert into transactions (buy_order_id,sell_order_id,stock_id,units,buy_order_price,sell_order_price) values(?,?,?,?,?,?)";
    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    
    int buy_order_id = buy_order.order_id;
    int sell_order_id = sell_order.order_id;

    sqlite3_bind_int(stmt,1,buy_order_id);
    sqlite3_bind_int(stmt,2,sell_order_id);
    sqlite3_bind_int(stmt,3,stock_id);
    sqlite3_bind_int(stmt,4,units);
    sqlite3_bind_int(stmt,5,buy_order.price);
    sqlite3_bind_int(stmt,6,sell_order.price);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}