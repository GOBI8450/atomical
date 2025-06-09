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
#include "Slider.h"
#include "PhysicsSimulation.h"

//To run SinglePlayer
class SinglePlayer : public PhysicsSimulationVisual, public PhysicsSimulationActions {
protected:

	// Essential variables for the SinglePlayer simulation
#pragma region EssantialVariables

// Screen and window settings
	sf::VideoMode desktopSize = sf::VideoMode::getDesktopMode(); // Get the desktop resolution
	int window_height = options.window_height; // Window height from options
	int window_width = options.window_width;   // Window width from options
	bool fullscreen = options.fullscreen;      // Fullscreen mode flag
	int oldGravity = options.gravity;          // Previous gravity value
	double massLock = options.massLock;        // Mass lock for objects

	// Window and view settings
	sf::RenderWindow& window;                  // Reference to the main render window
	sf::View view;                             // View for rendering
	sf::ContextSettings settings;              // Context settings for the window
	const float zoomFactor = 1.5f;            // Zoom factor for the view
	float howMuchZoomed = 1;            // Zoom value

	// Cursor settings
	sf::Cursor handCursor;                     // Hand cursor for interaction
	sf::Cursor defaultCursor;                  // Default arrow cursor

	// Screen state
	std::string screen = "MAIN MENU";          // Current screen (e.g., MAIN MENU, GAME)

	// Interaction flags
	bool hovering = false;                     // Whether the mouse is hovering over an object
	bool connectingMode = false;               // Whether connecting mode is active
	bool createConnectedObjMode = false;       // Whether creating connected objects is active
	bool createChain = false;                  // Whether chain creation mode is active
	bool planetMode = false;                   // Whether planet creation mode is active
	bool enableCollison = true;                // Whether collision is enabled
	bool borderless = false;                   // Whether borderless mode is active
	bool renderButtonsMenu = true;             // Whether to render the buttons menu
	bool deletedSomething = false;             // Whether an object was deleted

	// Key-action mappings
	std::vector<std::pair<sf::Keyboard::Key, std::function<void()>>> keyActions; // Key-action pairs

	// Physics and simulation parameters
	float lineLength = 45;                     // Default line length for connections
	ObjectsList objectList;                    // List of objects in the simulation
	float deltaTime = 1.0f / 60.0f;            // Time step for physics updates
	float elastic = 0.0;                       // Elasticity for collisions
	int objCount = 0;                          // Object count
	float radius = 50;                         // Default radius for circles

	// Mouse and interaction state
	sf::Vector2f* previousMousePos = nullptr;  // Previous mouse position
	sf::Vector2f currentMousePos;              // Current mouse position
	bool leftMouseClickFlag = false;           // Left mouse button click flag
	bool rightMouseClickFlag = false;          // Right mouse button click flag
	bool mouseFlagScrollUp = false;            // Mouse scroll up flag
	bool mouseFlagScrollDown = false;          // Mouse scroll down flag
	int mouseScrollPower = 5;                  // Scroll power for scaling
	bool scaleFlag = false;                    // Whether scaling is active
	bool TouchedOnceLeftClick = false;         // Whether left click was touched once
	bool TouchedOnceRightClick = false;        // Whether right click was touched once
	float moveSpeedScreen = 30.f;              // Speed for moving the screen
	bool freeze = false;                       // Whether the simulation is frozen
	int typeOfLink = 1;                        // Type of link (1: fixed, 2: non-fixed, 3: custom)
	float textureResizer = 1.2;                // Texture resizing factor

	// Object pointers for interaction
	BaseShape* thisBallPointer = nullptr;      // Pointer to the currently selected object
	BaseShape* previousBallPointer = nullptr;  // Pointer to the previously selected object
	BaseShape* connecttableBallPointer = nullptr; // Pointer to the object being connected
	BaseShape* previousConnecttableBallPointer = nullptr; // Pointer to the previously connected object

	// Visual settings
	sf::Color ball_color = sf::Color(238, 238, 238); // Default ball color
	sf::Color proton_color = sf::Color(255, 222, 33); // Proton color
	sf::Color electron_color = sf::Color(106, 102, 157); // Electron color
	sf::Color ball_color2 = sf::Color(50, 5, 11); // Secondary ball color
	sf::Color background_color = sf::Color(30, 30, 30); // Background color
	sf::Color buttonColor = sf::Color(55, 58, 64); // Button color
	sf::Color outlineColor = sf::Color(255, 255, 255); // Outline color
	sf::Color previousColor = sf::Color(0, 0, 0); // Previous color for interaction
	sf::Color sideMenuColor = sf::Color(23, 23, 23, 204); // Transparent side menu color
	sf::Color startColorGradient = sf::Color(128, 0, 128); // Gradient start color (purple)
	sf::Color endColorGradient = sf::Color(0, 0, 255); // Gradient end color (blue)
	sf::Color startColorSlider = sf::Color(146, 0, 146); // Slider start color (dark purple)
	sf::Color endColorSlider = sf::Color(66, 0, 66); // Slider end color (darker purple)

