//#pragma once
//#include <SFML/Graphics.hpp>;
//#include <SFML/Window.hpp>
//#include <sstream>
//#include <iostream> using namespace std;
//#include "LineLink.h"
//#include "ObjectsList.h"
//#include "Grid.h"
//#include "Button.h"
//#include "Rectangle.h"
//#include <random>  // For random number generation
//#include <ctime>   // For seeding with current time
//#include <boost\asio.hpp>
//#include "PhysicsSimulation.h"
//#include "SinglePlayer.h"
//#include "Options.h"
//#include "UI.h"
//
//int BaseShape::objectCount = 0;
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
//void Run(std::string& screen, SinglePlayer& simulation, MainMenu& mainMenu, Settings& settingsClass, sf::RenderWindow& window) {
//    while (window.isOpen()) {
//        if (screen == "START") {
//            simulation.SetScreen(screen);
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
//int main() {
//    try {
//        sf::RenderWindow window;
//        sf::ContextSettings settings;
//        settings.antialiasingLevel = 8;
//        sf::View view = window.getDefaultView();
//        initializeWindow(window, view, settings);
//
//        SinglePlayer simulation(window);
//        MainMenu mainMenu(window);
//        Settings settingsClass(window);
//        std::string screen = "START";
//
//        Run(screen, simulation, mainMenu, settingsClass, window);
//
//        return 0;
//    }
//    catch (const std::exception& error) {
//        std::cerr << "Error: " << error.what() << std::endl;
//        return 1;
//    }
//}