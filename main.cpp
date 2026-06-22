#include "orderbook.hpp"
#include <iostream>

int main() {
    auto book = OrderBook("BTCUSD");

    book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));
    book.add_order(Order(900, Side::BUY, 2, "BTCUSD"));
    book.add_order(Order(850, Side::BUY, 3, "BTCUSD"));
    book.add_order(Order(700, Side::BUY, 1, "BTCUSD"));

    book.add_order(Order(1200, Side::SELL, 3, "BTCUSD"));
    book.add_order(Order(1100, Side::SELL, 2, "BTCUSD"));
    book.add_order(Order(1070, Side::SELL, 3, "BTCUSD"));

    // spread hasn't been crossed yet - no order gets filled
    book.print_book();
    
    
    // spread is crossed (1080 > 1070), order gets filled
    book.add_order(Order(1080, Side::BUY, 1, "BTCUSD"));
    book.print_book();
    
    
    // more aggressive buy - going through multiple price levels to fill order
    // think this demonstrates slippage
    book.add_order(Order(1100, Side::BUY, 4, "BTCUSD"));
    book.print_book();
    
    // an aggressive sell - it doesn't even get filled completely, and now it's sitting on the book
    // this is what happens in low liquidity environments
    book.add_order(Order(900, Side::SELL, 10, "BTCUSD"));
    book.print_book();

    // a market buy order that cleans all the asks
    book.add_order(Order(Side::BUY, 6, "BTCUSD"));
    book.print_book();

    // a market sell order that cleans all the bids
    book.add_order(Order(Side::SELL, 4, "BTCUSD"));
    book.print_book();


    auto& trades = book.trade_log();
    std::cout << "\n==== TRADES ==== \n";
    for (const auto& t : trades) {
        std::cout << t << "\n";
    }

    return 0;
}