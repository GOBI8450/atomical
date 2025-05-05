#pragma once
// Include necessary libraries
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <string>
#include <deque>
#include <mutex>
#include <SFML/Graphics.hpp>; // SFML graphics library
#include <SFML/Window.hpp>    // SFML window library
#include "BaseShape.h"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "Options.h"
#include "Serialization.h"
#include "UI.h"

// Use Boost Asio for networking
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

// Class to handle client-side networking logic
class HandleNetworkingClient {
public:
	// Constructor initializes sockets and endpoints
	HandleNetworkingClient(boost::asio::io_context& io_context,
		const std::string& host,
		unsigned short tcp_port,
		unsigned short udp_port)
		: io_context_(io_context),
		tcp_socket_(io_context),
		udp_socket_(io_context, udp::endpoint(udp::v4(), 0)), // binds UDP socket to any available port
		resolver_(io_context),
		should_try_connect_(true),
		retry_timer_(io_context) {

		auto tcp_results = resolver_.resolve(host, std::to_string(tcp_port));
		tcp_endpoint_ = *tcp_results.begin(); // use the first resolved TCP endpoint
		udp_endpoint_ = udp::endpoint(tcp_endpoint_.address(), udp_port); // set UDP endpoint with resolved IP and given port
	}

	// Stop reconnect attempts
	void stop_connecting() {
		should_try_connect_ = false;
		retry_timer_.cancel(); // cancel any pending retry timers
	}

	// Begin connection process
	void connect() {
		attempt_connect();
	}

	// Send a message over TCP
	void send_tcp_message(const std::string& message) {
		bool write_in_progress = !tcp_message_queue_.empty(); // check if a write is already in progress
		tcp_message_queue_.push_back(message + "\n"); // add message to the queue

		if (!write_in_progress) {
			do_tcp_write(); // start writing if no other write is ongoing
		}
	}

	// Send a message over UDP
	void send_udp_message(const std::string& message) {
		udp_socket_.async_send_to(
			boost::asio::buffer(message),
			udp_endpoint_,
			[this, message](const boost::system::error_code& ec, std::size_t /*bytes_sent*/) {
				if (!ec) {
					// UDP message sent successfully
				}
				else {
					std::cout << "UDP send failed: " << ec.message() << std::endl;
				}
			});
	}

	// Disconnect from server, cleanly closing sockets
	void disconnect_from_server() {
		should_try_connect_ = false;
		retry_timer_.cancel();

		if (tcp_socket_.is_open()) {
			boost::system::error_code ec;
			tcp_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec); // shutdown both send and receive
			if (ec) {
				std::cout << "TCP socket shutdown error: " << ec.message() << std::endl;
			}
			tcp_socket_.close(ec); // close TCP socket
			if (ec) {
				std::cout << "TCP socket close error: " << ec.message() << std::endl;
			}
			else {
				std::cout << "TCP socket disconnected." << std::endl;
			}
		}

		if (udp_socket_.is_open()) {
			boost::system::error_code ec;
			udp_socket_.close(ec); // close UDP socket
			if (ec) {
				std::cout << "UDP socket close error: " << ec.message() << std::endl;
			}
			else {
				std::cout << "UDP socket disconnected." << std::endl;
			}
		}
	}

protected:
	// Method to handle received messages, to be overridden by subclass
	virtual void TranslateMessage(const std::string& message) {}

	// Thread-safe storage of received messages
	void SaveMessage(const std::string& message) {
		std::lock_guard<std::mutex> lock(storedMessagesMutex);
		storedMessages.push_back(message);
	}

	// Retrieve stored messages (thread-safe)
	const std::vector<std::string>& GetStoredMessages() {
		std::lock_guard<std::mutex> lock(storedMessagesMutex);
		return storedMessages;
	}

	// Clear stored messages
	void ClearStoredMessages() {
		std::lock_guard<std::mutex> lock(storedMessagesMutex);
		storedMessages.clear();
	}