	// Textures for buttons
	sf::Texture addButtonTexture;
	sf::Texture planetButtonTexture;
	sf::Texture trashButtonTexture;
	sf::Texture paticleButtonTexture;
	sf::Texture explosionButtonTexture;

	// Gradient settings
	short int gradientStep = 0;                // Current gradient step
	short int gradientStepMax = 400;           // Maximum gradient steps
	std::vector<sf::Color> gradient;           // Gradient colors

	// UI Elements
	sf::Font font;                             // Font for text
	sf::Text ballsCountText;                   // Text for ball count
	sf::Text fpsText;                          // Text for FPS
	sf::Text linkingText;                      // Text for linking status
	sf::RectangleShape sideMenuRec;            // Rectangle for the side menu
	std::vector<Button> buttons;               // Buttons in the UI

	// Sliders
	std::vector<Slider*> slidersVec;           // Vector of sliders
	Slider* gravitySlider = new Slider(300, 20, 200, 20, endColorSlider, startColorSlider, 0, 100, false); // Gravity slider
	Slider* colorSlider = new Slider(550, 20, 200, 20, sf::Color(0,0,0), sf::Color(255, 255, 255), 0, 360, true); // Color slider
	Slider* lineLengthSlider = new Slider(800, 20, 200, 20, sf::Color::Blue, sf::Color::Cyan, 0, 100, false); // Line length slider

	// Menu elements
	sf::RectangleShape headerText;             // Header text background
	std::vector<std::pair<Button, bool>> settingsButtonVec; // Settings buttons

	// Performance tracking
	sf::Clock clock;                           // Clock for frame timing
	sf::Clock fpsClock;                        // Clock for FPS calculation
	int frameCount = 0;                        // Frame count
	float currentFPS = 0.0f;                   // Current FPS

	// Object templates
	Circle* copyObjCir;                        // Template for circles
	RectangleClass* copyObjRec;                // Template for rectangles

	// Spawn settings
	float posYStartingPoint = 200;             // Y-coordinate for spawning objects
	int posXStartingPoint = radius;            // X-coordinate for spawning objects
	short int startingPointAdder = 31;         // Increment for spawning positions
	sf::Vector2f spawnStartingPoint;           // Starting point for spawning
	sf::Vector2f initialVel = sf::Vector2f(0, 0); // Initial velocity for spawned objects

	// Electricity settings
	int particleType = 3;                      // Type of particle (e.g., proton, electron)
	sf::Vector2f electronInitialVel = sf::Vector2f(0, 0); // Initial velocity for electrons

#pragma endregion

public:
	SinglePlayer(sf::RenderWindow& win) : PhysicsSimulationVisual(), PhysicsSimulationActions(),
		window(win),
		settings(8),
		objectList(lineLength),
		spawnStartingPoint(posXStartingPoint, posYStartingPoint)
	{
		view = window.getDefaultView();  // Initialize view from window
		initializeCursors();
		initializeUI();
		InitializeKeyActions();
		setupGradient();
	}

	// the main function to run the SinglePlayer simulation
	std::string Run() override{
		currentMousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
		handleAllEvents();
		renderSimulation();
		return screen;
	}

	// set the screen to a new value
	void SetScreen(std::string newScreen) override{
		screen = newScreen;
	}

	// Restart the simulation
	void Restart() override{
		objectList.DeleteAll();
		deletedSomething = true;
		objCount = 0;
		planetMode = false;
		connectingMode = false;
		previousColor = sf::Color(0, 0, 0);
		initialVel = sf::Vector2f(0, 0);

		// Mouse and interaction state
		previousMousePos = nullptr;
		currentMousePos = sf::Vector2f(0, 0);
		mouseFlagScrollUp = false;
		mouseFlagScrollDown = false;
		scaleFlag = false;
		TouchedOnceLeftClick = false;
		TouchedOnceRightClick = false;
		leftMouseClickFlag = false;
		rightMouseClickFlag = false;

		window.setMouseCursor(defaultCursor);
	}

