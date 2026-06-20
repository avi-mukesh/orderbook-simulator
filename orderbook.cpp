#include "orderbook.hpp"
#include <optional>
#include <deque>
#include <algorithm>

OrderBook::OrderBook(std::string symbol) : symbol_(std::move(symbol)) {}


std::vector<Trade> OrderBook::add_order(Order order) {
    order.id = ++next_id_;

    std::vector<Trade> fills = match(order);

    // rest order only if it has quantity left over after matching
    if (order.quantity > 0) {
        double p = order.price;
        if(order.side==Side::BUY) {
            // if key p doesn't exist, it is auto created
            bids_[p].push_back(order);
        } else {
            asks_[p].push_back(order);
        }
    }

    for (const auto& f : fills) {
        trade_log_.push_back(f);
    }

    return fills;
}

std::vector<Trade> OrderBook::match(Order& incomingOrder) {
    std::vector<Trade> fills;
    
    auto run_match = [&](auto& opposite, bool is_buy) {
        while (incomingOrder.quantity>0 && !opposite.empty()) {
            auto best_it = opposite.begin();
            double best_price = best_it->first;

            if (is_buy && best_price > incomingOrder.price
            || !is_buy && best_price < incomingOrder.price) {
                break;
            }

            auto& opposite_orders = best_it->second;
            while(incomingOrder.quantity>0 && !opposite_orders.empty()) {
                auto& opposite_order = opposite_orders.front();
                int fill_quantity = std::min(opposite_order.quantity, incomingOrder.quantity);

                opposite_order.quantity -= fill_quantity;
                incomingOrder.quantity -= fill_quantity;

                Trade t = Trade();
                t.fill_price = opposite_order.price;
                t.quantity = fill_quantity;
                t.buy_id = is_buy ? incomingOrder.id : opposite_order.id;
                t.sell_id = is_buy ? opposite_order.id : incomingOrder.id;
                fills.push_back(t);

                if (opposite_order.quantity==0) {
                    opposite_orders.pop_front();
                }
            }

            if (opposite_orders.empty()) {
                opposite.erase(best_price);
            }
        }
    };
    
    if(incomingOrder.side == Side::BUY) {
        run_match(asks_, true);
    } else {
        run_match(bids_, false);
    }

    return fills;
}

void OrderBook::print_book() const {
    std::cout << "\n\n==== ORDER BOOK ====\n";

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