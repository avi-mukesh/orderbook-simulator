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
    uint64_t order_id = book.add_order(Order(1050, Side::SELL, 3, "BTCUSD"));
    book.print_book();
    
    bool cancelled = book.cancel_order(order_id);
    std::cout << (cancelled ? "\nCancelled order" : "\nUnable to cancel order") << " with id: " << order_id << "\n";
    book.print_book();
    
    // trying to cancel the same order again
    cancelled = book.cancel_order(order_id);
    std::cout << (cancelled ? "\nCancelled order" : "\nUnable to cancel order") << " with id: " << order_id << "\n";
    

    // this order would've filled if the previous one wasn't cancelled, so now it just rests
    book.add_order(Order(1050, Side::BUY, 4, "BTCUSD"));
    book.print_book();
    
    
    // trying to cancel an already filled order
    order_id = book.add_order(Order(1000, Side::SELL, 4, "BTCUSD"));
    book.add_order(Order(1000, Side::BUY, 4, "BTCUSD"));
    cancelled = book.cancel_order(order_id);
    std::cout << (cancelled ? "\nCancelled order" : "\nUnable to cancel order") << " with id: " << order_id << "\n";
    book.print_book();


    // testing IOC (immediate or cancel) order
    // after this sell order is rested, on the book there are two price levels on the asks book: $1100 (qty=4) and $1200 (qty=4)
    // IOC buy order $1100 (qty=8) will only fill if price is equal to or better than what we specify (just like normal limit orders about)
    // BUT, if unfilled, it won't rest on the book (like a market order)
    // so in this case, the IOC will only fill the qty=4 at the $1100 level, and the unfilled part is discarded
    book.add_order(Order(1200, Side::SELL, 4, "BTCUSD"));
    book.print_book();
    
    
    // testing FOK (fill or kill) order
    // at this stage, in bids we have only one level $1000 (qty=4)
    // if we place a sell FOK order at $1000 (qty=6) then it won't fill at all (because quantity 2 would be left over)
    // instead, it will be killed (cancelled) completely
    book.add_order(Order(OrderType::FOK, 1000, Side::SELL, 6, "BTCUSD"));
    book.print_book();

    // this FOK order gets filled
    book.add_order(Order(OrderType::FOK, 1000, Side::SELL, 4, "BTCUSD"));
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