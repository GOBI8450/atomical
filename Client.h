#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <string>
#include <deque>
#include <mutex>
#include <SFML/Graphics.hpp>;
#include <SFML/Window.hpp>
#include "BaseShape.h"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "PhysicsSimulatotion.h"
#include "Options.h"
#include "UI.h"


using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class Client {
public:
    Client(boost::asio::io_context& io_context,
        const std::string& host,
        unsigned short tcp_port,
        unsigned short udp_port)
        : io_context_(io_context),
        tcp_socket_(io_context),
        udp_socket_(io_context, udp::endpoint(udp::v4(), 0)),
        resolver_(io_context),
        should_try_connect_(true),
        retry_timer_(io_context) {

        auto tcp_results = resolver_.resolve(host, std::to_string(tcp_port));
        tcp_endpoint_ = *tcp_results.begin();
        udp_endpoint_ = udp::endpoint(tcp_endpoint_.address(), udp_port);
    }

    void stop_connecting() {
        should_try_connect_ = false;
        retry_timer_.cancel();
    }

    void connect() {
        attempt_connect();
    }

    void send_tcp_message(const std::string& message) {
        bool write_in_progress = !tcp_message_queue_.empty();
        tcp_message_queue_.push_back(message + "\n");

        if (!write_in_progress) {
            do_tcp_write();
        }
    }

    void send_udp_message(const std::string& message) {
        udp_socket_.async_send_to(
            boost::asio::buffer(message),
            udp_endpoint_,
            [this, message](const boost::system::error_code& ec, std::size_t /*bytes_sent*/) {
                if (!ec) {
                    std::cout << "UDP message sent: " << message << std::endl;
                }
                else {
                    std::cout << "UDP send failed: " << ec.message() << std::endl;
                }
            });
    }

private:
    void attempt_connect() {
        if (!should_try_connect_) {
            std::cout << "Stopped trying to connect." << std::endl;
            return;
        }

        // Close socket if it's open before attempting new connection
        if (tcp_socket_.is_open()) {
            tcp_socket_.close();
        }

        std::cout << "Attempting to connect to server..." << std::endl;
        tcp_socket_.async_connect(
            tcp_endpoint_,
            [this](const boost::system::error_code& ec) {
                if (!ec) {
                    std::cout << "Connected to TCP server!" << std::endl;
                    start_tcp_receive();
                    start_udp_receive();
                }
                else {
                    std::cout << "TCP connection failed: " << ec.message() << std::endl;
                    schedule_reconnect();
                }
            });
    }

    void schedule_reconnect() {
        if (!should_try_connect_) return;

        std::cout << "Retrying in 5 seconds..." << std::endl;
        retry_timer_.expires_after(std::chrono::seconds(5));
        retry_timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec && should_try_connect_) {
                attempt_connect();
            }
            });
    }

    // Add these as class members:
    bool should_try_connect_;
    boost::asio::steady_timer retry_timer_;

    void start_tcp_receive() {
        boost::asio::async_read_until(
            tcp_socket_,
            tcp_buffer_,
            '\n',
            [this](const boost::system::error_code& ec, std::size_t length) {
                if (!ec) {
                    std::string message(boost::asio::buffers_begin(tcp_buffer_.data()),
                        boost::asio::buffers_begin(tcp_buffer_.data()) + length);
                    tcp_buffer_.consume(length);

                    std::cout << "TCP received: " << message;

                    start_tcp_receive();
                }
                else {
                    std::cout << "TCP receive failed: " << ec.message() << std::endl;
                }
            });
    }

    void do_tcp_write() {
        boost::asio::async_write(
            tcp_socket_,
            boost::asio::buffer(tcp_message_queue_.front()),
            [this](const boost::system::error_code& ec, std::size_t /*length*/) {
                if (!ec) {
                    tcp_message_queue_.pop_front();
                    if (!tcp_message_queue_.empty()) {
                        do_tcp_write();
                    }
                }
                else {
                    std::cout << "TCP write failed: " << ec.message() << std::endl;
                }
            });
    }

    void start_udp_receive() {
        udp_socket_.async_receive_from(
            boost::asio::buffer(udp_data_, max_length),
            udp_sender_endpoint_,
            [this](const boost::system::error_code& ec, std::size_t bytes_received) {
                if (!ec) {
                    std::string message(udp_data_, bytes_received);
                    std::cout << "UDP received: " << message << std::endl;
                    start_udp_receive();
                }
                else {
                    std::cout << "UDP receive failed: " << ec.message() << std::endl;
                }
            });
    }


    boost::asio::io_context& io_context_;
    tcp::socket tcp_socket_;
    udp::socket udp_socket_;
    tcp::resolver resolver_;
    tcp::endpoint tcp_endpoint_;
    udp::endpoint udp_endpoint_;
    udp::endpoint udp_sender_endpoint_;
    boost::asio::streambuf tcp_buffer_;
    enum { max_length = 1024 };
    char udp_data_[max_length];
    std::deque<std::string> tcp_message_queue_;
};

