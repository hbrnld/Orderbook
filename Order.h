#pragma once

#include <list>
#include <format>
#include <iostream>
#include "Datatypes.h"
#include "OrderType.h"

class Order {
public: 
    Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity)
        : orderType_ {orderType}, 
          orderId_ {orderId}, 
          side_ {side}, 
          price_ {price}, 
          initialQuantity_ {quantity},
          remainingQuantity_ {quantity}
    { }

    Order(OrderId orderId, Side side, Quantity quantity)
        : Order(OrderType::Market, orderId, side, Constants::InvalidPrice, quantity)
    { }
    
    OrderId GetOrderId() const { return orderId_; }
    Side GetSide() const { return side_; }
    Price GetPrice() const { return price_; }
    OrderType GetOrderType() const { return orderType_; }
    Quantity GetInitialQuantity() const { return initialQuantity_; }
    Quantity GetRemainingQuantity() const { return remainingQuantity_; }
    Quantity GetFilledQuantity() const { return initialQuantity_ - remainingQuantity_; }

    bool IsFilled() const { return GetRemainingQuantity() == 0; }

    void Fill(Quantity quantity) {
        if (quantity > GetRemainingQuantity()) {
            throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity ({})", GetOrderId(), GetRemainingQuantity()));
        } else {
            remainingQuantity_ -= quantity;
        }   
    }

    void MakeGoodUntilCancel(Price price) {
        if (GetOrderType() != OrderType::Market)
            throw std::logic_error(std::format("Order ({}) must be a market order to be converted to GoodUntilCancel", GetOrderId()));

        price_ = price; 
        orderType_ = OrderType::GoodUntilCancel;
    }

private: 
    OrderType orderType_;
    OrderId orderId_;
    Side side_;
    Price price_;
    Quantity initialQuantity_;
    Quantity remainingQuantity_;
};

// Reference semantics
using OrderPointer = std::shared_ptr<Order>;    // Order can be stored in both orders dict and bid/ask dict
using OrderPointers = std::list<OrderPointer>;  // List of order pointers