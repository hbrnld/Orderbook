#include "InputHandler.h"
#include "OrderBook.h"

std::uint32_t InputHandler::ToNumber(const std::string_view& str) const {
    std::int64_t value{0};
    std::from_chars(str.data(), str.data() + str.size(), value);
    if (value < 0)
        throw std::logic_error("Value is below zero.");
    return static_cast<std::uint32_t>(value);
}

std::vector<std::string_view> InputHandler::Split(const std::string_view& str, char delimiter) const {
    std::vector<std::string_view> result;
    std::size_t start {}, end {};
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        std::size_t distance = end - start;
        auto substr = str.substr(start, distance);
        start = end + 1;
        result.push_back(substr);
    }
    result.push_back(str.substr(start));
    return result;
}

Side InputHandler::TryParseSide(const std::string_view& str) const {
    if (str == "B") {
        return Side::Buy;
    } else if (str == "S") {
        return Side::Sell;
    } else {
        throw std::logic_error("Invalid side");
    }
}

OrderType InputHandler::TryParseOrderType(const std::string_view& str) const {
    if (str == "GoodUntilCancel") {
        return OrderType::GoodUntilCancel;
    } else if (str == "FillAndKill") {
        return OrderType::FillAndKill;
    } else if (str == "FillOrKill") {
        return OrderType::FillOrKill;
    } else if (str == "Market") {
        return OrderType::Market;
    } else {
        throw std::logic_error("Invalid order type");
    }
}

Price InputHandler::TryParsePrice(const std::string_view& str) const {
    if (str.empty())
        throw std::logic_error("Invalid price");

    return ToNumber(str);
}

Quantity InputHandler::TryParseQuantity(const std::string_view& str) const {
    if (str.empty())
        throw std::logic_error("Invalid quantity");

    return ToNumber(str);
}

OrderId InputHandler::TryParseOrderId(const std::string_view& str) const {
    if (str.empty())
        throw std::logic_error("Invalid orderid");

    return static_cast<OrderId>(ToNumber(str));
}

bool InputHandler::TryParseInfo(const std::string_view& str, Info& info) const {
    auto value = str.at(0);
    auto values = Split(str, ' ');
    
    if (value == 'A') {
        info.action_ = ActionType::Add;
        info.side_ = TryParseSide(values[1]);
        info.orderType_ = TryParseOrderType(values[2]);
        info.price_ = TryParsePrice(values[3]);
        info.quantity_ = TryParseQuantity(values[4]);
        info.orderId_ = TryParseOrderId(values[5]);
    } else if (value == 'M') {
        info.action_ = ActionType::Modify;
        info.orderId_ = TryParseOrderId(values[1]);
        info.side_ = TryParseSide(values[2]);
        info.price_ = TryParsePrice(values[3]);
        info.quantity_ = TryParseQuantity(values[4]);
    } else if (value == 'C') {
        info.action_ = ActionType::Cancel;
        info.orderId_ = TryParseOrderId(values[1]);
    } else {
        return false;
    }
    return true;
}

void InputHandler::ReadFromFile(const std::filesystem::path& path, Orderbook& orderbook) const {
    Infos infos;
    std::string line;
    std::ifstream file {path};

    if (!file.is_open())
        throw std::runtime_error("File not found");

    while (std::getline(file, line)) {
        if (line.empty())
            break;

        Info info;

        if (TryParseInfo(line, info)) {
            infos.push_back(info);
        } else {
            std::cerr << line << std::endl;
            throw std::logic_error("Invalid input");
        }
    }
    file.close();
    ProcessInfo(infos, orderbook);
}

void InputHandler::ReadFromInput(Orderbook& orderbook) const {
    std::string line_;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, line_);
    std::string_view line {line_};

    Info info;
    Infos infos;

    if (TryParseInfo(line, info)) {
        infos.push_back(info);
        ProcessInfo(infos, orderbook);
    } else {
        std::cerr << "Invalid input, try again" << std::endl;
    }
}

void InputHandler::ProcessInfo(const Infos& infos, Orderbook& orderbook) const {
    for (const auto& info : infos) {
        switch (info.action_) {
            case ActionType::Add:
                orderbook.AddOrder(std::make_shared<Order>(info.orderType_, info.orderId_, info.side_, info.price_, info.quantity_));
                break;
            case ActionType::Modify:
                orderbook.ModifyOrder(OrderModify(info.orderId_, info.side_, info.price_, info.quantity_));
                break;
            case ActionType::Cancel:
                orderbook.CancelOrder(info.orderId_);
                break;
        }
    }
}
