#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <charconv>
#include "OrderBook.h"

/* 
Input file format

A B GoodUntilCancel 100 50 1        // Add Side OrderType Price Quantity OrderId
C 1                                 // Cancel OrderId
M 1 S 100 10                        // Modify OrderId Side Price Quantity 
*/

enum class ActionType {
    Add,
    Modify,
    Cancel
};

struct Info {
    ActionType action_;
    Side side_;
    OrderType orderType_;
    Price price_;
    Quantity quantity_;
    OrderId orderId_;
};

using Infos = std::vector<Info>;

struct InputHandler {
private:
    std::uint32_t ToNumber(const std::string_view& str) const;
    std::vector<std::string_view> Split(const std::string_view& str, char delimiter) const;

    Side TryParseSide(const std::string_view& str) const;
    OrderType TryParseOrderType(const std::string_view& str) const;
    Price TryParsePrice(const std::string_view& str) const;
    Quantity TryParseQuantity(const std::string_view& str) const;
    OrderId TryParseOrderId(const std::string_view& str) const;
    bool TryParseInfo(const std::string_view& str, Info& info) const;

public: 
    void ReadFromFile(const std::filesystem::path& path, Orderbook& orderbook) const;
    void ReadFromInput(Orderbook& orderbook) const;
    void ProcessInfo(const Infos& infos, Orderbook& orderbook) const;
};

