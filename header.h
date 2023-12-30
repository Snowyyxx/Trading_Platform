#include <iostream>
#include <queue>
#include <stack>
#include <thread>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <atomic>
#include <string>
#include <random>
#include <unistd.h>
#include <mutex>

const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string RESET = "\033[0m";
std::string date = "27/12/2023";

class order;

class stock {
public:
    int stock_id;
    std::vector<order> buy_queue;
    std::vector<order> sell_queue;
    stock(int stock_id){
        this->stock_id =stock_id;
    }
};

std::vector<stock> stock_queue;

bool check_stock_exists(int stock_id) {
    for (auto &i : stock_queue) {
        if (i.stock_id == stock_id) {
            return true;
        }
    }
    return false;
}

int return_stock_index(int stock_id) {
    if (check_stock_exists(stock_id)) {
        int count = 0;
        for (auto &i : stock_queue) {
            if (stock_id == i.stock_id) {
                break;
            }
            count++;
        }
        return count;
    } else{
        stock new_stock(stock_id);
        stock_queue.push_back(new_stock);
        return stock_queue.size()-1;
    }
    return -1; // Indicate that the stock does not exist
}

void heapify(std::vector<order>& queue, int i, int n);
void build_max_heap(std::vector<order>& queue);
void heap_sort(std::vector<order>& queue);

class order {
public:
    static int count;
    int stock_id;
    bool buy_flag;
    int order_id;
    bool sell_flag;
    int units;
    float price;
    int user_id;
    std::string status;

    order(int stock_id, bool buy_flag, bool sell_flag, int units, float price, std::string status, int user_id) {
        this->stock_id = stock_id;
        this->buy_flag = buy_flag;
        this->sell_flag = sell_flag;
        this->units = units;
        this->price = price;
        this->status = status;
        this->user_id = user_id;
        this->order_id = count;
        count++;
        int i = return_stock_index(stock_id);
        if (i >= 0) {
            if (buy_flag) {
                stock_queue[i].buy_queue.push_back(*this);
                heap_sort(stock_queue[i].buy_queue);
            } else {
                stock_queue[i].sell_queue.push_back(*this);
                heap_sort(stock_queue[i].sell_queue);
                std::reverse(stock_queue[i].sell_queue.begin(), stock_queue[i].sell_queue.end());
            }
        }
    }

    void display_buy_queue(int stock_id) {
        int i = return_stock_index(stock_id);
        if (i >= 0) {
            std::vector<order>& buy_queue = stock_queue[i].buy_queue;
            std::cout << "Buy Queue:" << std::endl;
            for (auto& ord : buy_queue) {
                std::cout << "Price: " << ord.price << std::endl;
            }
            std::cout << std::endl;
        }
    }

    void display_sell_queue(int stock_id) {
        int i = return_stock_index(stock_id);
        if (i >= 0) {
            std::vector<order>& sell_queue = stock_queue[i].sell_queue;
            std::cout << "Sell Queue:" << std::endl;
            for (auto& ord : sell_queue) {
                std::cout << "Price: " << ord.price << std::endl;
            }
            std::cout << std::endl;
        }
    }
};

void heapify(std::vector<order>& queue, int i, int n) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && queue[left].price > queue[largest].price) {
        largest = left;
    }
    if (right < n && queue[right].price > queue[largest].price) {
        largest = right;
    }
    if (largest != i) {
        std::swap(queue[i], queue[largest]);
        heapify(queue, largest, n);
    }
}

void build_max_heap(std::vector<order>& queue) {
    int n = queue.size();
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(queue, i, n);
    }
}

void heap_sort(std::vector<order>& queue) {
    build_max_heap(queue);
    int n = queue.size();
    for (int i = n - 1; i > 0; i--) {
        std::swap(queue[0], queue[i]);
        heapify(queue, 0, i);
    }
}
int order::count=0;