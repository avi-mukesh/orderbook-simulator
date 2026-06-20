#include "orderbook.hpp"
#include <iostream>

int main() {
    auto book = OrderBook("BTCUSD");

    book.add_order(Order(250, Side::BUY, 5, "BTCUSD"));
    book.add_order(Order(300, Side::BUY, 3, "BTCUSD"));
    book.add_order(Order(300, Side::SELL, 3, "BTCUSD"));
    book.add_order(Order(200, Side::BUY, 1, "BTCUSD"));
    book.add_order(Order(400, Side::SELL, 4, "BTCUSD"));

    book.print_book();

    return 0;
}