	// Reset the view to the default state
	void ResetView() {
		howMuchZoomed = 1;
		view.setSize(options.window_width, window_height);
		view.setCenter(window_width / 2, window_height / 2);
		window.setView(view);
	}

private:
	// initialize key actions
	void InitializeKeyActions() {
		// Populate the key-action vector
		keyActions.push_back({ sf::Keyboard::Escape, [&]() {
			screen = "MAIN MENU";
			objectList.DeleteAll();
			deletedSomething = true;
			objCount = 0;
		} });

		keyActions.push_back({ sf::Keyboard::A, [&]() { AddCirclesInOrder(); } });
		keyActions.push_back({ sf::Keyboard::T, [&]() { AddRectanglesInOrder(); } });
		keyActions.push_back({ sf::Keyboard::K, [&]() {
			createConnectedObjMode = !createConnectedObjMode;
			previousBallPointer = thisBallPointer;
		} });

		keyActions.push_back({ sf::Keyboard::X, [&]() { ToggleChainMode(); } });
		keyActions.push_back({ sf::Keyboard::BackSpace, [&]() {
			if(!deletedSomething && leftMouseClickFlag){
				objectList.DeleteThisObj(thisBallPointer);
				window.setMouseCursor(defaultCursor);
				deletedSomething = true;
			}
		}});

		keyActions.push_back({ sf::Keyboard::H, [&]() { CreateRandomConnectedCircles(); } });
		keyActions.push_back({ sf::Keyboard::F11, [&]() { ToggleFullscreen(); } });
		keyActions.push_back({ sf::Keyboard::L, [&]() { createPlanet(); } });
		keyActions.push_back({ sf::Keyboard::E, [&]() { createElectricalParticle(particleType); } });
		keyActions.push_back({ sf::Keyboard::F, [&]() { createExplosionCircles(); } });
		keyActions.push_back({ sf::Keyboard::J, [&]() { createExplosionRectangles(); } });
		keyActions.push_back({ sf::Keyboard::R, [&]() { Restart(); } });
		keyActions.push_back({ sf::Keyboard::N, [&]() { ResetView(); } });
		keyActions.push_back({ sf::Keyboard::S, [&]() {
			if (leftMouseClickFlag) scaleFlag = true;
		} });

		keyActions.push_back({ sf::Keyboard::Space, [&]() { ToggleFreeze(); } });
		keyActions.push_back({ sf::Keyboard::Q, [&]() {
			objectList.CreateNewFixedCircle(sf::Color(255, 255, 255), currentMousePos);
			objCount += 1;
		} });

		keyActions.push_back({ sf::Keyboard::Num1, [&]() { typeOfLink = 1; } });
		keyActions.push_back({ sf::Keyboard::Num2, [&]() { typeOfLink = 2; } });
		keyActions.push_back({ sf::Keyboard::Num3, [&]() { particleType = 3; } });
		keyActions.push_back({ sf::Keyboard::Num4, [&]() { particleType = 4; } });
		keyActions.push_back({ sf::Keyboard::Num5, [&]() { particleType = 5; } });
		keyActions.push_back({ sf::Keyboard::Num6, [&]() { enableCollison = !enableCollison; } });
		keyActions.push_back({ sf::Keyboard::Num7, [&]() { borderless = !borderless; } });
		keyActions.push_back({ sf::Keyboard::Num0, [&]() { The3BodyProblem(); } });

		keyActions.push_back({ sf::Keyboard::Left, [&]() { view.move(-moveSpeedScreen * howMuchZoomed, 0.f); } });
		keyActions.push_back({ sf::Keyboard::Right, [&]() { view.move(moveSpeedScreen * howMuchZoomed, 0.f); } });
		keyActions.push_back({ sf::Keyboard::Up, [&]() { view.move(0.f, -moveSpeedScreen * howMuchZoomed); } });
		keyActions.push_back({ sf::Keyboard::Down, [&]() { view.move(0.f, moveSpeedScreen * howMuchZoomed); } });

	}

	// hadle events from the sfml events
	void handleEventsFromPollEvent(sf::Event event) override {
		if (event.type == sf::Event::Closed) { window.close(); }
		//if (event.type == sf::Event::Resized) {
		//	// resize my view
		//	window_width = event.size.width;
		//	window_height = event.size.height;
		//	view.setSize({
		//			static_cast<float>(event.size.width),
		//			static_cast<float>(event.size.height)
		//		});
		//	window.setView(view);
		//}
		handleKeyPress(event);
		handleMouseRelase(event);
		handleMouseWheel(event);
	}

	// Handles all events in the main loop
	void handleAllEvents() override {
		sf::Event event;
		while (window.pollEvent(event)) {
			handleEventsFromPollEvent(event);
		}
		handleMouseClick();
		handleScaling();
		handleMouseInteraction();
	}

	// handles key press events
	void handleKeyPress(sf::Event event) override {
		if (event.type == sf::Event::KeyPressed) {
			for (const auto& [key, action] : keyActions) {
				if (event.key.code == key) {
					action(); // Execute the lambda function for the key
					break;   // Exit once found the event
				}
			}
		}
	} 

