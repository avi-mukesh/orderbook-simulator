A simple order book simulator.

`main.cpp` is to demo the functionality.

Compile using `g++ main.cpp orderbook.cpp -std=c++17 -o main`

Run using `./main`

Supports limit orders, market orders, IOC orders, and FOK orders.

- To place a limit order, create an `Order` with a price. Fills only at prices equal to or better than the limit price, and the remaining order rests on the book.
- To place a market order, create an `Order` with no price. Fills at any price and doesn't rest on the book.
- To place an IOC order, create an `Order` with a price, and type `OrderType::IOC`. Fills like limit orders, but if unfilled, gets discarded like market orders.
- To place a FOK order, create an `Order` with a price, and type `OrderType::FOK`. Fills like limit orders ONLY if we can fill the whole order. Otherwise, gets discarded completely.

Logs out a list of all trades that get executed.

It logs every executed trade, with the VWAP (Volume-Weighted Average Price) shown underneath.
