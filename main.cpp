#include "order.hpp"
#include <iostream>

int main() {
    auto order = Order(1, 250, Side::BUY, 5, "BTCUSD");
    std::cout << order << "\n";
    return 0;
}