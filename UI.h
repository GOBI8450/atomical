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
#include "PhysicsSimulatotion.h"
#include "Options.h"

class MainMenu
{
private:
    sf::RenderWindow& window;
    sf::Color mainMenuBackgroundColor = sf::Color(25, 25, 25);
    std::vector<std::pair<Button, bool>> mainMenuButtonVec;
    sf::RectangleShape headerText;
    bool hovering = false;
    bool mouseClickFlag = false;
    sf::Vector2f currentMousePos;
    float textureResizer = 1.8;
    float headerTextureResizer = 1.2;

    // Textures
    sf::Texture startButtonTexture;
    sf::Texture exitButtonTexture;
    sf::Texture settingsButtonTexture;
    sf::Texture mainMenuHeaderTexture;


public:
    MainMenu(sf::RenderWindow& window) :window(window) {
        if (!loadTextures()) {
            throw std::runtime_error("Failed to load textures");
        }
        setupButtons();
    };

    bool loadTextures() {
        if (!startButtonTexture.loadFromFile("START.png")) return false;
        if (!exitButtonTexture.loadFromFile("EXIT.png")) return false;
        if (!settingsButtonTexture.loadFromFile("SETTINGS.png")) return false;
        if (!mainMenuHeaderTexture.loadFromFile("Header.png")) return false;

        startButtonTexture.setSmooth(true);
        exitButtonTexture.setSmooth(true);
        settingsButtonTexture.setSmooth(true);
        mainMenuHeaderTexture.setSmooth(true);

        return true;
    }

    void setupButtons() {
        headerText = sf::RectangleShape(sf::Vector2f(1212 / headerTextureResizer, 80 / headerTextureResizer));
        headerText.setOrigin(headerText.getSize().x / 2, headerText.getSize().y / 2);
        headerText.setPosition(options.window_width / 2, options.window_height / 2 - 350);
        headerText.setTexture(&mainMenuHeaderTexture);

        Button startButtonMainMenu = Button(534 / textureResizer, 274 / textureResizer,
            sf::Vector2f(options.window_width / 2, 450), "START");
        Button settingsButtonMainMenu = Button(534 / textureResizer, 274 / textureResizer,
            sf::Vector2f(options.window_width / 2, 675), "SETTINGS");
        Button exitButtonMainMenu = Button(534 / textureResizer, 274 / textureResizer,
            sf::Vector2f(options.window_width / 2, 925), "EXIT");

        startButtonMainMenu.SetTexture(startButtonTexture);
        settingsButtonMainMenu.SetTexture(settingsButtonTexture);
        exitButtonMainMenu.SetTexture(exitButtonTexture);

        mainMenuButtonVec.push_back(std::make_pair(startButtonMainMenu, false));
        mainMenuButtonVec.push_back(std::make_pair(settingsButtonMainMenu, false));
        mainMenuButtonVec.push_back(std::make_pair(exitButtonMainMenu, false));
    }


    std::string handleMainMenu() {
        window.setTitle("Main Menu");

        sf::Vector2u windowSize = window.getSize();
        sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosFloat = static_cast<sf::Vector2f>(currentMousePos);
        bool mouseClickFlag = false;
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) { window.close(); }
            if (event.type == sf::Event::MouseButtonReleased) { mouseClickFlag = true; }
        }
        window.clear(mainMenuBackgroundColor);
        if (mouseClickFlag)
        {
            for (auto& pair : mainMenuButtonVec)
            {
                if (pair.first.IsInRadius(mousePosFloat))
                {
                    return pair.first.GetName();
                }
            }
        }
        for (auto& button : mainMenuButtonVec)
        {
            button.second = button.first.MouseHover(mousePosFloat, hovering);
        }
        window.draw(headerText);
        for (auto& button : mainMenuButtonVec)
        {

            button.first.draw(window);
        }

        window.display();
        return "MAIN MENU";
    }
};

class Settings
{
private:
    sf::RenderWindow& window;
    sf::Color settingsBackgroundColor = sf::Color(25, 25, 25);
    std::vector<std::pair<Button, bool>> settingsButtonVec;
    sf::RectangleShape headerText;
    bool hovering = false;
    bool mouseClickFlag = false;
    float textureResizer = 1.8;
    float headerTextureResizer = 1.2;

    sf::Texture fullscreenButtonTexture;
    sf::Texture exitButtonTexture;
    sf::Texture settingsHeaderTexture;

public:
    Settings(sf::RenderWindow& window) : window(window) {
        if (!loadTextures()) {
            throw std::runtime_error("Failed to load textures");
        }
        setupButtons();
    }

    bool loadTextures() {
        if (!fullscreenButtonTexture.loadFromFile("FULLSCREEN.png")) { return false; }
        if (!settingsHeaderTexture.loadFromFile("SETTINGS_HEADER.png")) { return false; }
        if (!exitButtonTexture.loadFromFile("EXIT.png")) { return false; }

        exitButtonTexture.setSmooth(true);
        settingsHeaderTexture.setSmooth(true);
        fullscreenButtonTexture.setSmooth(true);

        return true;
    }

    void setupButtons() {
        headerText = sf::RectangleShape(sf::Vector2f(555 / headerTextureResizer, 79 / headerTextureResizer));
        headerText.setOrigin(headerText.getSize().x / 2, headerText.getSize().y / 2);
        headerText.setPosition(options.window_width / 2, options.window_height / 2 - 350);

        Button fullscreenButton = Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(options.window_width / 2, 350), "FULLSCREEN");
        Button exitButtonSettings = Button(534 / textureResizer, 274 / textureResizer, sf::Vector2f(options.window_width / 2, 550), "MAIN MENU");

        fullscreenButton.SetTexture(fullscreenButtonTexture);
        headerText.setTexture(&settingsHeaderTexture);
        exitButtonSettings.SetTexture(exitButtonTexture);

        settingsButtonVec.push_back(std::make_pair(fullscreenButton, false));
        settingsButtonVec.push_back(std::make_pair(exitButtonSettings, false));
    }

    std::string handleSettings() {
        window.setTitle("Settings");

        sf::Vector2f mousePosFloat = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
        bool mouseClickFlag = false;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                mouseClickFlag = true;
            }
        }

        window.clear(settingsBackgroundColor);

        // Reset hover state
        hovering = false;

        // Update hover states
        for (auto& button : settingsButtonVec) {
            button.second = button.first.MouseHover(mousePosFloat, hovering);
        }

        // Check for clicks
        if (mouseClickFlag) {
            for (auto& button : settingsButtonVec) {
                if (button.first.IsInRadius(mousePosFloat)) {
                    return button.first.GetName();
                }
            }
        }

        // Draw everything
        window.draw(headerText);
        for (auto& button : settingsButtonVec) {
            button.first.draw(window);
        }

        window.display();
        return "SETTINGS";
    }
};

