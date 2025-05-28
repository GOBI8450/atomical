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
#include "Options.h"

// Class representing the main menu of the application
class MainMenu
{
private:
    sf::RenderWindow& window; // Reference to the main application window
    sf::Color mainMenuBackgroundColor = sf::Color(25, 25, 25); // Background color for the main menu
    std::vector<std::pair<Button, bool>> mainMenuButtonVec; // Vector of buttons and their hover states
    sf::RectangleShape headerText; // Header text for the main menu
    bool hovering = false; // Flag to track if the mouse is hovering over a button
    bool mouseClickFlag = false; // Flag to track if the mouse was clicked
    sf::Vector2f currentMousePos; // Current mouse position
    float textureResizer = 1.8; // Resizer for button textures
    float headerTextureResizer = 1.2; // Resizer for header texture

    // Textures for the main menu buttons and header
    sf::Texture offlineButtonTexture;
    sf::Texture onlineButtonTexture;
    sf::Texture exitButtonTexture;
    sf::Texture settingsButtonTexture;
    sf::Texture mainMenuHeaderTexture;

public:
    // Constructor: Initializes the main menu and loads textures
    MainMenu(sf::RenderWindow& window) : window(window) {
        if (!loadTextures()) {
            throw std::runtime_error("Failed to load textures");
        }
        setupButtons();
    };

    // Loads textures for the main menu buttons and header
    bool loadTextures() {
        if (!offlineButtonTexture.loadFromFile("visuals/OFFLINE.png")) return false;
        if (!onlineButtonTexture.loadFromFile("visuals/ONLINE.png")) return false;
        if (!exitButtonTexture.loadFromFile("visuals/EXIT.png")) return false;
        if (!settingsButtonTexture.loadFromFile("visuals/SETTINGS.png")) return false;
        if (!mainMenuHeaderTexture.loadFromFile("visuals/Header.png")) return false;

        // Enable smoothing for all textures
        offlineButtonTexture.setSmooth(true);
        onlineButtonTexture.setSmooth(true);
        exitButtonTexture.setSmooth(true);
        settingsButtonTexture.setSmooth(true);
        mainMenuHeaderTexture.setSmooth(true);

        return true;
    }

    // Sets up the buttons for the main menu
    void setupButtons() {
        // Configure the header text
        headerText = sf::RectangleShape(sf::Vector2f(922 / headerTextureResizer, 240 / headerTextureResizer));
        headerText.setOrigin(headerText.getSize().x / 2, headerText.getSize().y / 2);
        headerText.setPosition(options.window_width / 2, options.window_height / 2 - 350);
        headerText.setTexture(&mainMenuHeaderTexture);

        // Create buttons for the main menu
        Button offlineButtonMainMenu = Button(534 / textureResizer, 274 / textureResizer,
            sf::Vector2f(options.window_width / 2 - 200, 450), "OFFLINE");
        Button onlineButtonMainMenu = Button(534 / textureResizer, 274 / textureResizer,
            sf::Vector2f(options.window_width / 2 + 200, 450), "ONLINE");
        Button settingsButtonMainMenu = Button(534 / textureResizer, 274 / textureResizer,
            sf::Vector2f(options.window_width / 2, 675), "SETTINGS");
        Button exitButtonMainMenu = Button(534 / textureResizer, 274 / textureResizer,
            sf::Vector2f(options.window_width / 2, 925), "EXIT");

        // Assign textures to buttons
        offlineButtonMainMenu.SetTexture(offlineButtonTexture);
        onlineButtonMainMenu.SetTexture(onlineButtonTexture);
        settingsButtonMainMenu.SetTexture(settingsButtonTexture);
        exitButtonMainMenu.SetTexture(exitButtonTexture);

        // Add buttons to the vector
        mainMenuButtonVec.push_back(std::make_pair(offlineButtonMainMenu, false));
        mainMenuButtonVec.push_back(std::make_pair(onlineButtonMainMenu, false));
        mainMenuButtonVec.push_back(std::make_pair(settingsButtonMainMenu, false));
        mainMenuButtonVec.push_back(std::make_pair(exitButtonMainMenu, false));
    }

    // Handles the main menu logic, including button interactions
    std::string handleMainMenu() {
        window.setTitle("Main Menu");

        sf::Vector2u windowSize = window.getSize();
        sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosFloat = static_cast<sf::Vector2f>(currentMousePos);
        bool mouseClickFlag = false;
        sf::Event event;

        // Handle events
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { window.close(); }
            if (event.type == sf::Event::MouseButtonReleased) { mouseClickFlag = true; }
        }

