#pragma once

#include <iostream>
#include "OrderBook.h"
#include "InputHandler.h"

struct Interface {
private: 
    bool isNumeric(const std::string& str) const;
    int ToNumber(const std::string& str) const;
    
    Side SelectSide() const;
    OrderType SelectOrderType() const;
    Price SelectPrice() const;
    Quantity SelectQuantity() const;
    OrderId SelectOrderId() const;

    void PrintMenu() const;
    void PressAddOrder(Orderbook& orderbook, InputHandler& handler) const;
    void PressModifyOrder(Orderbook& orderbook, InputHandler& handler) const;
    void PressCancelOrder(Orderbook& orderbook, InputHandler& handler) const;
public:
    void Run(Orderbook& orderbook, InputHandler& handler) const;
};