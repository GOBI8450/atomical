#pragma once
#include <SFML/Graphics.hpp>;
#include <SFML/Window.hpp>
#include <sstream>
#include <iostream> using namespace std;
#include "LineLink.h"
#include "ObjectsList.h"
#include "Grid.h"
#include "Button.h"
#include "Rectangle.h"
#include <random>  // For random number generation
#include <ctime>   // For seeding with current time
#include <boost\asio.hpp>
#include "PhysicsSimulation.h"
#include "SinglePlayer.h"
#include "Options.h"
#include "UI.h"
#include "Client.h"
#include <windows.h>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

// Static member initialization for BaseShape
int BaseShape::objectCount = 0;

// Function to initialize the SFML window with specific settings
void initializeWindow(sf::RenderWindow& window, sf::View view, sf::ContextSettings settings) {
    // Create the window with specified dimensions and settings
    window.create(
        sf::VideoMode(options.window_width, options.window_height), "TomySim", sf::Style::Default, settings);
    view = window.getDefaultView(); // Set the default view
    window.setVerticalSyncEnabled(true); // Enable vertical sync
    window.setFramerateLimit(60); // Limit the frame rate to 60 FPS
}

// Function to toggle between fullscreen and windowed mode
void toggleFullscreen(sf::RenderWindow& window) {
    if (!options.fullscreen) {
        // Switch to fullscreen mode
        window.create(options.desktopSize, "Fullscreen Mode", sf::Style::Fullscreen);
        options.fullscreen = true;
    }
    else {
        // Switch to windowed mode
        window.create(sf::VideoMode(1920, 980), "Tomy Mode", sf::Style::Default);
        options.fullscreen = false;
    }
    window.setVerticalSyncEnabled(true); // Enable vertical sync
}

// Main loop for the application, handling different screens and states
bool mainLoop(std::string& screen, PhysicsSimulationActions& simulationActions, PhysicsSimulationVisual& simulationVisual, Client& client, MainMenu& mainMenu, Settings& settingsClass, sf::RenderWindow& window) {
    if (screen == "OFFLINE") {
        // Handle offline simulation
        simulationVisual.SetScreen(screen);
        screen = simulationActions.Run();
    }
    else if (screen == "ONLINE") {
        // Handle online simulation
        screen = client.Run();
    }
    else if (screen == "CONNECT_ONLINE") {
        // Handle connecting to online server
        screen = client.Run();
    }
    else if (screen == "MAIN MENU") {
        // Handle main menu
        screen = mainMenu.handleMainMenu();
    }
    else if (screen == "SETTINGS") {
        // Handle settings menu
        screen = settingsClass.handleSettings();
    }
    else if (screen == "FULLSCREEN") {
        // Toggle fullscreen mode
        toggleFullscreen(window);
        screen = settingsClass.handleSettings();
    }
    else {
        // Exit the application
        window.close();
        client.disconnect_from_server();
        return false; // Exit the loop
    }
    return true; // Continue the loop
}

// Function to restart the simulation and re-enter the main loop
void restartSimulation(std::string& screen, PhysicsSimulationActions& simulationActions, PhysicsSimulationVisual& simulationVisual, Client& client, MainMenu& mainMenu, Settings& settingsClass, sf::RenderWindow& window) {
    client.Restart(); // Restart the client
    simulationActions.Restart(); // Restart the simulation actions
    mainLoop(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window); // Re-enter the main loop
}

// Main application run loop, handling exceptions and restarting if necessary
void Run(std::string& screen, PhysicsSimulationActions& simulationActions, PhysicsSimulationVisual& simulationVisual, Client& client, MainMenu& mainMenu, Settings& settingsClass, sf::RenderWindow& window) {
    while (window.isOpen()) {
        try {
            // Run the main loop
            if (!mainLoop(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window)) {
                break; // Exit the loop if the main loop returns false
            }
        }
        catch (const std::exception& error) {
            // Handle standard exceptions
            std::cerr << "Error in source - Run: " << error.what() << std::endl;
            restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
        }
        catch (const boost::system::system_error& error) {
            // Handle Boost system errors
            std::cerr << "Error in source - Boost: " << error.what() << std::endl;
            restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
        }
        catch (const std::runtime_error& error) {
            // Handle runtime errors
            std::cerr << "Error in source - Runtime: " << error.what() << std::endl;
            restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
        }
        catch (const std::bad_alloc& error) {
            // Handle memory allocation errors
            std::cerr << "Error in source - Bad Alloc: " << error.what() << std::endl;
            restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
        }
        catch (const std::out_of_range& error) {
            // Handle out-of-range errors
            std::cerr << "Error in source - Out of Range: " << error.what() << std::endl;
            restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
        }
        catch (const std::invalid_argument& error) {
            // Handle invalid argument errors
            std::cerr << "Error in source - Invalid Argument: " << error.what() << std::endl;
            restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
        }
        catch (const std::logic_error& error) {
            // Handle logic errors
            std::cerr << "Error in source - Logic Error: " << error.what() << std::endl;
            restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
        }
        catch (...) {
            // Handle unknown errors
            std::cerr << "Unknown error occurred." << std::endl;
            restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
        }
    }
}

// Entry point of the application
int main() {
    try {
        // Networking setup
        const std::string server_ip = "127.0.0.1"; // Server IP address
        unsigned short tcp_port = 33333;
        unsigned short udp_port = 44444;

        // Display connection details
        std::cout << "Starting client..." << std::endl;
        std::cout << "Attempting to connect to:" << std::endl;
        std::cout << "Server IP: " << server_ip << std::endl;
        std::cout << "TCP port: " << tcp_port << std::endl;
        std::cout << "UDP port: " << udp_port << std::endl;

        // Boost ASIO context for networking
        boost::asio::io_context io_context;

        // SFML window and settings
        sf::RenderWindow window;
        sf::ContextSettings settings;
        settings.antialiasingLevel = 8; // Enable antialiasing

        sf::View view = window.getDefaultView();

        // Initialize the window
        initializeWindow(window, view, settings);

        // Initialize simulation and UI components
        SinglePlayer simulation(window);
        MainMenu mainMenu(window);
        Settings settingsClass(window);
        std::string screen = "OFFLINE"; // Initial screen state

        // Initialize the client and connect to the server
        Client client(window, io_context, server_ip, tcp_port, udp_port);
        client.connect();

        // Start a thread to run the IO service
        std::thread io_thread([&io_context]() {
            io_context.run();
            });

        // Run the main application loop
        Run(screen, simulation, simulation, client, mainMenu, settingsClass, window);

        // Stop the IO context and join the thread
        io_context.stop();
        io_thread.join();

        return 0; // Exit successfully
    }
    catch (const std::exception& error) {
        // Handle exceptions during initialization
        std::cerr << "Error in source: " << error.what() << std::endl;
    }
}