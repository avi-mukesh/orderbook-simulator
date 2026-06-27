#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "orderbook.hpp"
#include <optional>

TEST_CASE("testing orderbook creation") {
    OrderBook book("BTCUSD");

    CHECK(book.spread() == 0.0);
    CHECK(book.best_ask() == std::nullopt);
    CHECK(book.best_bid() == std::nullopt);

    
    SUBCASE("limit orders resting when spread isn't crossed, no trades executed") {
        book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));
        CHECK(book.best_ask() == 1000);
        CHECK(book.best_bid() == std::nullopt);

        book.add_order(Order(950, Side::SELL, 5, "BTCUSD"));
        CHECK(book.best_ask() == 950);
        CHECK(book.best_bid() == std::nullopt);
        
        book.add_order(Order(900, Side::BUY, 5, "BTCUSD"));
        CHECK(book.best_ask() == 950);
        CHECK(book.best_bid() == 900);

        CHECK(book.trade_log().size() == 0);
    }
    
    
    SUBCASE("a sell order crosses the spread exactly, fully matching one bid on the book, one trade is created with correct properties") {
        auto buy_id = book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));
        book.add_order(Order(900, Side::BUY, 2, "BTCUSD"));
        book.add_order(Order(850, Side::BUY, 3, "BTCUSD"));
        
        CHECK(book.trade_log().size() == 0);
        
        auto sell_id = book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));

        auto trade_log = book.trade_log();
        CHECK(trade_log.size() == 1);
        CHECK(trade_log[0].buy_id == buy_id);
        CHECK(trade_log[0].sell_id == sell_id);
        CHECK(trade_log[0].quantity == 5);
        CHECK(trade_log[0].fill_price == 1000);
    }

    SUBCASE("a sell order crosses the spread, exceeding it, fully matching one bid on the book, one trade is created with fill_price equal to the resting order's price") {
        auto buy_id = book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));
        book.add_order(Order(900, Side::BUY, 2, "BTCUSD"));
        book.add_order(Order(850, Side::BUY, 3, "BTCUSD"));
        
        CHECK(book.trade_log().size() == 0);
        
        auto sell_id = book.add_order(Order(950, Side::SELL, 5, "BTCUSD"));

        auto trade_log = book.trade_log();
        CHECK(trade_log.size() == 1);
        CHECK(trade_log[0].buy_id == buy_id);
        CHECK(trade_log[0].sell_id == sell_id);
        CHECK(trade_log[0].quantity == 5);
        CHECK(trade_log[0].fill_price == 1000);
    }

    SUBCASE("a sell order crosses the spread, matching two orders, generates two trades") {
        auto buy_id_1 = book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));
        auto buy_id_2 = book.add_order(Order(900, Side::BUY, 2, "BTCUSD"));
        book.add_order(Order(850, Side::BUY, 3, "BTCUSD"));
        
        CHECK(book.trade_log().size() == 0);
        
        auto sell_id = book.add_order(Order(900, Side::SELL, 7, "BTCUSD"));

        auto trade_log = book.trade_log();
        CHECK(trade_log.size() == 2);
        CHECK(trade_log[0].buy_id == buy_id_1);
        CHECK(trade_log[0].sell_id == sell_id);
        CHECK(trade_log[0].quantity == 5);
        CHECK(trade_log[0].fill_price == 1000);

        CHECK(trade_log[1].buy_id == buy_id_2);
        CHECK(trade_log[1].sell_id == sell_id);
        CHECK(trade_log[1].quantity == 2);
        CHECK(trade_log[1].fill_price == 900);
    }

    SUBCASE("a sell order crosses the spread, matching one order, with quantity less than the resting order's quantity, the fill_quantity is the incoming order's quantity") {
        auto buy_id = book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));
        book.add_order(Order(900, Side::BUY, 2, "BTCUSD"));
        book.add_order(Order(850, Side::BUY, 3, "BTCUSD"));
        
        CHECK(book.trade_log().size() == 0);
        
        auto sell_id = book.add_order(Order(1000, Side::SELL, 3, "BTCUSD"));

        auto trade_log = book.trade_log();
        CHECK(trade_log.size() == 1);
        CHECK(trade_log[0].buy_id == buy_id);
        CHECK(trade_log[0].sell_id == sell_id);
        CHECK(trade_log[0].quantity == 3);
        CHECK(trade_log[0].fill_price == 1000);
    }


    SUBCASE("large order partially fills, and the remainder rests") {
        auto sell_id = book.add_order(Order(1000, Side::SELL, 7, "BTCUSD"));
        
        CHECK(book.trade_log().size() == 0);
        
        auto buy_id = book.add_order(Order(1200, Side::BUY, 10, "BTCUSD"));

        auto trade_log = book.trade_log();
        CHECK(trade_log.size() == 1);
        CHECK(trade_log[0].buy_id == buy_id);
        CHECK(trade_log[0].sell_id == sell_id);
        CHECK(trade_log[0].quantity == 7);
        CHECK(trade_log[0].fill_price == 1000);

        // order wasn't fully filled so it rests (quantity 3 left over)
        CHECK(book.quantity_at(Side::BUY, 1200)==3);
    }

    SUBCASE("canceling a resting order removes it from the book") {
        auto buy_id = book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));
        CHECK(book.best_bid() == 1000);
        CHECK(book.quantity_at(Side::BUY, 1000) == 5);

        bool cancelled = book.cancel_order(buy_id);
        CHECK(cancelled == true);

        CHECK(book.best_bid() == std::nullopt);
        CHECK(book.quantity_at(Side::BUY, 1000) == 0);
    }

    SUBCASE("cancelling an already cancelled order fails") {
        auto buy_id = book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));
        CHECK(book.best_bid() == 1000);
        CHECK(book.quantity_at(Side::BUY, 1000) == 5);
        
        bool cancelled = book.cancel_order(buy_id);
        CHECK(cancelled == true);
        
        CHECK(book.best_bid() == std::nullopt);
        CHECK(book.quantity_at(Side::BUY, 1000) == 0);
        
        
        cancelled = book.cancel_order(buy_id);
        CHECK(cancelled == false);
    }
    
    SUBCASE("cancelling an order that didn't exist fails") {
        bool cancelled = book.cancel_order(999);
        CHECK(cancelled == false);
    }
    
    SUBCASE("cancelling a filled order returns false") {
        auto sell_id = book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));
        
        book.add_order(Order(1000, Side::BUY, 2, "BTCUSD"));
        book.add_order(Order(1100, Side::BUY, 4, "BTCUSD"));
        
        bool cancelled = book.cancel_order(sell_id);
        CHECK(cancelled == false);
    }
    
    SUBCASE("market order sweeps all orders on the opposite side of the book until filled") {
        auto sell_id_1 = book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));
        book.add_order(Order(1010, Side::SELL, 5, "BTCUSD"));
        book.add_order(Order(1020, Side::SELL, 5, "BTCUSD"));
        book.add_order(Order(1030, Side::SELL, 5, "BTCUSD"));
        auto sell_id_5 = book.add_order(Order(1040, Side::SELL, 5, "BTCUSD"));

        auto buy_id = book.add_order(Order(Side::BUY, 25, "BTCUSD"));
        
        auto trade_log = book.trade_log();

        CHECK(trade_log.size() == 5);
        CHECK(trade_log[0].buy_id == buy_id);
        CHECK(trade_log[0].sell_id == sell_id_1);
        CHECK(trade_log[0].quantity == 5);
        CHECK(trade_log[0].fill_price == 1000);
        
        CHECK(trade_log[4].buy_id == buy_id);
        CHECK(trade_log[4].sell_id == sell_id_5);
        CHECK(trade_log[4].quantity == 5);
        CHECK(trade_log[4].fill_price == 1040);
    }

    SUBCASE("market order, if only partially filled, doesn't rest on the book") {
        book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));
        book.add_order(Order(1010, Side::SELL, 5, "BTCUSD"));
        book.add_order(Order(1020, Side::SELL, 5, "BTCUSD"));
        book.add_order(Order(1030, Side::SELL, 5, "BTCUSD"));
        book.add_order(Order(1040, Side::SELL, 5, "BTCUSD"));

        book.add_order(Order(Side::BUY, 30, "BTCUSD"));
        
        CHECK(book.trade_log().size() == 5);
        CHECK(book.best_bid() == std::nullopt);
    }

    SUBCASE("IOC order if partially filled, won't rest on the book") {
        auto sell_id_1 = book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));
        auto sell_id_2 = book.add_order(Order(1010, Side::SELL, 5, "BTCUSD"));

        auto buy_id = book.add_order(Order(OrderType::IOC, 1200, Side::BUY, 7, "BTCUSD"));
        
        auto trade_log = book.trade_log();
        CHECK(trade_log.size() == 2);
        CHECK(trade_log[0].buy_id == buy_id);
        CHECK(trade_log[0].sell_id == sell_id_1);
        CHECK(trade_log[0].quantity == 5);
        CHECK(trade_log[0].fill_price == 1000);

        CHECK(trade_log[1].buy_id == buy_id);
        CHECK(trade_log[1].sell_id == sell_id_2);
        CHECK(trade_log[1].quantity == 2);
        CHECK(trade_log[1].fill_price == 1010);

        CHECK(book.best_bid() == std::nullopt);
    }

    SUBCASE("FOK order if can fill completely, gets filled") {
        auto sell_id_1 = book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));
        auto sell_id_2 = book.add_order(Order(1010, Side::SELL, 5, "BTCUSD"));

        auto buy_id = book.add_order(Order(OrderType::FOK, 1200, Side::BUY, 7, "BTCUSD"));
        
        auto trade_log = book.trade_log();
        CHECK(trade_log.size() == 2);
        CHECK(trade_log[0].buy_id == buy_id);
        CHECK(trade_log[0].sell_id == sell_id_1);
        CHECK(trade_log[0].quantity == 5);
        CHECK(trade_log[0].fill_price == 1000);

        CHECK(trade_log[1].buy_id == buy_id);
        CHECK(trade_log[1].sell_id == sell_id_2);
        CHECK(trade_log[1].quantity == 2);
        CHECK(trade_log[1].fill_price == 1010);
        
        CHECK(book.best_bid() == std::nullopt);
    }

    SUBCASE("FOK order if can't fill completely, doesn't rest on the book") {
        auto sell_id_1 = book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));
        auto sell_id_2 = book.add_order(Order(1010, Side::SELL, 5, "BTCUSD"));

        auto buy_id = book.add_order(Order(OrderType::FOK, 1200, Side::BUY, 200, "BTCUSD"));
        
        auto trade_log = book.trade_log();
        CHECK(trade_log.size() == 0);
        
        CHECK(book.best_bid() == std::nullopt);
    }

    SUBCASE("amending the price of an order removes it from the book and readds it with a new id") {
        auto buy_id = book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));
        CHECK(book.quantity_at(Side::BUY, 1000) == 5);
        CHECK(book.quantity_at(Side::BUY, 1200) == 0);
        
        auto new_buy_id = book.amend_order(buy_id, 1200, 5);
        CHECK(book.quantity_at(Side::BUY, 1000) == 0);
        CHECK(book.quantity_at(Side::BUY, 1200) == 5);
        
        CHECK(buy_id != new_buy_id);
    }
    
    SUBCASE("amending an order buy increasing its quantity removes it from the book and readds it with a new id") {
        auto buy_id = book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));
        CHECK(book.quantity_at(Side::BUY, 1000) == 5);

        auto new_buy_id = book.amend_order(buy_id, 1000, 8);
        CHECK(book.quantity_at(Side::BUY, 1000) == 8);
        
        CHECK(buy_id != new_buy_id);
    }
        
    SUBCASE("amending an order buy decreasing its quantity updates in place and keeps the same id") {
        auto buy_id = book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));
        CHECK(book.quantity_at(Side::BUY, 1000) == 5);

        auto new_buy_id = book.amend_order(buy_id, 1000, 1);
        CHECK(book.quantity_at(Side::BUY, 1000) == 1);
        
        CHECK(buy_id == new_buy_id);
    }

    SUBCASE("amending an order buy decreasing its quantity, then matching results it still being filled first") {
        auto sell_id_1 = book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));
        auto sell_id_2 = book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));
        CHECK(book.quantity_at(Side::SELL, 1000) == 10);

        sell_id_1 = book.amend_order(sell_id_1, 1000, 3);

        book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));
        
        auto trade_log = book.trade_log();

        CHECK(trade_log.size() == 2);
        CHECK(trade_log[0].sell_id == sell_id_1);
        CHECK(trade_log[1].sell_id == sell_id_2);
    }


    SUBCASE("amending an order buy increasing its quantity, then matching results it being filled second as it is deprioritsed") {
        auto sell_id_1 = book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));
        auto sell_id_2 = book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));
        CHECK(book.quantity_at(Side::SELL, 1000) == 10);

        sell_id_1 = book.amend_order(sell_id_1, 1000, 6);

        book.add_order(Order(1000, Side::BUY, 10, "BTCUSD"));
        
        auto trade_log = book.trade_log();

        CHECK(trade_log.size() == 2);
        CHECK(trade_log[0].sell_id == sell_id_2);
        CHECK(trade_log[1].sell_id == sell_id_1);
    }

    SUBCASE("price-time priority is respected so earlier order at a level fills first") {
        auto sell_id_1 = book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));
        auto sell_id_2 = book.add_order(Order(1000, Side::SELL, 5, "BTCUSD"));

        book.add_order(Order(1000, Side::BUY, 5, "BTCUSD"));

        auto trade_log = book.trade_log();
        CHECK(trade_log.size() == 1);
        CHECK(trade_log[0].sell_id == sell_id_1);
        CHECK(book.quantity_at(Side::SELL, 1000) == 5);
    }
}