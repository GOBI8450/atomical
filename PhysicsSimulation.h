#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <sstream>
#include <iostream>
#include <random>
#include <ctime>
#include "LineLink.h"
#include "ObjectsList.h"
#include "Grid.h" 
#include "Button.h"
#include "Rectangle.h"
#include <boost\asio.hpp>
#include "Options.h"

// Abstract class for handling simulation actions (e.g., input, object creation)
class PhysicsSimulationActions
{
public:
    // Default constructor
    PhysicsSimulationActions() {}

    // Virtual destructor for polymorphism
    virtual ~PhysicsSimulationActions() = default;

    // Runs the simulation logic (to be implemented by derived classes)
    virtual std::string Run() { return ""; };

    // Restarts the simulation (to be implemented by derived classes)
    virtual void Restart() {};

protected:
    // Handles events from SFML's event polling system
    virtual void handleEventsFromPollEvent(sf::Event event) {};

    // Handles all events (e.g., input, interactions)
    virtual void handleAllEvents() {};

    // Handles key press events
    virtual void handleKeyPress(sf::Event event) {};

    // Handles mouse click events
    virtual void handleMouseClick() {};

    // Handles mouse release events
    virtual void handleMouseRelase(sf::Event event) {};

    // Handles mouse interactions (e.g., dragging, hovering)
    virtual void handleMouseInteraction() {};

    // Handles mouse wheel events (e.g., zooming)
    virtual void handleMouseWheel(sf::Event event) {};

    // Toggles the mode for connecting objects
    virtual void toggleConnectingMode() {};

    // Creates connected objects (e.g., linked shapes)
    virtual void createConnectedObjects() {};

    // Creates a planet object
    virtual void createPlanet() {};

    // Handles scaling of objects (e.g., resizing)
    virtual void handleScaling() {};

    // Scales a circle object
    virtual void scaleCircle(Circle* circle) {};

    // Scales a rectangle object
    virtual void scaleRectangle(RectangleClass* rectangle) {};

    // Handles connecting objects (e.g., linking shapes)
    virtual void handleConnecting() {};

    // Creates an explosion of circles
    virtual void createExplosionCircles() {};

    // Creates an explosion of rectangles
    virtual void createExplosionRectangles() {};
};

// Abstract class for handling simulation visuals (e.g., rendering, UI)
class PhysicsSimulationVisual {
public:
    // Default constructor
    PhysicsSimulationVisual() {}

    // Initializes the mouse cursors
    virtual void initializeCursors() = 0;

    // Loads resources (e.g., textures, fonts)
    virtual void loadResources() = 0;

    // Initializes the user interface
    virtual void initializeUI() = 0;

    // Sets up a gradient background
    virtual void setupGradient() {};

    // Sets the current screen (e.g., main menu, simulation)
    virtual void SetScreen(std::string newScreen) = 0;

protected:
    // Renders the simulation (to be implemented by derived classes)
    virtual void renderSimulation() = 0;

    // Moves and draws objects in the simulation
    virtual void MoveAndDrawObjects() = 0;

    // Generates a gradient with a specified number of steps
    virtual std::vector<sf::Color> GenerateGradient(int steps) { return std::vector<sf::Color>(); };

    // Loads textures for the simulation
    virtual void loadTextures() = 0;

    // Updates the frames per second (FPS) counter
    virtual void updateFPS() = 0;

    // Sets up text elements (e.g., labels, counters)
    virtual void setupText() = 0;

    // Sets up headers for the UI
    virtual void SetupHeaders() = 0;

    // Renders text elements on the screen
    virtual void renderTexts() = 0;

    // Limits the frame rate of the simulation
    virtual void limitFrameRate() = 0;

    // Converts objects for sending over a network
    virtual std::vector<BaseShape> ConvertForSending() = 0;
};