#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <limits>
#include <algorithm>

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

using boost::asio::local::stream_protocol;

const std::string SOCKET_FILE = "/tmp/trading_socket";

void placeOrder(stream_protocol::socket& socket) {
    std::string symbol, typeStr;
    double price;
    int quantity;

    std::cout << "Enter stock symbol (e.g., AAPL, GOOG): ";
    std::cin >> symbol;
    
    std::cout << "Enter order type (BUY or SELL): ";
    while (std::cin >> typeStr && !(typeStr == "BUY" || typeStr == "buy" || typeStr == "SELL" || typeStr == "sell")) {
        std::cout << "Invalid type. Please enter BUY or SELL: ";
    }
    if (typeStr == "buy") typeStr = "BUY";
    if (typeStr == "sell") typeStr = "SELL";

    std::cout << "Enter price: ";
    while (!(std::cin >> price) || price <= 0) {
        std::cout << "Invalid price. Please enter a positive number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "Enter quantity: ";
    while (!(std::cin >> quantity) || quantity <= 0) {
        std::cout << "Invalid quantity. Please enter a positive integer: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    
    // Command format is now: ADD;SYMBOL;TYPE;PRICE;QUANTITY
    std::string command = "ADD;" + symbol + ";" + typeStr + ";" + std::to_string(price) + ";" + std::to_string(quantity) + "\n";
    boost::asio::write(socket, boost::asio::buffer(command));
}

void viewOrderBook(stream_protocol::socket& socket) {
    std::string symbol;
    std::cout << "Enter stock symbol to view (e.g., AAPL, GOOG): ";
    std::cin >> symbol;

    // Command format is now: VIEW;SYMBOL
    std::string command = "VIEW;" + symbol + "\n";
    boost::asio::write(socket, boost::asio::buffer(command));
}

void listen_for_response(stream_protocol::socket& socket) {
    boost::asio::streambuf response_buf;
    boost::system::error_code ec;
    
    boost::asio::read_until(socket, response_buf, '\n', ec);

    if (ec && ec != boost::asio::error::eof) {
        throw boost::system::system_error(ec);
    }
    
    std::istream response_stream(&response_buf);
    std::string response_data;
    std::getline(response_stream, response_data, '\0');

    std::stringstream ss(response_data);
    std::string header;
    std::getline(ss, header, ';');
    
    std::cout << "\n<-- Response from Server:\n";
    if (header == "BOOK") {
        std::string book_content;
        std::getline(ss, book_content, '\0');
        std::cout << book_content;
    } else {
        std::cout << response_data;
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        stream_protocol::socket s(io_context);
        s.connect(stream_protocol::endpoint(SOCKET_FILE));
        std::cout << "Connected to trading engine." << std::endl;

        while (true) {
            std::cout << "\n--- Trading Client ---\n1. Place Order\n2. View Order Book\n3. Exit\n> ";
            int choice;
            if (!(std::cin >> choice)) {
                std::cout << "Invalid input.\n";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            if (choice == 1) {
                placeOrder(s);
                listen_for_response(s);
            } else if (choice == 2) {
                viewOrderBook(s);
                listen_for_response(s);
            } else if (choice == 3) {
                break;
            } else {
                std::cout << "Invalid choice.\n";
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
        std::cerr << "(Is the server running?)" << std::endl;
        return 1;
    }
    return 0;
}

#else
#error UNIX domain sockets not supported on this platform.
#endif