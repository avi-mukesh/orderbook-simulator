#include "orderbook.hpp"
#include <optional>
#include <list>
#include <algorithm>
#include <iterator>

OrderBook::OrderBook(std::string symbol) : symbol_(std::move(symbol)) {}

uint64_t OrderBook::add_order(Order order) {
    if(order.symbol != symbol_) {
        std::cerr << "\nThis order book is for " << symbol_ << ". " << order.symbol << " is not supported.\n";
        return {};
    }

    order.id = ++next_id_;

    // FOK orders don't rest (fill all or nothing)
    if (order.type == OrderType::FOK && !can_fill_fully(order)) return order.id;

    std::vector<Trade> fills = match(order);

    // rest order only if it's a limit order that has quantity left over after matching
    // market orders don't rest
    // IOC orders don't rest (unfilled part gets discarded)
    if (order.type==OrderType::LIMIT && order.quantity > 0) {
        double p = order.price;
        Side s = order.side;
        uint64_t id = order.id;
        if(s==Side::BUY) {
            // if key p doesn't exist, it is auto created
            // also, using std::move here to avoid copying the whole Order struct
            // trying to access e.g. order.symbol after the move does not work
            bids_[p].push_back(std::move(order));
        } else {
            asks_[p].push_back(std::move(order));
        }

        OrderLocation location = OrderLocation();
        location.side = s;
        location.price_level = p;
        location.it = std::prev(s==Side::BUY ? bids_[p].end() : asks_[p].end());
        
        order_locations_by_id_[id] = location;
    }

    for (const auto& f : fills) {
        trade_log_.push_back(f);
    }

    return order.id;
}

// check for FOK (fill or kill) orders
bool OrderBook::can_fill_fully(Order& incoming) {
    auto check = [&](auto& opposite, bool is_buy) {
        int quantity_to_fill = incoming.quantity;
        auto opposite_it = opposite.begin();

        while(quantity_to_fill > 0 && opposite_it != opposite.end()) {
            double price = opposite_it->first;
            auto& opposite_orders = opposite_it->second;

            if (is_buy && price > incoming.price
                || !is_buy && price < incoming.price) {
                    break;
                }

            auto order_it = opposite_orders.begin();
            
            while(quantity_to_fill > 0 && order_it != opposite_orders.end()) {
                int fill_quantity = std::min(quantity_to_fill, (*order_it).quantity);
                quantity_to_fill -= fill_quantity;
                
                if (quantity_to_fill<=0) {
                    return true;
                }
                order_it++;
            }
            opposite_it++;
        }
        return false;
    };

    if (incoming.side == Side::BUY) {
        return check(asks_, true);
    } else {
        return check(bids_, false);
    }
}

bool OrderBook::cancel_order(uint64_t order_id) {
    auto it = order_locations_by_id_.find(order_id);

    // if there is no OrderLocation associated with this order id
    if (it == order_locations_by_id_.end()) {
        return false;
    }

    // remove order from the appropriate map at the appropriate price
    OrderLocation location = it->second;
    auto& level = location.side == Side::BUY ? bids_[location.price_level] : asks_[location.price_level];
    level.erase(location.it);
    if (level.empty()) {
        if (location.side == Side::BUY) {
            bids_.erase(location.price_level);
        } else {
            asks_.erase(location.price_level);
        }
    }

    // remove from the map of order id -> OrderLocation also
    order_locations_by_id_.erase(order_id);

    return true;
}

uint64_t OrderBook::amend_order(uint64_t order_id, double new_price, int new_quantity) {
    auto it = order_locations_by_id_.find(order_id);

    // if there is no OrderLocation associated with this order id
    if (it == order_locations_by_id_.end()) {
        return 0;
    }

    OrderLocation location = it->second;
    // grab order by reference, so we can modify it in-place
    Order& order_to_amend = *location.it;

    // nothing to update if price and quantity haven't changed
    if (order_to_amend.price == new_price && order_to_amend.quantity == new_quantity) return true;

    // if price is changed, then order belongs on new price level, so remove
    // if new quantity is higher than current quantity, then it should be deprioritised so remove from list and readd at the end
    // otherwise, the new quantity is lower, so we can keep the order's priority in the queue and just update it in-place
    if (order_to_amend.price != new_price || order_to_amend.quantity < new_quantity) {
        Order amended_order = Order(new_price, order_to_amend.side, new_quantity, order_to_amend.symbol);
        cancel_order(order_id);
        return add_order(amended_order);
    } else {
        order_to_amend.quantity = new_quantity;
        return order_to_amend.id;
    }
}

std::vector<Trade> OrderBook::match(Order& incomingOrder) {
    std::vector<Trade> fills;
    
    auto run_match = [&](auto& opposite, bool is_buy) {
        while (incomingOrder.quantity>0 && !opposite.empty()) {
            auto best_it = opposite.begin();
            double best_price = best_it->first;

            if (incomingOrder.type==OrderType::LIMIT
                || incomingOrder.type==OrderType::IOC
                || incomingOrder.type==OrderType::FOK) {
                    if (is_buy && best_price > incomingOrder.price
                    || !is_buy && best_price < incomingOrder.price) {
                        break;
                    }
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
                    // remove filled order from map of order id -> locations
                    order_locations_by_id_.erase(opposite_order.id);

                    // and remove filled order from book
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

    std::cout << "----- Spread: $" << this->spread() << " ----\n";

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

double OrderBook::spread() const {
    if (asks_.empty() || bids_.empty()) {
        return 0.0;
    }

    // best_ask and best_bid both return std::optional<double>
    // so the minus '-' operator isn't defined on them
    // need to dereference using *
    // alternative is to to do .value()
    return *best_ask() - *best_bid();
}