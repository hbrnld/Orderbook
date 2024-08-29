#include "OrderBook.h"
#include "InputHandler.h"
#include "Interface.h"

int main() {
    Orderbook orderbook;
    InputHandler handler;
    Interface interface;

    handler.ReadFromFile("Data/Fill_Orderbook.txt", orderbook);
    interface.Run(orderbook, handler);

    return 0;
}