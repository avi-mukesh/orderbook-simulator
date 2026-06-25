#include "orderbook.hpp"
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>

// benchmark harness
int main() {
    // set current time as random seed
    std::srand(std::time({}));

    auto book = OrderBook("BTCUSD");

    int num_orders = 1000000;
    
    // preconfiguring all orders, so not counting the time taken for e.g. rand() to run
    // only testing limit orders here 
    std::vector<Side> sides = {};
    std::vector<double> prices = {};
    std::vector<int> quantities = {};
    for (int i=0; i<num_orders; i++) {
        sides.push_back(rand()%2 == 0 ? Side::BUY : Side::SELL);
        prices.push_back((1+rand()%500)+1000);
        quantities.push_back((1+rand()%1000));
    }
    
    // the actual benchmarking 
    const auto start{std::chrono::high_resolution_clock::now()};
    for (int i=0; i<num_orders; i++) {
        book.add_order(Order(prices[i], sides[i], quantities[i], "BTCUSD"));
    }
    const auto finish{std::chrono::high_resolution_clock::now()};
    
    const std::chrono::duration<double> elapsed_duration{finish - start};
    const double elapsed_seconds = elapsed_duration.count();
    double orders_per_second = num_orders / elapsed_seconds;
    double latency_per_order = (1/orders_per_second) * 1e9;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Elapsed time: " << elapsed_seconds << '\n';
    std::cout << "Limit orders / second: " << orders_per_second << "\n";
    std::cout << "Latency per order: " << latency_per_order << "ns\n";

    return 0;
}