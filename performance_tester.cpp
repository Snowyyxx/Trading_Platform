#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <boost/asio.hpp>

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

using boost::asio::local::stream_protocol;

const std::string SOCKET_FILE = "/tmp/trading_socket";

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <number_of_orders_to_send>" << std::endl;
        return 1;
    }

    try {
        const int num_orders = std::stoi(argv[1]);
        if (num_orders <= 0) {
            std::cerr << "Number of orders must be a positive integer." << std::endl;
            return 1;
        }

        boost::asio::io_context io_context;
        stream_protocol::socket s(io_context);
        s.connect(stream_protocol::endpoint(SOCKET_FILE));
        
        std::cout << "Connected to engine. Preparing to send " << num_orders << " orders..." << std::endl;

        std::vector<std::string> symbols = {"AAPL", "GOOG", "MSFT", "TSLA", "NVDA"};
        std::vector<std::string> types = {"BUY", "SELL"};

        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < num_orders; ++i) {
            // Generate a plausible, but random, order
            std::string symbol = symbols[i % symbols.size()];
            std::string type = types[i % types.size()];
            double price = 100.0 + (rand() % 1000) / 100.0;
            int quantity = 1 + (rand() % 100);

            std::string command = "ADD;" + symbol + ";" + type + ";" + std::to_string(price) + ";" + std::to_string(quantity) + "\n";
            
            // Send the order. We don't wait for a response to measure raw ingestion speed.
            boost::asio::write(s, boost::asio::buffer(command));
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;

        std::cout << "\n--- Performance Test Complete ---" << std::endl;
        std::cout << "Sent " << num_orders << " orders in " << elapsed.count() << " seconds." << std::endl;
        double throughput = num_orders / elapsed.count();
        std::cout << "Throughput: " << std::fixed << throughput << " orders/second." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Tester error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#else
#error UNIX domain sockets not supported on this platform.
#endif