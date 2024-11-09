#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <chrono>



using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class GameClient {
public:
    GameClient(const std::string& host, int tcp_port, int udp_port)
        : io_context_(),
        work_guard_(boost::asio::make_work_guard(io_context_)),
        tcp_socket_(io_context_),
        udp_socket_(io_context_),
        server_host_(host),
        tcp_port_(tcp_port),
        udp_port_(udp_port),
        resolver_(io_context_),
        reconnect_timer_(io_context_),
        connected_(false) {

        messages_to_send_.clear();
    }

    void start() {
        // Start IO context in separate thread
        io_thread_ = std::thread([this]() {
            try {
                io_context_.run();
            }
            catch (std::exception& e) {
                std::cerr << "IO context error: " << e.what() << std::endl;
            }
            });

        connectToServer();
    }

    void stop() {
        connected_ = false;

        if (tcp_socket_.is_open()) {
            boost::system::error_code ec;
            tcp_socket_.shutdown(tcp::socket::shutdown_both, ec);
            tcp_socket_.close();
        }

        if (udp_socket_.is_open()) {
            udp_socket_.close();
        }

        io_context_.stop();

        if (io_thread_.joinable()) {
            io_thread_.join();
        }
    }

    // Send game state update via TCP
    void sendGameState(const std::string& state) {
        boost::json::object msg = {
            {"type", "game_state"},
            {"state", state}
        };
        sendTcpMessage(msg);
    }

    // Send position update via UDP
    void sendPosition(float x, float y, float z) {
        boost::json::object msg = {
            {"type", "position"},
            {"position", {
                {"x", x},
                {"y", y},
                {"z", z}
            }}
        };
        sendUdpMessage(msg);
    }

private:
    void connectToServer() {
        std::cout << "Attempting to connect to server..." << std::endl;

        resolver_.async_resolve(
            server_host_,
            std::to_string(tcp_port_),
            [this](const boost::system::error_code& ec, tcp::resolver::results_type endpoints) {
                if (!ec) {
                    attemptConnection(endpoints);
                }
                else {
                    handleConnectionFailure("Resolution error: " + ec.message());
                }
            }
        );
    }

    void attemptConnection(const tcp::resolver::results_type& endpoints) {
        boost::asio::async_connect(
            tcp_socket_,
            endpoints,
            [this](const boost::system::error_code& ec, const tcp::endpoint&) {
                if (!ec) {
                    handleSuccessfulConnection();
                }
                else {
                    handleConnectionFailure("Connection error: " + ec.message());
                }
            }
        );
    }

    void handleSuccessfulConnection() {
        std::cout << "TCP connection established!" << std::endl;
        connected_ = true;

        // Setup UDP socket
        setupUdpSocket();

        // Send initial connection message with UDP port
        boost::json::object connect_msg = {
            {"type", "connect"},
            {"udp_address", udp_socket_.local_endpoint().port()}
        };
        sendTcpMessage(connect_msg);

        // Start receiving messages
        startReceiving();

        // Send any queued messages
        sendQueuedMessages();
    }

    void handleConnectionFailure(const std::string& error) {
        std::cerr << error << std::endl;
        connected_ = false;

        // Schedule reconnection attempt
        reconnect_timer_.expires_after(std::chrono::seconds(2));
        reconnect_timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec) {
                std::cout << "Attempting to reconnect..." << std::endl;
                connectToServer();
            }
            });
    }

    void setupUdpSocket() {
        boost::system::error_code ec;
        udp_socket_.open(udp::v4(), ec);
        if (ec) {
            std::cerr << "UDP socket open error: " << ec.message() << std::endl;
            return;
        }

        // Prepare UDP endpoint for sending
        server_udp_endpoint_ = udp::endpoint(
            boost::asio::ip::address::from_string(server_host_),
            udp_port_
        );
    }

    void startReceiving() {
        // Start TCP receive
        asyncReadTcp();

        // Start UDP receive
        asyncReadUdp();
    }

    void asyncReadTcp() {
        tcp_socket_.async_read_some(
            boost::asio::buffer(tcp_buffer_),
            [this](const boost::system::error_code& ec, std::size_t length) {
                if (!ec) {
                    // Process received TCP data
                    std::string received(tcp_buffer_.begin(), tcp_buffer_.begin() + length);
                    processTcpMessage(received);
                    asyncReadTcp(); // Continue reading
                }
                else {
                    handleDisconnect();
                }
            }
        );
    }

    void asyncReadUdp() {
        udp_socket_.async_receive_from(
            boost::asio::buffer(udp_buffer_),
            udp_remote_endpoint_,
            [this](const boost::system::error_code& ec, std::size_t length) {
                if (!ec) {
                    // Process received UDP data
                    std::string received(udp_buffer_.begin(), udp_buffer_.begin() + length);
                    processUdpMessage(received);
                    asyncReadUdp(); // Continue reading
                }
                // For UDP, always continue reading regardless of errors
                asyncReadUdp();
            }
        );
    }

    void sendTcpMessage(const boost::json::object& message) {
        if (!connected_) {
            // Queue message if not connected
            messages_to_send_.push_back(boost::json::serialize(message));
            return;
        }

        std::string msg = boost::json::serialize(message) + "\n";
        boost::asio::async_write(
            tcp_socket_,
            boost::asio::buffer(msg),
            [this](const boost::system::error_code& ec, std::size_t /*length*/) {
                if (ec) {
                    handleDisconnect();
                }
            }
        );
    }

    void sendUdpMessage(const boost::json::object& message) {
        if (!connected_) return;

        try {
            std::string msg = boost::json::serialize(message);
            udp_socket_.async_send_to(
                boost::asio::buffer(msg),
                server_udp_endpoint_,
                [](const boost::system::error_code& /*ec*/, std::size_t /*bytes*/) {}
            );
        }
        catch (std::exception& e) {
            std::cerr << "UDP send error: " << e.what() << std::endl;
        }
    }

    void sendQueuedMessages() {
        for (const auto& msg : messages_to_send_) {
            boost::asio::async_write(
                tcp_socket_,
                boost::asio::buffer(msg + "\n"),
                [this](const boost::system::error_code& ec, std::size_t /*length*/) {
                    if (ec) {
                        handleDisconnect();
                    }
                }
            );
        }
        messages_to_send_.clear();
    }

    void handleDisconnect() {
        if (!connected_) return;

        connected_ = false;
        std::cout << "Disconnected from server. Attempting to reconnect..." << std::endl;

        boost::system::error_code ec;
        tcp_socket_.close(ec);
        connectToServer();
    }

    void processTcpMessage(const std::string& message) {
        try {
            boost::json::value parsed = boost::json::parse(message);
            // Add your message processing logic here
            std::cout << "TCP message received: " << message << std::endl;
        }
        catch (std::exception& e) {
            std::cerr << "Error processing TCP message: " << e.what() << std::endl;
        }
    }

    void processUdpMessage(const std::string& message) {
        try {
            boost::json::value parsed = boost::json::parse(message);
            // Add your message processing logic here
            std::cout << "UDP message received: " << message << std::endl;
        }
        catch (std::exception& e) {
            std::cerr << "Error processing UDP message: " << e.what() << std::endl;
        }
    }

    boost::asio::io_context io_context_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    tcp::socket tcp_socket_;
    udp::socket udp_socket_;
    tcp::resolver resolver_;
    boost::asio::steady_timer reconnect_timer_;

    std::string server_host_;
    int tcp_port_;
    int udp_port_;
    std::atomic<bool> connected_;

    udp::endpoint server_udp_endpoint_;
    udp::endpoint udp_remote_endpoint_;

    std::array<char, 1024> tcp_buffer_;
    std::array<char, 1024> udp_buffer_;

    std::vector<std::string> messages_to_send_;
    std::thread io_thread_;
};

// Example usage
int main() {
    try {
        GameClient client("127.0.0.1", 5000, 5001);
        client.start();

        std::cout << "Client started. Commands:\n"
            << "1. 'pos x y z' to send position\n"
            << "2. 'state <message>' to send game state\n"
            << "3. 'quit' to exit\n";

        std::string input;
        while (std::getline(std::cin, input)) {
            if (input == "quit") {
                break;
            }

            // Parse commands
            std::istringstream iss(input);
            std::string command;
            iss >> command;

            if (command == "pos") {
                float x, y, z;
                if (iss >> x >> y >> z) {
                    client.sendPosition(x, y, z);
                }
            }
            else if (command == "state") {
                std::string state;
                std::getline(iss >> std::ws, state);
                client.sendGameState(state);
            }
        }

        client.stop();
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}