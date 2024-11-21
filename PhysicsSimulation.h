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

//Abstract class
class PhysicsSimulationActions
{
public:


	PhysicsSimulationActions() {}

	virtual ~PhysicsSimulationActions() = default;

protected:

	virtual void handleEvent(sf::Event event) = 0;

	virtual void handleSimulationEvents() = 0;

	virtual void handleKeyPress(sf::Event event) = 0;

	virtual void handleMouseClick() = 0;

	virtual void handleMouseRelase(sf::Event event) = 0;

	virtual void handleMouseInteraction() = 0;

	virtual void handleMouseWheel(sf::Event event) = 0;

	// Helper methods for the main handlers
	virtual void toggleConnectingMode() = 0;

	virtual void createConnectedObjects() = 0;

	virtual void createPlanet() = 0;

	virtual void handleScaling() = 0;

	virtual void scaleCircle(Circle* circle) = 0;

	virtual void scaleRectangle(RectangleClass* rectangle) = 0;

	virtual void handleConnecting() = 0;

	virtual void createExplosionCircles() = 0;

	virtual void createExplosionRectangles() = 0;
};

//Abstract class
class PhysicsSimulationVisual {
public:
	PhysicsSimulationVisual() {}

	virtual void initializeCursors() = 0;

	virtual void loadResources() = 0;

	virtual void initializeUI() = 0;

	virtual void setupGradient() = 0;

	virtual std::string Run() = 0;

protected:
	virtual void renderSimulation() = 0;

	virtual void MoveAndDrawObjects() = 0;

	virtual std::vector<sf::Color> GenerateGradient(sf::Color startColor, sf::Color endColor, int steps) = 0;

	virtual void loadTextures() = 0;

	virtual void updateFPS() = 0;

	virtual void setupText() = 0;

	virtual void setupHeaders() = 0;

	virtual void renderTexts() = 0;

	virtual void limitFrameRate() = 0;

	virtual std::vector<BaseShape> ConvertForSending() = 0;
};

