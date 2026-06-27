A simple order book simulator.

# Main

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

# Benchmarking

`benchmark_mixed.cpp` and `benchmark_insert.cpp` are used as benchmark harnesses, to demo speed.

Compile using `g++ benchmark_mixed.cpp orderbook.cpp -O2 -std=c++17 -o benchmark_mixed`

Run using `./benchmark_mixed`

Compile using `g++ benchmark_insert.cpp orderbook.cpp -O2 -std=c++17 -o benchmark_insert`

Run using `./benchmark_insert`

`benchmark_insert` measures purely the `add_order` method for inserting orders all on one side. Insertion is O(log n) in number of price levels in `std::map`. easured per-order latency rises from ~120ns to ~880ns as the book scales to 10M orders — steeper than log alone, as the node-based `std::map`/`std::list` lose cache locality at scale.

`benchmark_mixes` measures a random mixture of order inserts and matches, representing a more realistic workload.

# Tests

Unit tests use [doctest](https://github.com/doctest/doctest).

Compile using ​`g++ tests.cpp orderbook.cpp -std=c++17 -o tests`

Run using ​`./tests`
