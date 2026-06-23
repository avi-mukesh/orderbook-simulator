#pragma once

#include "order.hpp"
#include <vector>
#include <optional>
#include <map>
#include <list>
#include <functional> // for std::greater
#include <cstdint> // for uint64_t

struct Trade {
    uint64_t buy_id;
    uint64_t sell_id;
    double fill_price;
    int quantity;
};

inline std::ostream & operator<<(std::ostream & str,const Trade & t) {
    str << "FILL " << "buy#" << t.buy_id << ", sell#" << t.sell_id << " " << t.quantity << " units @ " << t.fill_price;
    return str;
}

struct OrderLocation {
    Side side;
    double price_level;
    std::list<Order>::iterator it;
};

class OrderBook {
public: 
    uint64_t add_order(Order order);
    bool cancel_order(uint64_t order_id);
    double spread() const;
    void print_book() const;
    // the const at the end means the method promises not to modify any member variables of the class
    std::optional<double> best_bid() const;
    std::optional<double> best_ask() const;
    const std::vector<Trade>& trade_log() const {return trade_log_; }

    // without explicit, e.g. OrderBook book = "BTCUSD" would be allowed
    explicit OrderBook(std::string symbol);

private:
    uint64_t next_id_ = 0;

    std::vector<Trade> trade_log_;
    std::string symbol_;
    
    std::map<double, std::list<Order>, std::greater<double>> bids_;
    std::map<double, std::list<Order>> asks_;
    
    std::unordered_map<uint64_t, OrderLocation> order_locations_by_id_;

    std::vector<Trade> match(Order& incomingOrder);
};