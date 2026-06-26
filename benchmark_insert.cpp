#include "orderbook.hpp"
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>

void generate_data(std::vector<double>& prices, std::vector<int>& quantities, int num_orders){
    int k = 10;
    for (int i=0; i<num_orders; i++) {
        // grow the number of price levels with number of orders, keeping the fraction num_price_levels / num_orders constant
        // otherwise, at e.g. 1000000 orders and num price levels 
        int price_range = num_orders / k;
        prices.push_back((1+rand()%price_range)+1000); // TODO: uniform prices aren't realistic, they are usually clustered around the touch
        quantities.push_back((1+rand()%1000));
    }
}

// benchmark just the insertion
// should be O(log n) insertion into std::map
// i.e. each time we 10x orders, (with 10x price levels as well), we should add roughly the same amount of additional time each increase
// but after n reaches a certain size, local cache is not available anymore, and the data is scattered across RAM
// so might be better to use an array (contiguous block of memory)
int main() {
    // set current time as random seed
    std::srand(std::time({}));

    std::vector<int> possible_num_orders = {1000, 10000, 100000, 1000000, 10000000};
    std::map<int, double> time_taken;
    
    for (int num_orders : possible_num_orders) {
        auto book = OrderBook("BTCUSD");

        // rest a bunch of asks for now
        std::vector<double> prices = {};
        std::vector<int> quantities = {};
        generate_data(prices, quantities, num_orders);
        const auto start{std::chrono::high_resolution_clock::now()};
        for (int i=0; i<num_orders; i++) {
            book.add_order(Order(prices[i], Side::BUY, quantities[i], "BTCUSD"));
        }
        const auto finish{std::chrono::high_resolution_clock::now()};
        
        const std::chrono::duration<double> elapsed_duration{finish - start};
        const double elapsed_seconds = elapsed_duration.count();
        time_taken[num_orders] = elapsed_seconds;
    }

    std::cout << std::fixed << std::setprecision(2);
    for (auto const& [n, t] : time_taken) {
        std::cout << "Num orders: " << n << '\n';
        std::cout << "Elapsed time: " << t << '\n';
        
        double orders_per_second = n / t;
        double latency_per_order = (1/orders_per_second) * 1e9;

        std::cout << "Limit orders / second: " << orders_per_second << "\n";
        std::cout << "Latency per order: " << latency_per_order << "ns\n\n";
    }

    return 0;
}