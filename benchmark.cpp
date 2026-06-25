#include "orderbook.hpp"
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>

void generate_data(std::vector<Side>& sides, std::vector<double>& prices, std::vector<int>& quantities, int num_orders){
    int k = 10;
    for (int i=0; i<num_orders; i++) {
        sides.push_back(rand()%2 == 0 ? Side::BUY : Side::SELL);
        // grow the number of price levels with number of orders, keeping the fraction num_price_levels / num_orders constant
        // otherwise, at e.g. 1000000 orders and num price levels 
        int price_range = num_orders / k;
        prices.push_back((1+rand()%price_range)+1000); // TODO: uniform prices aren't realistic, they are usually clustered around the touch
        quantities.push_back((1+rand()%1000));
    }
}

// benchmark harness
int main() {
    // set current time as random seed
    std::srand(std::time({}));

    
    std::vector<int> possible_num_orders = {10, 100, 1000, 10000, 100000, 1000000, 10000000};
    std::map<int, double> time_taken;
    
    for (int num_orders : possible_num_orders) {
        auto book = OrderBook("BTCUSD");
        // preconfiguring all orders, so not counting the time taken for e.g. rand() to run
        // only testing limit orders here 
        std::vector<Side> sides = {};
        std::vector<double> prices = {};
        std::vector<int> quantities = {};
        generate_data(sides, prices, quantities, num_orders);


        // the actual benchmarking 
        const auto start{std::chrono::high_resolution_clock::now()};
        for (int i=0; i<num_orders; i++) {
            book.add_order(Order(prices[i], sides[i], quantities[i], "BTCUSD"));
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