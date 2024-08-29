#include "Interface.h"

bool Interface::isNumeric(const std::string& str) const {
    for (char c : str) {
        if (!std::isdigit(c))
            return false;
    }
    return true;
}

int Interface::ToNumber(const std::string& str) const {
    return std::stoi(str);
}

Side Interface::SelectSide() const {
    char choice;
    do {
        std::cout << "Select side\n"
                    << "1. Buy\n"
                    << "2. Sell\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;
        if (choice == '1') {
            return Side::Buy;
        } else if (choice == '2') {
            return Side::Sell;
        } else {
            std::cout << "Invalid side\n";
        }
    } while (true);
}

OrderType Interface::SelectOrderType() const {
    char choice;
    do {
        std::cout << "Select order type\n"
                    << "1. GoodUntilCancel\n" 
                    << "2. FillAndKill\n"
                    << "3. FillOrKill\n"
                    << "4. Market\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;
        if (choice == '1') {
            return OrderType::GoodUntilCancel;
            break;
        } else if (choice == '2') {
            return OrderType::FillAndKill;
            break;
        } else if (choice == '3') {
            return OrderType::FillOrKill;
            break;
        } else if (choice == '4') {
            return OrderType::Market;
            break;
        } else {
            std::cout << "Invalid order type\n";
        }
    } while (true);
}

Price Interface::SelectPrice() const {
    std::string number;
    do {
        std::cout << "Enter price: ";
        std::cin >> number;

        if (isNumeric(number)) {
            return static_cast<Price>(ToNumber(number));
            break;
        } else {
            std::cout << "Invalid price\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    } while (true);
}

Quantity Interface::SelectQuantity() const {
    std::string number;
    do {
        std::cout << "Enter quantity: ";
        std::cin >> number;

        if (isNumeric(number)) {
            return static_cast<Quantity>(ToNumber(number));
            break;
        } else {
            std::cout << "Invalid quantity\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    } while (true);
}

OrderId Interface::SelectOrderId() const {
    std::string number;
    do {
        std::cout << "Enter OrderId: ";
        std::cin >> number;

        if (isNumeric(number)) {
            return static_cast<OrderId>(ToNumber(number));
            break;
        } else {
            std::cout << "Invalid OrderId\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    } while (true);
}

void Interface::PrintMenu() const {
    std::cout << "---------------------\n";
    std::cout << "P. Print Order Book\n";
    std::cout << "A. Add Order\n";
    std::cout << "M. Modify Order\n";
    std::cout << "C. Cancel Order\n";
    std::cout << "Q. Quit\n";
    std::cout << "---------------------\n";
}

void Interface::PressAddOrder(Orderbook& orderbook, InputHandler& handler) const {
    Info info;
    info.action_ = ActionType::Add;
    info.side_ = SelectSide();
    info.orderType_ = SelectOrderType();
    info.price_ = SelectPrice();
    info.quantity_ = SelectQuantity();
    info.orderId_ = rand() % 10000;

    handler.ProcessInfo({info}, orderbook);
}

void Interface::PressModifyOrder(Orderbook& orderbook, InputHandler& handler) const {
    std::cout << "Modifying Order...\n";
    Info info; 
    info.action_ = ActionType::Modify;
    info.orderId_ = SelectOrderId();
    info.side_ = SelectSide();
    info.price_ = SelectPrice();
    info.quantity_ = SelectQuantity();

    handler.ProcessInfo({info}, orderbook);
}

void Interface::PressCancelOrder(Orderbook& orderbook, InputHandler& handler) const {
    Info info; 
    info.action_ = ActionType::Cancel;
    info.orderId_ = SelectOrderId();

    handler.ProcessInfo({info}, orderbook);
}

void Interface::Run(Orderbook& orderbook, InputHandler& handler) const {
    char choice;
    PrintMenu();
    do {
        std::cout << "Enter choice: ";
        std::cin >> choice;
        choice = std::toupper(choice);
        
        switch (choice) {
            case 'P': {
                orderbook.PrintOrderbook();
                break;
            }
            case 'A': {
                PressAddOrder(orderbook, handler);
                orderbook.PrintOrderbook();
                break;
            }
            case 'M': {
                PressModifyOrder(orderbook, handler);
                orderbook.PrintOrderbook();
                break;
            }
            case 'C': {
                PressCancelOrder(orderbook, handler);
                break;
            }
            case 'Q': {
                break;
            }
            default: {
                std::cout << "Invalid choice\n";
                break;
            }
        }
    } while (choice != 'Q');
}