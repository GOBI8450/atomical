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

class HandleNetworkingClient {
public:
    HandleNetworkingClient(boost::asio::io_context& io_context,
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

    virtual std::vector<BaseShape> TranslateMessage() { return std::vector<BaseShape>();}

    void start_udp_receive() {
        udp_socket_.async_receive_from(
            boost::asio::buffer(udp_data_, max_length),
            udp_sender_endpoint_,
            [this](const boost::system::error_code& errorCode, std::size_t bytesRecived) {
                if (!errorCode) {
                    std::string message(udp_data_, bytesRecived);
                    std::cout << "UDP received: " << message << std::endl;
                    TranslateMessage();
                    start_udp_receive();
                }
                else {
                    std::cout << "UDP receive failed: " << errorCode.message() << std::endl;
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