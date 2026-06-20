#pragma once

#include "order.hpp"

struct Trade {
    uint64_t buy_id;
    uint64_t sell_id;
    double fill_price;
    int quantity;
};