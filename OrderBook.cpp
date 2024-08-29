#include "OrderBook.h"
#include "OrderType.h"
#include "Order.h"

bool Orderbook::CanMatch(Side side, Price price) const {
    if (side == Side::Buy) {
        return !asks_.empty() && asks_.begin()->first <= price;
    } else {
        return !bids_.empty() && bids_.begin()->first >= price;
    }
}

bool Orderbook::CanFullyFill(Side side, Price price, Quantity quantity) const {
	if (!CanMatch(side, price))
		return false;

	std::optional<Price> threshold;

	if (side == Side::Buy) {
		const auto [askPrice, _] = *asks_.begin();
		threshold = askPrice;
	} else {
		const auto [bidPrice, _] = *bids_.begin();
		threshold = bidPrice;
	}

	for (const auto& [levelPrice, levelData] : data_) {
        if (threshold.has_value() && ((side == Side::Buy && threshold.value() > levelPrice) || (side == Side::Sell && threshold.value() < levelPrice)))
            continue;

		if ((side == Side::Buy && levelPrice > price) || (side == Side::Sell && levelPrice < price))
            continue;

		if (quantity <= levelData.quantity_)
			return true;

		quantity -= levelData.quantity_;
	}
	return false;
}

void Orderbook::UpdateLevelData(Price price, Quantity quantity, LevelData::Action action) {
	auto& data = data_[price];

    // Update order count
	data.count_ += action == LevelData::Action::Remove ? -1 : action == LevelData::Action::Add ? 1 : 0;

    // Update order quantity
	if (action == LevelData::Action::Remove || action == LevelData::Action::Match) {
		data.quantity_ -= quantity;
	} else {
		data.quantity_ += quantity;
	}

    // Remove price level if no orders
	if (data.count_ == 0)
		data_.erase(price);
}

Trades Orderbook::MatchOrders() {
    Trades trades; 
    trades.reserve(orders_.size());
    
    while (true) {
        if (bids_.empty() || asks_.empty())
            break;
        
        auto& [bidPrice, bids] = *bids_.begin();    // Get the highest bid price
        auto& [askPrice, asks] = *asks_.begin();    // Get the lowest ask price

        if (bidPrice < askPrice) {
            break;
        }

        while (!bids.empty() && !asks.empty()) {
            auto bid = bids.front();
            auto ask = asks.front();

            Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

            std::cout << std::format("Matching bid {} with ask {} for quantity {}", bid->GetPrice(), ask->GetPrice(), quantity) << std::endl;

            bid->Fill(quantity);
            ask->Fill(quantity);

            if (bid->IsFilled()) {
                orders_.erase(bid->GetOrderId());
                bids.pop_front();
            }

            if (ask->IsFilled()) {
                orders_.erase(ask->GetOrderId());
                asks.pop_front();
            }

            trades.push_back(Trade{
                TradeInfo{bid->GetOrderId(), bid->GetPrice(), quantity},  // Append bid trade
                TradeInfo{ask->GetOrderId(), ask->GetPrice(), quantity}   // Append ask trade
            });

            UpdateLevelData(bid->GetPrice(), quantity, bid->IsFilled() ? LevelData::Action::Remove : LevelData::Action::Match);
            UpdateLevelData(ask->GetPrice(), quantity, ask->IsFilled() ? LevelData::Action::Remove : LevelData::Action::Match);
        }

        if (bids.empty()) {
            bids_.erase(bidPrice);
            data_.erase(bidPrice);
        }

        if (asks.empty()) {
            asks_.erase(askPrice);
            data_.erase(askPrice);
        }
    }

    // Cancel FillAndKill buy orders
    if (!bids_.empty()) {
        auto& [_, bids] = *bids_.begin();
        auto& order = bids.front();
        if (order->GetOrderType() == OrderType::FillAndKill) {
            std::cout << "Cancelling FillAndKill order" << std::endl;
            CancelOrder(order->GetOrderId());
        }
    }

    // Cancel FillAndKill sell orders
    if (!asks_.empty()) {
        auto& [_, asks] = *asks_.begin();
        auto& order = asks.front();
        if (order->GetOrderType() == OrderType::FillAndKill) {
            std::cout << "Cancelling FillAndKill order" << std::endl;
            CancelOrder(order->GetOrderId());
        }
    }

    return trades; 
}

