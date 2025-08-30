#include <iostream>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "database.hpp"
#include "trading_engine.hpp"

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

using boost::asio::local::stream_protocol;

const std::string SOCKET_FILE = "/tmp/trading_socket";

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(stream_protocol::socket socket, TradingEngine& engine)
        : socket_(std::move(socket)), engine_(engine) {}

    void start() { do_read(); }

private:
    void do_read() {
        auto self(shared_from_this());
        boost::asio::async_read_until(socket_, buffer_, "\n",
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string command(boost::asio::buffer_cast<const char*>(buffer_.data()), length - 1);
                    buffer_.consume(length);

                    std::string response = engine_.processCommand(command);
                    do_write(response);
                }
            });
    }

    void do_write(const std::string& response) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(response),
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    do_read(); // Wait for the next command
                }
            });
    }

    stream_protocol::socket socket_;
    TradingEngine& engine_;
    boost::asio::streambuf buffer_;
};

class Server {
public:
    Server(boost::asio::io_context& io_context, const std::string& file, TradingEngine& engine)
        : acceptor_(io_context, stream_protocol::endpoint(file)), engine_(engine) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, stream_protocol::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket), engine_)->start();
                }
                do_accept();
            });
    }

    stream_protocol::acceptor acceptor_;
    TradingEngine& engine_;
};

std::atomic<int> orderIdCounter;

int main() {
    unlink(SOCKET_FILE.c_str());

    try {
        Database db("orders.db");
        orderIdCounter = db.getMaxOrderId() + 1;
        TradingEngine engine(db, orderIdCounter);
        
        std::cout << "[Server] Engine started. Order IDs begin at " << orderIdCounter.load() << "." << std::endl;
        
        boost::asio::io_context io_context;
        Server s(io_context, SOCKET_FILE, engine);
        
        std::cout << "[Server] Listening on " << SOCKET_FILE << ". Press Ctrl+C to stop." << std::endl;
        
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

#else
#error UNIX domain sockets not supported on this platform.
#endif