        // Clear the window with the background color
        window.clear(mainMenuBackgroundColor);

        // Check for button clicks
        if (mouseClickFlag) {
            for (auto& pair : mainMenuButtonVec) {
                if (pair.first.IsInRadius(mousePosFloat)) {
                    return pair.first.GetName(); // Return the name of the clicked button
                }
            }
        }

        // Update hover states for buttons
        for (auto& button : mainMenuButtonVec) {
            button.second = button.first.MouseHover(mousePosFloat, hovering);
        }

        // Draw the header and buttons
        window.draw(headerText);
        for (auto& button : mainMenuButtonVec) {
            button.first.draw(window);
        }

        window.display();
        return "MAIN MENU"; // Default return value
    }
};

// Class representing the settings menu of the application
class Settings
{
private:
    sf::RenderWindow& window; // Reference to the main application window
    sf::Color settingsBackgroundColor = sf::Color(25, 25, 25); // Background color for the settings menu
    std::vector<std::pair<Button, bool>> settingsButtonVec; // Vector of buttons and their hover states
    sf::RectangleShape headerText; // Header text for the settings menu
    bool hovering = false; // Flag to track if the mouse is hovering over a button
    bool mouseClickFlag = false; // Flag to track if the mouse was clicked
    float textureResizer = 1.8; // Resizer for button textures
    float headerTextureResizer = 1.2; // Resizer for header texture

    // Textures for the settings menu buttons and header
    sf::Texture fullscreenButtonTexture;
    sf::Texture exitButtonTexture;
    sf::Texture settingsHeaderTexture;

public:
    // Constructor: Initializes the settings menu and loads textures
    Settings(sf::RenderWindow& window) : window(window) {
        if (!loadTextures()) {
            throw std::runtime_error("Failed to load textures");
        }
        setupButtons();
    }

    // Loads textures for the settings menu buttons and header
    bool loadTextures() {
        if (!fullscreenButtonTexture.loadFromFile("visuals/FULLSCREEN.png")) { return false; }
        if (!settingsHeaderTexture.loadFromFile("visuals/SETTINGS_HEADER.png")) { return false; }
        if (!exitButtonTexture.loadFromFile("visuals/EXIT.png")) { return false; }

        // Enable smoothing for all textures
        exitButtonTexture.setSmooth(true);
        settingsHeaderTexture.setSmooth(true);
        fullscreenButtonTexture.setSmooth(true);

        return true;
    }

    // Sets up the buttons for the settings menu
    void setupButtons() {
        // Configure the header text
        headerText = sf::RectangleShape(sf::Vector2f(555 / headerTextureResizer, 79 / headerTextureResizer));
        headerText.setOrigin(headerText.getSize().x / 2, headerText.getSize().y / 2);
        headerText.setPosition(options.window_width / 2, options.window_height / 2 - 350);

        // Create buttons for the settings menu
        Button fullscreenButton = Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(options.window_width / 2, 350), "FULLSCREEN");
        Button exitButtonSettings = Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(options.window_width / 2, 550), "MAIN MENU");

        // Assign textures to buttons
        fullscreenButton.SetTexture(fullscreenButtonTexture);
        headerText.setTexture(&settingsHeaderTexture);
        exitButtonSettings.SetTexture(exitButtonTexture);

        // Add buttons to the vector
        settingsButtonVec.push_back(std::make_pair(fullscreenButton, false));
        settingsButtonVec.push_back(std::make_pair(exitButtonSettings, false));
    }

    // Handles the settings menu logic, including button interactions
    std::string handleSettings() {
        window.setTitle("Settings");

        sf::Vector2f mousePosFloat = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
        bool mouseClickFlag = false;

        sf::Event event;

        // Handle events
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                mouseClickFlag = true;
            }
        }

        // Clear the window with the background color
        window.clear(settingsBackgroundColor);

        // Reset hover state
        hovering = false;

        // Update hover states for buttons
        for (auto& button : settingsButtonVec) {
            button.second = button.first.MouseHover(mousePosFloat, hovering);
        }

        // Check for button clicks
        if (mouseClickFlag) {
            for (auto& button : settingsButtonVec) {
                if (button.first.IsInRadius(mousePosFloat)) {
                    return button.first.GetName(); // Return the name of the clicked button
                }
            }
        }

        // Draw the header and buttons
        window.draw(headerText);
        for (auto& button : settingsButtonVec) {
            button.first.draw(window);
        }

        window.display();
        return "SETTINGS"; // Default return value
    }
};