Trades Orderbook::AddOrder(OrderPointer order) {
    if (orders_.contains(order->GetOrderId())) {
        std::cout << std::format("OrderId {} already exists", order->GetOrderId()) << std::endl;
        return {};
    }

    // Market order is effectively buying/selling avaliable orders until filled
    if (order->GetOrderType() == OrderType::Market) {
        if (order->GetSide() == Side::Buy && !asks_.empty()) {
            const auto& [worstAsk, _] = *asks_.rbegin();
            order->MakeGoodUntilCancel(worstAsk);              // If not filled, place order at worstAsk
        } else if (order->GetSide() == Side::Sell && !bids_.empty()){
            const auto& [worstBid, _] = *bids_.rbegin();
            order->MakeGoodUntilCancel(worstBid);              // If not filled, place order at worstBid
        } else {
            return {};
        }
    }

    if (order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice())) {
        std::cout << std::format("No match for FillAndKill order {}", order->GetOrderId()) << std::endl;
        return {};
    }

    if (order->GetOrderType() == OrderType::FillOrKill && !CanFullyFill(order->GetSide(), order->GetPrice(), order->GetInitialQuantity())) {
        std::cout << std::format("Cannot fully fill FillOrKill order {}", order->GetOrderId()) << std::endl;
        return {};
    }

    OrderPointers::iterator iterator;

    if (order->GetSide() == Side::Buy) {
        auto& orders = bids_[order->GetPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.end());
    } else {
        auto& orders = asks_[order->GetPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.end());
    }

    orders_.insert({order->GetOrderId(), OrderEntry{order, iterator}});
    UpdateLevelData(order->GetPrice(), order->GetInitialQuantity(), LevelData::Action::Add);

    return MatchOrders();
}

void Orderbook::CancelOrder(OrderId orderId) {
    if (!orders_.contains(orderId)) {
        std::cout << std::format("OrderId {} does not exist", orderId) << std::endl;
        return;
    }

    const auto [order, iterator] = orders_.at(orderId);
    orders_.erase(orderId);

    if (order->GetSide() == Side::Sell) {
        auto price = order->GetPrice();
        auto& orders = asks_.at(price);
        orders.erase(iterator);
        if (orders.empty())
            asks_.erase(price);
    } else {
        auto price = order->GetPrice();
        auto& orders = bids_.at(price);
        orders.erase(iterator);
        if (orders.empty())
            bids_.erase(price);
    }

    UpdateLevelData(order->GetPrice(), order->GetRemainingQuantity(), LevelData::Action::Remove);
}

Trades Orderbook::ModifyOrder(OrderModify order) {
    OrderType orderType;

    if (!orders_.contains(order.GetOrderId())) {
        std::cout << "Order does not exist" << std::endl;
        return {};
    }

    const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
    orderType = existingOrder->GetOrderType();
    
    CancelOrder(order.GetOrderId());
    return AddOrder(order.ToOrderPointer(orderType));
}

std::size_t Orderbook::Size() const { 
    return orders_.size(); 
}

std::size_t Orderbook::BidSize() const {
    return bids_.size();
}

std::size_t Orderbook::AskSize() const {
    return asks_.size();
}

OrderbookLevelInfos Orderbook::GetOrderInfos() const {
    LevelInfos bidInfos, askInfos;
    bidInfos.reserve(orders_.size());
    askInfos.reserve(orders_.size());

    auto CreateLevelInfos = [](Price price, const OrderPointers& orders) {
        Quantity quantity = std::accumulate(orders.begin(), orders.end(), (Quantity)0, [](Quantity acc, const OrderPointer& order) {
            return acc + order->GetRemainingQuantity();
        });
        return LevelInfo{price, quantity};
    };

    for (const auto& [price, orders] : bids_) {
        bidInfos.push_back(CreateLevelInfos(price, orders));
    }
    for (const auto& [price, orders] : asks_) {
        askInfos.push_back(CreateLevelInfos(price, orders));
    }

    return OrderbookLevelInfos{bidInfos, askInfos}; 
}

void Orderbook::PrintOrderbook() const {
    OrderbookLevelInfos orderbookLevelInfos = GetOrderInfos();
    bool hasAsks = !orderbookLevelInfos.GetAsks().empty();
    bool hasBids = !orderbookLevelInfos.GetBids().empty();
    
    std::cout << "\n======== Orderbook ========\n" << std::endl;

    if (hasAsks) {
        for (auto it = orderbookLevelInfos.GetAsks().rbegin(); it != orderbookLevelInfos.GetAsks().rend(); ++it) {
            const auto& [price, quantity] = *it;
            std::cout << "\t\033[1;31m"
                    << "$" << std::setw(4) << std::fixed << std::setprecision(2) 
                    << price << std::setw(5) << quantity << "\033[0m ";
            
            for (int i = 0; i < quantity/10; i++)
                std::cout << "█";

            std::cout << std::endl;
        }
    } else {
        std::cout << "\t  \033[1;31mNo Asks\033[0m\n" << std::endl;
    }

    if (hasAsks && hasBids) {
        Price bestBid = orderbookLevelInfos.GetBids().begin()->price_;
        Price bestAsk = orderbookLevelInfos.GetAsks().begin()->price_;
        std::cout << "\033[1;33m"
                << "\n------- Spread: $" << std::setprecision(2) << static_cast<double>(bestAsk-bestBid) 
                << " -------" << "\033[0m\n" << std::endl;
    }

    if (hasBids) {
        for (const auto& [price, quantity] : orderbookLevelInfos.GetBids()) {
            std::cout << "\t\033[1;32m"
                    << "$" << std::setw(4) << std::fixed << std::setprecision(2) 
                    << price << std::setw(5) << quantity << "\033[0m ";

            for (int i = 0; i < quantity/10; i++)
                std::cout << "█";
            
            std::cout << std::endl;
        }
    } else {
        std::cout << "\n\t  \033[1;32mNo Bids\033[0m" << std::endl;
    }

    std::cout << std::endl;
}