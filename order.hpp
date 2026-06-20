#pragma once

#include<iostream>
#include<cstdint>
#include<string>
#include<utility>

enum class Side {BUY, SELL};

struct Order {
    uint64_t id;
    double price;
    Side side;
    int quantity;
    std::string symbol;

    Order(uint16_t id, double price, Side side, int quantity, std::string symbol) : 
            id(id), price(price), side(side), quantity(quantity), symbol(std::move(symbol)) {}
};

inline std::ostream& operator<<(std::ostream & str, Order const & order) {
    str << "ID: " << order.id << " Price: " << order.price;
    return str;
}