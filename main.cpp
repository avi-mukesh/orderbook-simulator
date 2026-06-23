#include "orderbook.hpp"
#include <iostream>
#include <iomanip> // for std::setprecision

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

    book.add_order(Order(1000, Side::BUY, 1, "ETHUSD"));
    
    // a market buy order that cleans all the asks
    book.add_order(Order(Side::BUY, 6, "BTCUSD"));
    book.print_book();

    // a market sell order that cleans all the bids
    book.add_order(Order(Side::SELL, 4, "BTCUSD"));
    book.print_book();
    

    // can cancel a resting order by specifying it's id
    book.add_order(Order(1100, Side::SELL, 4, "BTCUSD"));
    uint64_t order_to_cancel_id = book.add_order(Order(1050, Side::SELL, 3, "BTCUSD"));
    book.print_book();
    
    book.cancel_order(order_to_cancel_id);
    book.print_book();


    auto& trades = book.trade_log();

    int total_quantity = 0;
    double total_notional = 0;

    std::cout << "\n==== TRADES ==== \n";
    for (const auto& t : trades) {
        std::cout << t << "\n";
        total_quantity += t.quantity;
        total_notional += t.quantity * t.fill_price;
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n===Some stats===\n";
    std::cout << "Quantity: " << total_quantity << " units\n";
    std::cout << "Notional: $" << total_notional << "\n";
    std::cout << "VWAP: ";
    if (total_notional > 0) std::cout << "$" << total_notional/total_quantity << "\n";
    else std::cout << "N/A - no trades made\n";

    return 0;
}