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

int BaseShape::objectCount = 0;

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

bool mainLoop(std::string& screen, PhysicsSimulationActions& simulationActions, PhysicsSimulationVisual& simulationVisual, Client& client, MainMenu& mainMenu, Settings& settingsClass, sf::RenderWindow& window) {
	// Main loop for the application
		if (screen == "OFFLINE") {
			simulationVisual.SetScreen(screen);
			screen = simulationActions.Run();
		}
		else if (screen == "ONLINE") {
			screen = client.Run();
		}
		else if (screen == "CONNECT_ONLINE") {
			screen = client.Run();
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
			client.disconnect_from_server();
			return false;  //  break to exit the loop when closing
		}
		return true;  // continue the loop
}

void restartSimulation(std::string& screen, PhysicsSimulationActions& simulationActions, PhysicsSimulationVisual& simulationVisual, Client& client, MainMenu& mainMenu, Settings& settingsClass, sf::RenderWindow& window) {
	client.Restart();
	simulationActions.Restart();
	mainLoop(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
}

void Run(std::string& screen, PhysicsSimulationActions& simulationActions, PhysicsSimulationVisual& simulationVisual,Client& client, MainMenu& mainMenu, Settings& settingsClass, sf::RenderWindow& window) {
	while (window.isOpen()) {
		try
		{
			if (!mainLoop(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window))
			{
				break;
			}
		}
		catch (const std::exception& error)
		{
			std::cerr << "Error in source - Run: " << error.what() << std::endl;
			restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
		}
		catch (const boost::system::system_error& error)
		{
			std::cerr << "Error in source - Boost: " << error.what() << std::endl;
			restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
		}
		catch (const std::runtime_error& error)
		{
			std::cerr << "Error in source - Runtime: " << error.what() << std::endl;
			restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
		}
		catch (const std::bad_alloc& error)
		{
			std::cerr << "Error in source - Bad Alloc: " << error.what() << std::endl;
			restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
		}
		catch (const std::out_of_range& error)
		{
			std::cerr << "Error in source - Out of Range: " << error.what() << std::endl;
			restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
		}
		catch (const std::invalid_argument& error)
		{
			std::cerr << "Error in source - Invalid Argument: " << error.what() << std::endl;
			restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
		}
		catch (const std::logic_error& error)
		{
			std::cerr << "Error in source - Logic Error: " << error.what() << std::endl;
			restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
		}
		catch (...)
		{
			std::cerr << "Unknown error occurred." << std::endl;
			restartSimulation(screen, simulationActions, simulationVisual, client, mainMenu, settingsClass, window);
		}
	}
}

int main() {
	try {
		//NETWORKING:
		const std::string server_ip = "10.100.102.19";  // or "localhost"
		unsigned short tcp_port = 8080;
		unsigned short udp_port = 8081;

		std::cout << "Starting client..." << std::endl;
		std::cout << "Attempting to connect to:" << std::endl;
		std::cout << "Server IP: " << server_ip << std::endl;
		std::cout << "TCP port: " << tcp_port << std::endl;
		std::cout << "UDP port: " << udp_port << std::endl;

		boost::asio::io_context io_context;

		sf::RenderWindow window;
		sf::ContextSettings settings;
		settings.antialiasingLevel = 8;

		sf::View view = window.getDefaultView();

		initializeWindow(window, view, settings);

		SinglePlayer simulation(window);
		MainMenu mainMenu(window);
		Settings settingsClass(window);
		std::string screen = "OFFLINE";

		Client client(window, io_context, server_ip, tcp_port, udp_port);
		client.connect();

		// start a thread to run the IO service
		std::thread io_thread([&io_context]() {
			io_context.run();
			});

		Run(screen, simulation, simulation, client, mainMenu, settingsClass, window); //Stops when exited/bug/network stopped etc..

		io_context.stop();
		io_thread.join();

		return 0;
	}
	catch (const std::exception& error) {
		std::cerr << "Error in source: " << error.what() << std::endl;
	}
}