	// Handles mouse clicks and interactions
	void handleMouseClick() override {
		//Left click:
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && leftMouseClickFlag == false && !deletedSomething) {
			thisBallPointer = objectList.IsInRadius(currentMousePos); // Check if a circle is within radius
			if (previousBallPointer == nullptr)
			{
				previousBallPointer = thisBallPointer;
			}
			if (thisBallPointer != nullptr) { // Check if a circle was found
				leftMouseClickFlag = true; // Set flag if circle found
				window.setMouseCursor(handCursor);
			}
			//Visuals:
			handleSliders();
		}

		//Right click:
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && rightMouseClickFlag == false) {
			connecttableBallPointer = objectList.IsInRadius(currentMousePos); // Check if a circle is within radius
			if (previousConnecttableBallPointer == nullptr)
			{
				previousConnecttableBallPointer = connecttableBallPointer;
			}
			if (connecttableBallPointer != nullptr) { // Check if a circle was found
				rightMouseClickFlag = true; // Set flag if circle found
				window.setMouseCursor(handCursor);
			}
		}
	}

	// handle mouse release events
	void handleMouseRelase(sf::Event event) override {
		if (event.type == sf::Event::MouseButtonReleased) {
			//Left mouse button:
			if (event.mouseButton.button == sf::Mouse::Left) {
				leftMouseClickFlag = false;
				scaleFlag = false;
				if (thisBallPointer != nullptr && !deletedSomething)
				{
					window.setMouseCursor(defaultCursor);
					thisBallPointer->setColor(previousColor);
					thisBallPointer->SetOutline(outlineColor, 0);
				}
				TouchedOnceLeftClick = false;
				deletedSomething = false;
			}

			//Right mouse button:
			if (event.mouseButton.button == sf::Mouse::Right) {
				window.setMouseCursor(defaultCursor);
				rightMouseClickFlag = false;
				TouchedOnceRightClick = false;
			}
		}
	}

	// Handles mouse interactions such as clicking and dragging
	void handleMouseInteraction() override {
		if (leftMouseClickFlag && !deletedSomething) { // Check if a circle is found
			thisBallPointer->SetPosition(currentMousePos); // Set position of the found ball
			if (!TouchedOnceLeftClick && thisBallPointer!=nullptr)
			{
				thisBallPointer->SetOutline(outlineColor, 5);
				// Get the current color
				sf::Color currentColor = thisBallPointer->GetColor();
				previousColor = currentColor;
				// Darken the color by reducing the RGB values (without going below 0)
				currentColor.r = std::max(0, currentColor.r - 15);
				currentColor.g = std::max(0, currentColor.g - 15);
				currentColor.b = std::max(0, currentColor.b - 15);

				// Apply the new color to the rectangle
				thisBallPointer->setColor(currentColor);
				TouchedOnceLeftClick = true;
			}
			handleScaling();
		}


		if (rightMouseClickFlag)
		{
			if (!TouchedOnceRightClick)
			{
				handleConnecting();
				toggleConnectingMode();
				TouchedOnceRightClick = true;
			}
		}
	}

	// Handles mouse wheel events for zooming or scaling
	void handleMouseWheel(sf::Event event) override {
		if (event.type == sf::Event::MouseWheelScrolled) {
			if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
				//sf::Vector2f beforeZoom = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y), view);
				if (event.mouseWheelScroll.delta > 0) {

					if (!scaleFlag) {
						view.zoom(1.f / zoomFactor);
						howMuchZoomed *= 1.f / zoomFactor;
					}
					mouseFlagScrollUp = true;
				}

				else if (event.mouseWheelScroll.delta < 0) {
					if (!scaleFlag) {
						view.zoom(zoomFactor);
						howMuchZoomed *= zoomFactor;
					}
					mouseFlagScrollDown = true;
				}

				//sf::Vector2f afterZoom = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y), view);
				//sf::Vector2f offset = beforeZoom - afterZoom;
				//view.move(offset);
			}
		}
	}

	// add circles in order with a gradient
	void AddCirclesInOrder() {
		for (size_t i = 0; i < 10; i++)
		{
			BaseShape* newObject = objectList.CreateNewCircle(options.gravity, gradient[gradientStep], spawnStartingPoint, initialVel);
			//newObject->SetOutline(outlineColor, 2);
			objCount += 1;
			gradientStep += 1;
			spawnStartingPoint.x += startingPointAdder;
			if (gradientStep == gradientStepMax) {
				std::reverse(gradient.begin(), gradient.end());
				gradientStep = 0;
			}
			if (spawnStartingPoint.x >= options.window_width - radius || spawnStartingPoint.x <= radius)
			{
				startingPointAdder *= -1;
			}
			objectList.connectedObjects.AddObject(newObject);
		}
	}

	// add rectangles in order with a gradient
	void AddRectanglesInOrder() {
		for (size_t i = 0; i < 10; i++)
		{
			objectList.CreateNewRectangle(options.gravity, gradient[gradientStep], spawnStartingPoint);
			objCount += 1;
			gradientStep += 1;
			spawnStartingPoint.x += startingPointAdder;
			if (gradientStep == gradientStepMax) {
				std::reverse(gradient.begin(), gradient.end());
				gradientStep = 0;
			}
			if (spawnStartingPoint.x >= options.window_width - radius || spawnStartingPoint.x <= radius)
			{
				startingPointAdder *= -1;
			}
		}
	}

	// Handles connecting mode of objects in the simulation
	void toggleConnectingMode() override {
		if (!connectingMode)
		{
			connectingMode = true;
			previousConnecttableBallPointer = connecttableBallPointer;
			linkingText.setString("ACTIVATED");
			linkingText.setFillColor(sf::Color::Magenta);
		}
		else {
			connectingMode = false;
			linkingText.setString("DEACTIVATED");
			linkingText.setFillColor(sf::Color::White);
		}
	}

	//toggle chain mode for creating chains of objects
	void ToggleChainMode() {
		createChain = !createChain;
		previousBallPointer = thisBallPointer;
	}

	// Toggles fullscreen mode for the window
	void ToggleFullscreen() {
		if (!options.fullscreen)
		{
			window.create(options.desktopSize, "Fullscreen Mode", sf::Style::Fullscreen);
			//view.setSize({
			//	static_cast<float>(options.desktopSize.width),
			//	static_cast<float>(options.desktopSize.height)
			//});
			window.setView(view);
			options.fullscreen = true;
		}
		else {
			window.create(sf::VideoMode(1920, 980), "Tomy Mode", sf::Style::Default);
			options.fullscreen = false;
		}
	}

	// create connected objects in the simulation
	void createConnectedObjects() override {
		BaseShape* newObject = objectList.CreateNewCircle(options.gravity, gradient[gradientStep], currentMousePos, initialVel);
		objCount++;
		objectList.connectedObjects.AddObject(newObject);
		objectList.connectedObjects.ConnectRandom(10, typeOfLink);
	}

	// create a planet in the simulation
	void createPlanet() override {
		planetMode = true;
		initialVel = sf::Vector2f(200, 0);
		if (initialVel == sf::Vector2f(0,0))
		{
			initialVel = sf::Vector2f(200, 0);
			objectList.ChangeVelocityForAll(initialVel);
		}
		objectList.CreateNewPlanet(70000000, ball_color, currentMousePos, 20, 5.9722 * pow(10, 16));
		objCount++;
	}

	// create a simulation of the 3-body problem
	void The3BodyProblem() {
		planetMode = true;
		objectList.CreateNewPlanet(7000, sf::Color(205, 28, 24), sf::Vector2f(990, 466.02540), 20, 5.9722 * pow(10, 16));
		objectList.CreateNewPlanet(7000, sf::Color(0, 71, 171), sf::Vector2f(1000 - 110, 600), 20, 5.9722 * pow(10, 16));
		objectList.CreateNewPlanet(7000, sf::Color(137, 243, 54), sf::Vector2f(1200 - 110, 600), 20, 5.9722 * pow(10, 16));
		objCount++;
	}

	// create an electrical particle in the simulation
	void createElectricalParticle(int particleType) {
		double muliplier = 100000000;
		if (particleType == 3)
		{
			objectList.CreateNewElectricalParticle(PROTON_CHARGE * muliplier, true, sf::Vector2f(0, 0), proton_color, currentMousePos, 20, PROTON_MASS * muliplier); //TODO: when interacting with objects do that the mass is appropriate or else super high speed
		}
		else if (particleType == 4)
		{
			objectList.CreateNewElectricalParticle(ELECTRON_CHARGE * muliplier, false, electronInitialVel, electron_color, currentMousePos, 20, ELECTRON_MASS * muliplier);
		}
		else if (particleType == 5)
		{
			objectList.CreateNewElectricalParticle(-PROTON_CHARGE * muliplier, false, sf::Vector2f(0, 0), electron_color, currentMousePos, 20, PROTON_MASS * muliplier); //TODO: when interacting with objects do that the mass is appropriate or else super high speed
		}
		objCount++;
	}

	// create random connected circles in the simulation
	void CreateRandomConnectedCircles() {
		BaseShape* newObject = objectList.CreateNewCircle(options.gravity, gradient[gradientStep], currentMousePos, initialVel);
		objCount += 1;
		objectList.connectedObjects.AddObject(newObject);
		objectList.connectedObjects.ConnectRandom(10, typeOfLink);
	}

	// Toggles the freeze state of the simulation
	void ToggleFreeze() {
		if (!freeze)
		{
			options.gravity = 0;
			objectList.ChangeGravityForAll(options.gravity);
			freeze = true;
		}
		else
		{
			freeze = false;
			options.gravity = oldGravity;
			objectList.ChangeGravityForAll(oldGravity);
		}
	}

	// Handles scaling of objects based on mouse scroll input
	void handleScaling() override {
		if (scaleFlag && (mouseFlagScrollUp || mouseFlagScrollDown)) {
			if (Circle* circle = dynamic_cast<Circle*>(thisBallPointer)) {
				scaleCircle(circle);
			}
			else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(thisBallPointer))
			{
				scaleRectangle(rectangle);
			}
		}
	}

	// Scales a circle or rectangle based on mouse scroll input
	void scaleCircle(Circle* circle) override {
		if (mouseFlagScrollDown && circle->getRadius() > 0.0001) {
			circle->SetRadiusAndCenter(circle->getRadius() - mouseScrollPower);
			circle->SetMass(circle->GetMass() - mouseScrollPower * 10);
			std::cout << circle->getRadius();
			mouseFlagScrollDown = false;
		}
		else {
			circle->SetRadiusAndCenter(circle->getRadius() + mouseScrollPower);
			circle->SetMass(circle->GetMass() + mouseScrollPower * 10);
			mouseFlagScrollUp = false;
		}
	}

	// Scales a rectangle based on mouse scroll input
	void scaleRectangle(RectangleClass* rectangle) override {
		if (mouseFlagScrollDown && rectangle->GetHeight() > 0.0001 && rectangle->GetWidth() > 0.0001) {
			rectangle->SetSizeAndOrigin(rectangle->GetWidth() - mouseScrollPower, rectangle->GetHeight() - mouseScrollPower);
			rectangle->SetMass(rectangle->GetMass() - mouseScrollPower * 10);
			mouseFlagScrollDown = false;
		}
		else {
			rectangle->SetSizeAndOrigin(rectangle->GetWidth() + mouseScrollPower, rectangle->GetHeight() + mouseScrollPower);
			rectangle->SetMass(rectangle->GetMass() + mouseScrollPower * 10);
			mouseFlagScrollUp = false;
		}
	}

	// Handles connecting objects in the simulation when in connecting mode
	void handleConnecting() override {
		if (connectingMode && connecttableBallPointer != previousConnecttableBallPointer)
		{
			objectList.connectedObjects.MakeNewLink(previousConnecttableBallPointer, connecttableBallPointer, typeOfLink);
		}
	}

	// Creates explosion of circles in the simulation
	void createExplosionCircles() override {
		objectList.CreateNewCircle(options.gravity, hueToRGB(options.explosionColor), sf::Vector2f(currentMousePos.x + 3, currentMousePos.y + 3), initialVel);
		for (size_t i = 0; i < 50; i++) {
			objectList.CreateNewCircle(options.gravity, hueToRGB(options.explosionColor), currentMousePos, initialVel);
			objCount++;
		}
	}

	// Creates explosion of rectangles in the simulation
	void createExplosionRectangles() override {
		objectList.CreateNewRectangle(options.gravity, hueToRGB(options.explosionColor), sf::Vector2f(currentMousePos.x + 3, currentMousePos.y + 3));
		for (size_t i = 0; i < 50; i++) {
			objectList.CreateNewRectangle(options.gravity, hueToRGB(options.explosionColor), currentMousePos);
			objCount++;
		}
	}

	// handles the sliders based on user input
	void handleSliders() {
		for (auto& slider:slidersVec)
		{
			if (slider->containMouse(currentMousePos, howMuchZoomed))
			{
				slider->handleClick(currentMousePos);
			}
		}
		UpdateValuesSliders();
	}

	// Updates the values of sliders and applies them to the simulation options
	void UpdateValuesSliders() {
		options.gravity = gravitySlider->getValue();
		options.explosionColor = colorSlider->getValue();
		oldGravity = options.gravity;
		objectList.ChangeGravityForAll(options.gravity);
		objectList.ChangeLineLengthForAll(lineLengthSlider->getValue() * 5);
	}

	//VISUALS:
	// Renders the simulation frame
	void renderSimulation() override {
		updateFPS();
		window.clear(background_color);
		window.setView(view);

		UpdateValuesSliders();
		MoveAndDrawObjects();

		window.setView(window.getDefaultView());
		renderTexts();
		RenderButtons();
		RenderSliders();

		window.display();

		limitFrameRate();
	}

	// Moves and draws objects in the simulation
	void MoveAndDrawObjects() override {
		if (!freeze)
		{
			objectList.MoveObjects(window_width, window_height, currentFPS, elastic, enableCollison, borderless);

		}
		else
		{
			objectList.MoveWhenFreeze(window_width, window_height, currentFPS, borderless);
		}

		objectList.DrawObjects(window, currentFPS, planetMode);
	}

	// initializes the user interface
	void initializeUI() override {
		loadResources();
		setupText();
		SetupHeaders();
		SetupSideMenu();
		SetupButtons();
		SetupSliders();
	}

	// sets up the sliders for user interaction
	void SetupSliders() {
		slidersVec.push_back(gravitySlider);
		slidersVec.push_back(colorSlider);
		slidersVec.push_back(lineLengthSlider);

		gravitySlider->setValue(options.gravity);
		colorSlider->setValue(options.explosionColor);
		lineLengthSlider->setValue(lineLength);
	}

	// Initializes the cursors for the window
	void initializeCursors() override {
		if (!defaultCursor.loadFromSystem(sf::Cursor::Arrow) ||
			!handCursor.loadFromSystem(sf::Cursor::Hand)) {
			throw std::runtime_error("Failed to load cursors");
		}
		window.setMouseCursor(defaultCursor);
	}

	// Loads resources such as fonts, icons, and textures
	void loadResources() override {
		if (!font.loadFromFile("visuals/font.ttf")) {
			throw std::runtime_error("Failed to load font");
		}
		sf::Image icon;
		if (!icon.loadFromFile("visuals/logo.png")) {
			throw std::runtime_error("Failed to load font");
		}
		window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
		loadTextures();
	}

	// Sets up the gradient for visual effects and colors
	void setupGradient() override {
		gradient = GenerateGradient(gradientStepMax);
	}

	// Generates a gradient of colors based on the start and end colors
	std::vector<sf::Color> GenerateGradient(int steps) override {
		std::vector<sf::Color> gradient;
		float stepR = (endColorGradient.r - startColorGradient.r) / static_cast<float>(steps - 1);
		float stepG = (endColorGradient.g - startColorGradient.g) / static_cast<float>(steps - 1);
		float stepB = (endColorGradient.b - startColorGradient.b) / static_cast<float>(steps - 1);

		for (int i = 0; i < steps; ++i) {
			gradient.push_back(sf::Color(
				startColorGradient.r + stepR * i,
				startColorGradient.g + stepG * i,
				startColorGradient.b + stepB * i
			));
		}
		return gradient;
	}

	// Loads textures for buttons and other UI elements
	void loadTextures() override {
		try {
			if (!addButtonTexture.loadFromFile("Visuals/Buttons/AddButton.png")) {
				throw std::runtime_error("Failed to load texture");
			}
			if (!planetButtonTexture.loadFromFile("Visuals/Buttons/PlanetButton.png")) {
				throw std::runtime_error("Failed to load texture");
			}
			if (!trashButtonTexture.loadFromFile("Visuals/Buttons/TrashButton.png")) {
				throw std::runtime_error("Failed to load texture");
			}
			if (!paticleButtonTexture.loadFromFile("Visuals/Buttons/ParticleButton.png")) {
				throw std::runtime_error("Failed to load texture");
			}
			if (!explosionButtonTexture.loadFromFile("Visuals/Buttons/ExplosionButton.png")) {
				throw std::runtime_error("Failed to load texture");
			}
			addButtonTexture.setSmooth(true);
			planetButtonTexture.setSmooth(true);
			trashButtonTexture.setSmooth(true);
			paticleButtonTexture.setSmooth(true);
			explosionButtonTexture.setSmooth(true);
		}
		catch (const std::exception& error) {
			std::cerr << "Texture loading error: " << error.what() << std::endl;
		}
	}

	// Updates the FPS counter and resets it every frame
	void updateFPS() override {
		frameCount++;
		float timeElapsed = fpsClock.getElapsedTime().asSeconds();

		if (timeElapsed >= 1.0f) {
			currentFPS = frameCount / timeElapsed;
			frameCount = 0;
			fpsClock.restart();
		}
	}

	// Sets up the text elements for the UI
	void setupText() override {
		// FPS Text
		fpsText.setFont(font);
		fpsText.setCharacterSize(20);
		fpsText.setFillColor(sf::Color::White);
		fpsText.setPosition(10, 10);

		// Balls Count Text
		ballsCountText.setFont(font);
		ballsCountText.setCharacterSize(20);
		ballsCountText.setFillColor(sf::Color::White);
		ballsCountText.setPosition(10, 40);

		// Linking Text
		linkingText.setFont(font);
		linkingText.setString("DEACTIVATED");
		linkingText.setCharacterSize(20);
		linkingText.setFillColor(sf::Color::White);
		linkingText.setPosition(10, 70);
	}

	// Sets up the side menu for the UI
	void SetupSideMenu() {
		sideMenuRec.setFillColor(sideMenuColor);
		sideMenuRec.setSize(sf::Vector2f(110, window_height));
		//sideMenuRec.setOrigin(sf::Vector2f(sideMenuRec.getSize().x / 2, sideMenuRec.getSize().y / 2));
		sideMenuRec.setPosition(window_width - 110, 0);
	}

	// Sets up the headers for the UI
	void SetupHeaders() override {
		headerText.setSize(sf::Vector2f(400.f, 100.f));
		headerText.setPosition(
			options.window_width / 2.f - headerText.getSize().x / 2.f,
			100.f
		);
		headerText.setFillColor(buttonColor);
	}

	// Sets up the buttons for the UI
	void SetupButtons() {
		int yAdder = 130;
		Button addButton = Button(85 / textureResizer, 85 / textureResizer,
			sf::Vector2f(options.window_width - 55, 60), "CIR");
		addButton.SetTexture(addButtonTexture);

		Button explosionButton = Button(85 / textureResizer, 85 / textureResizer,
			sf::Vector2f(options.window_width - 55, 60 + yAdder), "EXPLOSION");
		explosionButton.SetTexture(explosionButtonTexture);

		Button planetButton = Button(85 / textureResizer, 85 / textureResizer,
			sf::Vector2f(options.window_width - 55, 60 + yAdder*2), "PLANET");
		planetButton.SetTexture(planetButtonTexture);

		Button trashButton = Button(85 / textureResizer, 85 / textureResizer,
			sf::Vector2f(options.window_width - 55, 60 + yAdder*3), "RESTART");
		trashButton.SetTexture(trashButtonTexture);


		buttons.push_back(addButton);
		buttons.push_back(explosionButton);
		buttons.push_back(planetButton);
		buttons.push_back(trashButton);
	}

	// Renders the buttons in the side menu
	void RenderButtons() {
		window.draw(sideMenuRec);
		hovering = false;
		for (auto& button : buttons)
		{
			button.MouseHover(currentMousePos, hovering);
			button.draw(window);
		}
		std::string event = "";
		bool oldMouseClickFlag = leftMouseClickFlag;
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			leftMouseClickFlag = true;
		}
		if (leftMouseClickFlag)
		{
			for (auto& button : buttons)
			{
				if (button.IsInRadius(currentMousePos))
				{
					event = button.GetName();
				}
			}
		}
		ExectuteButtons(event);
		leftMouseClickFlag = oldMouseClickFlag;
	}

	// Renders the sliders
	void RenderSliders() {
		for (auto& slider :slidersVec)
		{
			slider->update(1 / 60);
			slider->draw(window);
		}
	}

	// Executes the button actions based on the event
	void ExectuteButtons(std::string event) {
		if (event != "")
		{
			if (event == "CIR")
			{
				AddCirclesInOrder();
			}
			else if (event == "EXPLOSION")
			{
				createExplosionCircles();
			}
			else if (event == "LINK")
			{
				toggleConnectingMode();
			}
			else if (event == "CHAIN")
			{
				ToggleChainMode();
			}
			else if (event == "PLANET")
			{
				createPlanet();
			}
			else if (event == "RESTART")
			{
				Restart();
			}
		}
	}

	// Renders the text elements on the screen
	void renderTexts() override {
		std::ostringstream fpsStream;
		std::ostringstream ballCountStream;

		fpsStream << "FPS: " << static_cast<int>(currentFPS);
		ballCountStream << "Balls Count: " << static_cast<int>(objCount);

		fpsText.setString(fpsStream.str());
		ballsCountText.setString(ballCountStream.str());

		window.draw(fpsText);
		window.draw(linkingText);
		window.draw(ballsCountText);
	}

	// Limits the frame rate to a specified fps(60 by default)
	void limitFrameRate() override {
		sf::Time elapsed = clock.restart();
		if (elapsed.asSeconds() < deltaTime) {
			sf::sleep(sf::seconds(deltaTime - elapsed.asSeconds()));
		}
	}

	// Converts a hue value to an RGB color for sliders
	sf::Color hueToRGB(float hue) {
		hue = fmod(hue, 360.f);
		if (hue < 0) hue += 360.f;

		float c = 1.0f;
		float x = 1.0f - fabs(fmod(hue / 60.f, 2.f) - 1.0f);

		float r = 0, g = 0, b = 0;
		if (hue < 60) { r = c; g = x; b = 0; }
		else if (hue < 120) { r = x; g = c; b = 0; }
		else if (hue < 180) { r = 0; g = c; b = x; }
		else if (hue < 240) { r = 0; g = x; b = c; }
		else if (hue < 300) { r = x; g = 0; b = c; }
		else { r = c; g = 0; b = x; }

		return sf::Color(
			static_cast<sf::Uint8>(r * 255),
			static_cast<sf::Uint8>(g * 255),
			static_cast<sf::Uint8>(b * 255)
		);
	}

	//TODO: I dont need it 
	std::vector<BaseShape> ConvertForSending() override {
		return std::vector<BaseShape>();
	}
};
