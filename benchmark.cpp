#include "orderbook.hpp"
#include <iostream>
#include <chrono>

int main() {
    auto book = OrderBook("BTCUSD");

    const auto start{std::chrono::high_resolution_clock::now()};
    for (int i=0; i<1000000; i++) {
        book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));
    }
    const auto finish{std::chrono::high_resolution_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{finish - start};

    std::cout << "Elapsed time: " << elapsed_seconds.count() << '\n';

    return 0;
}