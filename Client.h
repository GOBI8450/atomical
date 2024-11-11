//#pragma once
//#include <boost/asio.hpp>
//#include <iostream>
//#include <thread>
//#include <string>
//#include <deque>
//#include <mutex>
//#include <SFML/Graphics.hpp>;
//#include <SFML/Window.hpp>
//#include "BaseShape.h"
//#include <boost/archive/binary_iarchive.hpp>
//#include <boost/archive/binary_oarchive.hpp>
//#include "PhysicsSimulatotion.h"
//#include "Options.h"
//#include "UI.h"
//#include "HandleNetworkingClient.h"
//
//
//using boost::asio::ip::tcp;
//using boost::asio::ip::udp;
//
//class Client : public HandleNetworkingClient, public PhysicsSimulationLocal {
//private :
//    std::string Vector2iToString(const sf::Vector2i& vector) {
//        std::ostringstream oss;  // create a string stream
//        oss << "(" << vector.x << ", " << vector.y << ")";  // format the vector as a string
//        return oss.str();  // return the formatted string
//    }
//    std::string Vector2fToString(const sf::Vector2f& vector) {
//        std::ostringstream oss;  // create a string stream
//        oss << "(" << vector.x << ", " << vector.y << ")";  // format the vector as a string
//        return oss.str();  // return the formatted string
//    }
//public:
//    Client(sf::RenderWindow& win, boost::asio::io_context& io_context,
//        const std::string& host,
//        unsigned short tcp_port,
//        unsigned short udp_port) : 
//        PhysicsSimulationLocal(win), HandleNetworkingClient(io_context, host, tcp_port, udp_port) {}
//
//    //Local:
//    void handleMouseClick() override {
//        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && mouseClickFlag == false) {
//            thisBallPointer = objectList.IsInRadius(currentMousePos); // Check if a circle is within radius
//            if (previousBallPointer == nullptr)
//            {
//                previousBallPointer = thisBallPointer;
//            }
//            if (thisBallPointer != nullptr) { // Check if a circle was found
//                mouseClickFlag = true; // Set flag if circle found
//                window.setMouseCursor(handCursor);
//            }
//        }
//    }
//
//    void handleMouseRelase(sf::Event event) override {
//        if (event.type == sf::Event::MouseButtonReleased) {
//            mouseClickFlag = false;
//            scaleFlag = false;
//            if (thisBallPointer != nullptr)
//            {
//                window.setMouseCursor(defaultCursor);
//                thisBallPointer->setColor(previousColor);
//                thisBallPointer->SetOutline(outlineColor, 0);
//            }
//            TouchedOnce = false;
//        }
//    }
//
//    void handleMouseWheel(sf::Event event) override {
//        if (event.type == sf::Event::MouseWheelScrolled) {
//            if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
//                //sf::Vector2f beforeZoom = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y), view);
//                if (event.mouseWheelScroll.delta > 0) {
//
//                    if (!scaleFlag) {
//                        view.zoom(1.f / ZOOM_FACTOR);
//                    }
//                    mouseFlagScrollUp = true;
//                }
//
//                else if (event.mouseWheelScroll.delta < 0) {
//                    if (!scaleFlag) {
//                        view.zoom(ZOOM_FACTOR);
//                    }
//                    mouseFlagScrollDown = true;
//                }
//
//                //sf::Vector2f afterZoom = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y), view);
//                //sf::Vector2f offset = beforeZoom - afterZoom;
//                //view.move(offset);
//            }
//        }
//    }
//
//    void toggleConnectingMode() override {
//        connectingMode = !connectingMode;
//        linkingText.setString(connectingMode ? "ACTIVATED" : "DEACTIVATED");
//        linkingText.setFillColor(connectingMode ? sf::Color::Magenta : sf::Color::White);
//    }
//
//
//    //Sends to server:
//
//    void handleKeyPress(sf::Event event) override {
//        if (event.type == sf::Event::KeyPressed) {
//            //Sends to server:
//            if (event.key.code == sf::Keyboard::Escape) {
//                screen = "MAIN MENU";
//                send_tcp_message("EXIT");
//            }
//            if (event.key.code == sf::Keyboard::A) {
//                send_udp_message("10->CIR");
//            }
//            if (event.key.code == sf::Keyboard::T) {
//                send_udp_message("10->REC");
//            }
//            if (event.key.code == sf::Keyboard::BackSpace && mouseClickFlag) {
//                send_tcp_message("DEL->" + thisBallPointer->GetIDStr());
//            }
//            if (event.key.code == sf::Keyboard::H) {
//                send_udp_message("LINK_RND");
//            }
//            if (event.key.code == sf::Keyboard::L) {
//                send_udp_message("PLANET");
//            }
//            if (event.key.code == sf::Keyboard::F) {
//                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
//                send_udp_message("CIREXPLOSION->" + Vector2iToString(mousePos));
//            }
//            if (event.key.code == sf::Keyboard::J) {
//                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
//                send_udp_message("CIREXPLOSION->" + Vector2iToString(mousePos));
//            }
//            if (event.key.code == sf::Keyboard::R) {
//                send_udp_message("RESTART");
//
//            }
//
//            //Local Keys:
//            if (event.key.code == sf::Keyboard::C) {
//                send_tcp_message("LINK");
//                if (!connectingMode)
//                {
//                    connectingMode = true;
//                    previousBallPointer = thisBallPointer;
//                    linkingText.setString("ACTIVATED");
//                    linkingText.setFillColor(sf::Color::Magenta);
//                }
//                else {
//                    connectingMode = false;
//                    linkingText.setString("DEACTIVATED");
//                    linkingText.setFillColor(sf::Color::White);
//                }
//            }
//            if (event.key.code == sf::Keyboard::S && mouseClickFlag)
//            {
//                scaleFlag = true;
//            }
//            if (event.key.code == sf::Keyboard::Left) {
//                view.move(-moveSpeedScreen, 0.f);
//            }
//            if (event.key.code == sf::Keyboard::Right) {
//                view.move(moveSpeedScreen, 0.f);
//            }
//            if (event.key.code == sf::Keyboard::Up) {
//                view.move(0.f, -moveSpeedScreen);
//            }
//            if (event.key.code == sf::Keyboard::Down) {
//                view.move(0.f, moveSpeedScreen);
//            }
//            if (event.key.code == sf::Keyboard::F11)
//            {
//                if (!options.fullscreen)
//                {
//                    window.create(options.desktopSize, "Fullscreen Mode", sf::Style::Fullscreen);
//                    options.fullscreen = true;
//                }
//                else {
//                    window.create(sf::VideoMode(1920, 980), "Tomy Mode", sf::Style::Default);
//                    options.fullscreen = false;
//                }
//            }
//        }
//    }
//
//    void handleMouseInteraction() override {
//        if (mouseClickFlag) { // Check if a circle is found
//            // Set position of the found ball:
//            send_udp_message("NEWP," + thisBallPointer->GetIDStr() + "->" + Vector2fToString(currentMousePos));
//            if (!TouchedOnce)
//            {
//                thisBallPointer->SetOutline(outlineColor, 5);
//                // Get the current color
//                sf::Color currentColor = thisBallPointer->GetColor();
//                previousColor = currentColor;
//                // Darken the color by reducing the RGB values (without going below 0)
//                currentColor.r = std::max(0, currentColor.r - 15);
//                currentColor.g = std::max(0, currentColor.g - 15);
//                currentColor.b = std::max(0, currentColor.b - 15);
//
//                // Apply the new color to the rectangle
//                thisBallPointer->setColor(currentColor);
//                TouchedOnce = true;
//            }
//            else if (connectingMode && thisBallPointer != previousBallPointer)
//            {
//                send_udp_message("LINK->" + previousBallPointer->GetIDStr() + thisBallPointer->GetIDStr());
//            }
//            handleScaling();
//        }
//    }
//
//    void handleMouseWheel(sf::Event event) override {
//        if (event.type == sf::Event::MouseWheelScrolled) {
//            if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
//                //sf::Vector2f beforeZoom = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y), view);
//                if (event.mouseWheelScroll.delta > 0) {
//
//                    if (!scaleFlag) {
//                        view.zoom(1.f / ZOOM_FACTOR);
//                    }
//                    mouseFlagScrollUp = true;
//                }
//
//                else if (event.mouseWheelScroll.delta < 0) {
//                    if (!scaleFlag) {
//                        view.zoom(ZOOM_FACTOR);
//                    }
//                    mouseFlagScrollDown = true;
//                }
//
//                //sf::Vector2f afterZoom = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y), view);
//                //sf::Vector2f offset = beforeZoom - afterZoom;
//                //view.move(offset);
//            }
//        }
//    }
//
//    void handleScaling() override {
//        if (scaleFlag && mouseFlagScrollUp || mouseFlagScrollDown) {
//            if (Circle* circle = dynamic_cast<Circle*>(thisBallPointer)) {
//                send_udp_message("SCALE_C->" + circle->GetIDStr());
//            }
//            else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(thisBallPointer))
//            {
//                send_udp_message("SCALE_R->" + rectangle->GetIDStr());
//            }
//        }
//    }
//
//    //recive from server:
//    void MoveAndDrawObjects() override {
//        objectList.MoveAndDraw(window, currentFPS, elastic, planetMode, enableCollison, borderless);
//    }
//
//};
//
//void initializeWindow(sf::RenderWindow& window, sf::View view, sf::ContextSettings settings) {
//    window.create(
//        sf::VideoMode(options.window_width, options.window_height), "TomySim", sf::Style::Default, settings);
//    view = window.getDefaultView();
//    window.setVerticalSyncEnabled(true);
//    window.setFramerateLimit(60);
//}
//
//void toggleFullscreen(sf::RenderWindow& window) {
//    if (!options.fullscreen) {
//        window.create(options.desktopSize, "Fullscreen Mode", sf::Style::Fullscreen);
//        options.fullscreen = true;
//    }
//    else {
//        window.create(sf::VideoMode(1920, 980), "Tomy Mode", sf::Style::Default);
//        options.fullscreen = false;
//    }
//    window.setVerticalSyncEnabled(true);
//}
//
//void Run(std::string& screen, PhysicsSimulationLocal& simulation, MainMenu& mainMenu, Settings& settingsClass, sf::RenderWindow& window) {
//    while (window.isOpen()) {
//        if (screen == "START") {
//            screen = simulation.Run();
//        }
//        else if (screen == "MAIN MENU") {
//            screen = mainMenu.handleMainMenu();
//        }
//        else if (screen == "SETTINGS") {
//            screen = settingsClass.handleSettings();
//        }
//        else if (screen == "FULLSCREEN") {
//            toggleFullscreen(window);
//            screen = settingsClass.handleSettings();
//        }
//        else {
//            window.close();
//            break;  //  break to exit the loop when closing
//        }
//    }
//}
//
//
////int main() {
////    try {
////        const std::string server_ip = "127.0.0.1";  // or "localhost"
////        unsigned short tcp_port = 8080;
////        unsigned short udp_port = 8081;
////
////        std::cout << "Starting client..." << std::endl;
////        std::cout << "Attempting to connect to:" << std::endl;
////        std::cout << "Server IP: " << server_ip << std::endl;
////        std::cout << "TCP port: " << tcp_port << std::endl;
////        std::cout << "UDP port: " << udp_port << std::endl;
////
////        boost::asio::io_context io_context;
////
////        HandleNetworkingClient client(io_context, server_ip, tcp_port, udp_port);
////        client.connect();
////
////        // Start a thread to run the IO service
////        std::thread io_thread([&io_context]() {
////            io_context.run();
////            });
////
////        std::cout << "\nAvailable commands:" << std::endl;
////        std::cout << "- Type a message to send via TCP" << std::endl;
////        std::cout << "- Type 'udp:' followed by a message to send via UDP" << std::endl;
////        std::cout << "- Type 'stop' to stop connection attempts" << std::endl;
////        std::cout << "- Type 'connect' to start connection attempts" << std::endl;
////        std::cout << "- Type 'quit' to exit" << std::endl;
////
////        //ENGINE:
////        sf::RenderWindow window;
////        sf::ContextSettings settings;
////        settings.antialiasingLevel = 8;
////        sf::View view = window.getDefaultView();
////        initializeWindow(window, view, settings);
////
////        PhysicsSimulationLocal simulation(window);
////        MainMenu mainMenu(window);
////        Settings settingsClass(window);
////        std::string screen = "START";
////        Run(screen, simulation, mainMenu, settingsClass, window);
////
////        //ENGINE end
////
////        // Main loop to get user input and send messages
////        std::string input;
////        while (std::getline(std::cin, input)) {
////        
////            if (input == "quit") {
////                std::cout << "Shutting down client..." << std::endl;
////                client.stop_connecting();
////                break;
////            }
////            if (input == "stop") {
////                client.stop_connecting();
////                std::cout << "Stopped connection attempts." << std::endl;
////                continue;
////            }
////            if (input == "connect") {
////                std::cout << "Starting connection attempts..." << std::endl;
////                client.connect();
////                continue;
////            }
////
////            if (input.substr(0, 4) == "udp:") {
////                client.send_udp_message(input.substr(4));
////            }
////            else {
////                client.send_tcp_message(input);
////            }
////        }
////
////        io_context.stop();
////        io_thread.join();
////    }
////    catch (std::exception& e) {
////        std::cerr << "Fatal error: " << e.what() << std::endl;
////        return 1;
////    }
////
////    return 0;
////}