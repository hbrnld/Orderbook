#pragma once

#include <iostream>
#include <iomanip>
#include <format>
#include <list>
#include <map>
#include <unordered_map>
#include <numeric>
#include <algorithm>

#include "Datatypes.h"
#include "Order.h"
#include "OrderType.h"
#include "OrderModify.h"
#include "OrderBookLevelInfo.h"
#include "Trade.h"

class Orderbook {
private: 
    struct OrderEntry {
        OrderPointer order_ {nullptr};
        OrderPointers::iterator location_;
    };

    struct LevelData {
        Quantity quantity_{};
        Quantity count_{};

        enum class Action {
            Add,
            Remove,
            Match
        };
    };

    std::map<Price, OrderPointers, std::greater<Price>> bids_;  // Price maps to list of order pointers
    std::map<Price, OrderPointers, std::less<Price>> asks_;     // Price maps to list of order pointers
    std::unordered_map<OrderId, OrderEntry> orders_;            // OrderId maps to OrderEntry
    std::unordered_map<Price, LevelData> data_;                 // Price maps to LevelData

    bool CanMatch(Side side, Price price) const;
    bool CanFullyFill(Side side, Price price, Quantity quantity) const;
    void UpdateLevelData(Price price, Quantity quantity, LevelData::Action action);
    Trades MatchOrders();
    
public: 
    Orderbook() = default;                      // Constructor
    Orderbook(const Orderbook&) = delete;       // Copy constructor
    void operator=(const Orderbook&) = delete;  // Copy assignment
    Orderbook(Orderbook&&) = delete;            // Move constructor
    void operator=(Orderbook&&) = delete;       // Move assignment    
    ~Orderbook() = default;                     // Destructor

    Trades AddOrder(OrderPointer order);
    void CancelOrder(OrderId orderId);
    Trades ModifyOrder(OrderModify order);

    std::size_t Size() const;
    std::size_t BidSize() const;
    std::size_t AskSize() const;
    OrderbookLevelInfos GetOrderInfos() const;
    void PrintOrderbook() const;
};