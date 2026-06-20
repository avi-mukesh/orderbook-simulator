#pragma once

#include<iostream>
#include<cstdint>
#include<string>
#include<utility>

enum class Side {BUY, SELL};

struct Order {
    uint64_t id = 0;
    double price;
    Side side;
    int quantity;
    std::string symbol;

    Order(double price, Side side, int quantity, std::string symbol) : 
            price(price), side(side), quantity(quantity), symbol(std::move(symbol)) {}
};

inline std::ostream& operator<<(std::ostream & str, Order const & order) {
    str << "ID: " << order.id << " Price: " << order.price;
    return str;
}