void initializeWindow(sf::RenderWindow& window, sf::View view, sf::ContextSettings settings) {
    window.create(
        sf::VideoMode(options.window_width, options.window_height), "TomySim", sf::Style::Default, settings);
    view = window.getDefaultView();
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);
}

void toggleFullscreen(sf::RenderWindow& window) {
    if (!options.fullscreen) {
        window.create(options.desktopSize, "Fullscreen Mode", sf::Style::Fullscreen);
        options.fullscreen = true;
    }
    else {
        window.create(sf::VideoMode(1920, 980), "Tomy Mode", sf::Style::Default);
        options.fullscreen = false;
    }
    window.setVerticalSyncEnabled(true);
}

void Run(std::string& screen, PhysicsSimulationLocal& simulation, MainMenu& mainMenu, Settings& settingsClass, sf::RenderWindow& window) {
    while (window.isOpen()) {
        if (screen == "START") {
            screen = simulation.Run();
        }
        else if (screen == "MAIN MENU") {
            screen = mainMenu.handleMainMenu();
        }
        else if (screen == "SETTINGS") {
            screen = settingsClass.handleSettings();
        }
        else if (screen == "FULLSCREEN") {
            toggleFullscreen(window);
            screen = settingsClass.handleSettings();
        }
        else {
            window.close();
            break;  //  break to exit the loop when closing
        }
    }
}


int main() {
    try {
        const std::string server_ip = "127.0.0.1";  // or "localhost"
        unsigned short tcp_port = 8080;
        unsigned short udp_port = 8081;

        std::cout << "Starting client..." << std::endl;
        std::cout << "Attempting to connect to:" << std::endl;
        std::cout << "Server IP: " << server_ip << std::endl;
        std::cout << "TCP port: " << tcp_port << std::endl;
        std::cout << "UDP port: " << udp_port << std::endl;

        boost::asio::io_context io_context;

        Client client(io_context, server_ip, tcp_port, udp_port);
        client.connect();

        // Start a thread to run the IO service
        std::thread io_thread([&io_context]() {
            io_context.run();
            });

        std::cout << "\nAvailable commands:" << std::endl;
        std::cout << "- Type a message to send via TCP" << std::endl;
        std::cout << "- Type 'udp:' followed by a message to send via UDP" << std::endl;
        std::cout << "- Type 'stop' to stop connection attempts" << std::endl;
        std::cout << "- Type 'connect' to start connection attempts" << std::endl;
        std::cout << "- Type 'quit' to exit" << std::endl;

        //ENGINE:
        sf::RenderWindow window;
        sf::ContextSettings settings;
        settings.antialiasingLevel = 8;
        sf::View view = window.getDefaultView();
        initializeWindow(window, view, settings);

        PhysicsSimulationLocal simulation(window);
        MainMenu mainMenu(window);
        Settings settingsClass(window);
        std::string screen = "START";
        Run(screen, simulation, mainMenu, settingsClass, window);

        //ENGINE end

        // Main loop to get user input and send messages
        std::string input;
        while (std::getline(std::cin, input)) {
        
            if (input == "quit") {
                std::cout << "Shutting down client..." << std::endl;
                client.stop_connecting();
                break;
            }
            if (input == "stop") {
                client.stop_connecting();
                std::cout << "Stopped connection attempts." << std::endl;
                continue;
            }
            if (input == "connect") {
                std::cout << "Starting connection attempts..." << std::endl;
                client.connect();
                continue;
            }

            if (input.substr(0, 4) == "udp:") {
                client.send_udp_message(input.substr(4));
            }
            else {
                client.send_tcp_message(input);
            }
        }

        io_context.stop();
        io_thread.join();
    }
    catch (std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}