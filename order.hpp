#pragma once

#include<iostream>
#include<cstdint>
#include<string>
#include<utility>

enum class Side {BUY, SELL};

// considered inheritance approach e.g. remove price from here, and then create a new LimitOrder struct that inherits Order
// but this would mean e.g. add_order(Order order) wouldn't work, since it would strip out the extra price field
// limit orders are the only ones that rest on the book anyway
// market orders don't rest if not filled
enum class OrderType {LIMIT, MARKET};

struct Order {
    uint64_t id = 0;
    double price = 0;
    Side side;
    int quantity;
    std::string symbol;
    OrderType type;

    Order(double price, Side side, int quantity, std::string symbol) : 
            price(price), type(OrderType::LIMIT), side(side), quantity(quantity), symbol(std::move(symbol)) {}
    // initialiser list so each variable is constructed directly with the right value
    // if in body, then default-constructed first, and then reassigned which is 2 steps 

    Order(Side side, int quantity, std::string symbol) : 
            type(OrderType::MARKET), side(side), quantity(quantity), symbol(std::move(symbol)) {}
};

inline std::ostream& operator<<(std::ostream & str, Order const & order) {
    str << "ID: " << order.id << " Price: " << order.price;
    return str;
}