#pragma once

#include <cstdint>
#include <vector>
#include <limits>

enum class Side {
    Buy,
    Sell
};

using Price = std::int32_t;
using Quantity = std::int32_t;
using OrderId = std::uint64_t;

struct Constants {
    static const Price InvalidPrice = std::numeric_limits<Price>::quiet_NaN();
};