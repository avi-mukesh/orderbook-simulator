#include "orderbook.hpp"
#include <iostream>

int main() {
    auto book = OrderBook("BTCUSD");

    std::vector<Trade> t1 = book.add_order(Order(1, 250, Side::BUY, 5, "BTCUSD"));
    std::vector<Trade> t2 = book.add_order(Order(2, 300, Side::BUY, 3, "BTCUSD"));
    std::vector<Trade> t3 = book.add_order(Order(2, 300, Side::BUY, 3, "BTCUSD"));

    book.print_book();

    return 0;
}