private:
	// Try to connect to server
	void attempt_connect() {
		if (!should_try_connect_) {
			std::cout << "Stopped trying to connect." << std::endl;
			return;
		}

		if (tcp_socket_.is_open()) {
			tcp_socket_.close(); // close socket if already open
		}

		std::cout << "\033[31m" << "Attempting to connect to server..." << std::endl;
		tcp_socket_.async_connect(
			tcp_endpoint_,
			[this](const boost::system::error_code& ec) {
				if (!ec) {
					std::cout << "\033[0m" << "Connected to TCP server!" << std::endl;
					start_tcp_receive(); // begin receiving TCP messages
					start_udp_receive(); // begin receiving UDP messages
				}
				else {
					std::cout << "TCP connection failed: " << ec.message() << std::endl;
					schedule_reconnect(); // retry connection
				}
			});
	}

	// Schedule a reconnect attempt after 5 seconds
	void schedule_reconnect() {
		if (!should_try_connect_) return;

		std::cout << "Retrying in 5 seconds..." << std::endl;
		retry_timer_.expires_after(std::chrono::seconds(5));
		retry_timer_.async_wait([this](const boost::system::error_code& ec) {
			if (!ec && should_try_connect_) {
				attempt_connect(); // attempt reconnect
			}
			});
	}

	// Perform an async TCP write from message queue
	void do_tcp_write() {
		boost::asio::async_write(
			tcp_socket_,
			boost::asio::buffer(tcp_message_queue_.front()),
			[this](const boost::system::error_code& ec, std::size_t /*length*/) {
				if (!ec) {
					tcp_message_queue_.pop_front(); // remove sent message
					if (!tcp_message_queue_.empty()) {
						do_tcp_write(); // continue writing if more messages remain
					}
				}
				else {
					std::cout << "TCP write failed: " << ec.message() << std::endl;
				}
			});
	}

	// Begin receiving TCP messages
	void start_tcp_receive() {
		boost::asio::async_read_until(
			tcp_socket_,
			tcp_buffer_,
			'\n',
			[this](const boost::system::error_code& ec, std::size_t length) {
				if (!ec) {
					std::string message(boost::asio::buffers_begin(tcp_buffer_.data()),
						boost::asio::buffers_begin(tcp_buffer_.data()) + length);
					tcp_buffer_.consume(length); // remove consumed data

					TranslateMessage(message); // process the message
					start_tcp_receive(); // continue reading
				}
				else {
					std::cout << "TCP receive failed: " << ec.message() << ", Bytes -" << length << std::endl;
				}
			});

		// Send local UDP port to the server
		send_tcp_message("udp:" + std::to_string(udp_socket_.local_endpoint().port()));
	}

	// Begin receiving UDP messages
	void start_udp_receive() {
		udp_socket_.async_receive_from(
			boost::asio::buffer(udp_data_, max_length),
			udp_sender_endpoint_,
			[this](const boost::system::error_code& errorCode, std::size_t bytesRecived) {
				if (!errorCode) {
					std::string message(udp_data_, bytesRecived);
					TranslateMessage(message); // process message
					start_udp_receive(); // continue receiving
				}
				else {
					std::cout << "UDP receive failed: " << errorCode.message() << ", Bytes -" << bytesRecived << std::endl;
				}
			});
	}

	// Member variables for networking
	boost::asio::io_context& io_context_;
	tcp::socket tcp_socket_;                      // TCP socket
	udp::socket udp_socket_;                      // UDP socket
	tcp::resolver resolver_;                      // Resolver to get endpoints
	tcp::endpoint tcp_endpoint_;                  // TCP server endpoint
	udp::endpoint udp_endpoint_;                  // UDP server endpoint
	udp::endpoint udp_sender_endpoint_;           // Endpoint of sender (for receiving UDP)
	boost::asio::streambuf tcp_buffer_;           // Buffer for incoming TCP data
	enum { max_length = 8192 };                    // Max UDP packet length
	char udp_data_[max_length];                   // UDP data buffer
	std::deque<std::string> tcp_message_queue_;   // Queue for outgoing TCP messages
	std::vector<std::string> storedMessages;      // Stored incoming messages
	mutable std::mutex storedMessagesMutex;       // Mutex for thread-safe message storage

	bool should_try_connect_;                     // Flag to control connection retry logic
	boost::asio::steady_timer retry_timer_;       // Timer for connection retry
};
