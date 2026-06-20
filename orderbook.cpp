#include "orderbook.hpp"
#include <optional>
#include <deque>

OrderBook::OrderBook(std::string symbol) : symbol_(std::move(symbol)) {}


std::vector<Trade> OrderBook::add_order(Order order) {
    order.id = ++next_id_;

    double p = order.price;
    if(order.side==Side::BUY) {
        // if key p doesn't exist, it is auto created
        bids_[p].push_back(order);
    } else {
        asks_[p].push_back(order);
    }
    return std::vector<Trade>();
}

void OrderBook::print_book() const {
    std::cout << "==== ORDER BOOK ====\n";

    std::cout << "==== ASKS ====\n";
    
    for (auto const& [price, orders] : asks_) {
        std::cout << "$" << price << " : ";
        int total = 0;
        for (auto const& o : orders) {
            total += o.quantity;
        }
        std::cout << "quantity=" << total << "\n";
    }

    std::cout << "==== BIDS ====\n";
    
    for (auto const& [price, orders] : bids_) {
        std::cout << "$" << price << " : ";
        int total = 0;
        for (auto const& o : orders) {
            total += o.quantity;
        }
        std::cout << "quantity=" << total << "\n";
    }
}

std::optional<double> OrderBook::best_bid() const {
    if(!bids_.empty()) {
        const auto best_it = bids_.begin();
        return best_it->first;
    }
    return std::nullopt;
}

std::optional<double> OrderBook::best_ask() const {
    if(!asks_.empty()) {
        const auto best_it = asks_.begin();
        return best_it->first;
    }
    return std::nullopt;
}