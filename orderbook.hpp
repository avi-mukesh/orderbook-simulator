#pragma once

#include "order.hpp"
#include <vector>
#include <optional>
#include <map>
#include <deque>
#include <functional> // for std::greater
#include <cstdint>

struct Trade {
    uint64_t buy_id;
    uint64_t sell_id;
    double fill_price;
    int quantity;
};

inline std::ostream & operator<<(std::ostream & str,const Trade & t) {
    str << "FILL " << t.quantity << " @ " << t.fill_price;
    return str;
}

class OrderBook {
public: 
    std::vector<Trade> add_order(Order order);
    double spread() const;
    void print_book() const;
    std::optional<double> best_bid() const;
    std::optional<double> best_ask() const;
    const std::vector<Trade>& trade_log() const {return trade_log_; }

    // without explicit, e.g. OrderBook book = "BTCUSD" would be allowed
    explicit OrderBook(std::string symbol);

private:
    uint64_t next_id_ = 0;

    std::vector<Trade> trade_log_;
    std::string symbol_;

    std::map<double, std::deque<Order>, std::greater<double>> bids_;
    std::map<double, std::deque<Order>> asks_;

    std::vector<Trade> match(Order& incomingOrder);
};