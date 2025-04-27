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

#pragma region EssantialVariables
	sf::VideoMode desktopSize = sf::VideoMode::getDesktopMode();
	int window_height = options.window_height;
	int window_width = options.window_width;
	bool fullscreen = options.fullscreen;
	int oldGravity = options.gravity;
	double massLock = options.massLock;


	// Window and view settings
	sf::RenderWindow& window;
	sf::View view;
	sf::ContextSettings settings;
	const float ZOOM_FACTOR = 1.5f;

	// Cursors
	sf::Cursor handCursor;
	sf::Cursor defaultCursor;

	std::string screen = "MAIN MENU";

	bool hovering = false;
	bool connectingMode = false;
	bool createConnectedObjMode = false;
	bool createChain = false;
	bool planetMode = false;
	bool enableCollison = true;
	bool borderless = false;
	bool renderButtonsMenu = true;
	bool deletedSomething = false;

	std::vector<std::pair<sf::Keyboard::Key, std::function<void()>>> keyActions;

	// Physics and simulation parameters
	float lineLength = 45;
	ObjectsList objectList;
	float deltaTime = 1.0f / 60.0f;
	float elastic = 0.0;
	int objCount = 0;
	float radius = 50;

	// Mouse and interaction state
	sf::Vector2f* previousMousePos = nullptr;
	sf::Vector2f currentMousePos;
	bool leftMouseClickFlag = false;
	bool rightMouseClickFlag = false;
	bool mouseFlagScrollUp = false;
	bool mouseFlagScrollDown = false;
	int mouseScrollPower = 5;
	bool scaleFlag = false;
	bool TouchedOnceLeftClick = false;
	bool  TouchedOnceRightClick = false;
	float moveSpeedScreen = 15.f;
	bool freeze = false;
	int typeOfLink = 1; //1 -> fixed. 2 -> non fixed. TODO: 3 -> loose/custom by user
	float textureResizer = 1.2;

	// Object pointers for interaction
	BaseShape* thisBallPointer = nullptr;
	BaseShape* previousBallPointer = nullptr;
	BaseShape* connecttableBallPointer = nullptr;
	BaseShape* previousConnecttableBallPointer = nullptr;

	// Visual settings
	sf::Color ball_color = sf::Color(238, 238, 238);
	sf::Color proton_color = sf::Color(255, 222, 33);
	sf::Color electron_color = sf::Color(106, 102, 157);
	sf::Color ball_color2 = sf::Color(50, 5, 11);
	sf::Color background_color = sf::Color(30, 30, 30);
	sf::Color buttonColor = sf::Color(55, 58, 64);
	sf::Color bb = sf::Color(44, 55, 100);
	sf::Color explosionColor = sf::Color(205, 92, 8);
	sf::Color outlineColor = sf::Color(255, 255, 255);
	sf::Color previousColor = sf::Color(0, 0, 0);
	sf::Color sideMenuColor = sf::Color(23, 23, 23, 204); //not solid color more transperent
	sf::Color startColorGradient = sf::Color(128, 0, 128);  // purple
	sf::Color endColorGradient =  sf::Color(0, 0, 255);      // blue
	sf::Color startColorSlider = sf::Color(146, 0, 146);  // darker purple
	sf::Color endColorSlider = sf::Color(66, 0, 66);  // even darker purple

	//Textures:
	sf::Texture addButtonTexture;
	sf::Texture planetButtonTexture;
	sf::Texture trashButtonTexture;
	sf::Texture connectButtonTexture;
	sf::Texture chainButtonTexture;
	sf::Texture paticleButtonTexture;
	sf::Texture explosionButtonTexture;

	// Gradient settings
	short int gradientStep = 0;
	short int gradientStepMax = 400;
	std::vector<sf::Color> gradient;

	// UI Elements
	sf::Font font;
	sf::Text ballsCountText;
	sf::Text fpsText;
	sf::Text linkingText;
	sf::RectangleShape sideMenuRec;
	std::vector<Button> buttons;

	//Sliders:
	std::vector<Slider*> slidersVec;
	Slider* gravitySlider = new Slider(300, 20, 200, 20, endColorSlider, startColorSlider, 0, 100);
	Slider* collisionSlider = new Slider(550, 20, 200, 20, sf::Color::Red, explosionColor, 0, 100);
	Slider* lineLengthSlider = new Slider(800, 20, 200, 20, sf::Color::Blue, sf::Color::Cyan, 0, 100);

	// Menu elements
	sf::RectangleShape headerText;
	std::vector<std::pair<Button, bool>> settingsButtonVec;

	// Performance tracking
	sf::Clock clock;
	sf::Clock fpsClock;
	int frameCount = 0;
	float currentFPS = 0.0f;

	// Object templates
	Circle* copyObjCir;
	RectangleClass* copyObjRec;

	// Spawn settings
	float posYStartingPoint = 200;
	int posXStartingPoint = radius;
	short int startingPointAdder = 31;
	sf::Vector2f spawnStartingPoint;
	sf::Vector2f initialVel = sf::Vector2f(0, 0);

	//Electricity:
	int particleType = 3;
	sf::Vector2f electronInitialVel = sf::Vector2f(0, 0); // TODO : fix it

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

	std::string Run() override{
		currentMousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
		handleAllEvents();
		renderSimulation();
		return screen;
	}

	void SetScreen(std::string newScreen) override{
		screen = newScreen;
	}

private:
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

		keyActions.push_back({ sf::Keyboard::Left, [&]() { view.move(-moveSpeedScreen, 0.f); } });
		keyActions.push_back({ sf::Keyboard::Right, [&]() { view.move(moveSpeedScreen, 0.f); } });
		keyActions.push_back({ sf::Keyboard::Up, [&]() { view.move(0.f, -moveSpeedScreen); } });
		keyActions.push_back({ sf::Keyboard::Down, [&]() { view.move(0.f, moveSpeedScreen); } });

	}

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

	void handleAllEvents() override {
		sf::Event event;
		while (window.pollEvent(event)) {
			handleEventsFromPollEvent(event);
		}
		handleMouseClick();
		handleScaling();
		handleMouseInteraction();
	}

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

	void handleMouseWheel(sf::Event event) override {
		if (event.type == sf::Event::MouseWheelScrolled) {
			if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
				//sf::Vector2f beforeZoom = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y), view);
				if (event.mouseWheelScroll.delta > 0) {

					if (!scaleFlag) {
						view.zoom(1.f / ZOOM_FACTOR);
					}
					mouseFlagScrollUp = true;
				}

				else if (event.mouseWheelScroll.delta < 0) {
					if (!scaleFlag) {
						view.zoom(ZOOM_FACTOR);
					}
					mouseFlagScrollDown = true;
				}

				//sf::Vector2f afterZoom = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y), view);
				//sf::Vector2f offset = beforeZoom - afterZoom;
				//view.move(offset);
			}
		}
	}

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

	void ToggleChainMode() {
		createChain = !createChain;
		previousBallPointer = thisBallPointer;
	}

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

	void Restart() {
		objectList.DeleteAll();
		deletedSomething = true;
		objCount = 0;
		planetMode = false;
		connectingMode = false;
		previousColor = sf::Color(0, 0, 0);
		initialVel = sf::Vector2f(0, 0);

		// Mouse and interaction state
		previousMousePos = nullptr;
		currentMousePos = sf::Vector2f(0,0);
		mouseFlagScrollUp = false;
		mouseFlagScrollDown = false;
		scaleFlag = false;
		TouchedOnceLeftClick = false;
		TouchedOnceRightClick = false;
		leftMouseClickFlag = false;
		rightMouseClickFlag = false;

		window.setMouseCursor(defaultCursor);
	}

	void createConnectedObjects() override {
		BaseShape* newObject = objectList.CreateNewCircle(options.gravity, gradient[gradientStep], currentMousePos, initialVel);
		objCount++;
		objectList.connectedObjects.AddObject(newObject);
		objectList.connectedObjects.ConnectRandom(10, typeOfLink);
	}

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

	void The3BodyProblem() {
		planetMode = true;
		objectList.CreateNewPlanet(7000, sf::Color(205, 28, 24), sf::Vector2f(990, 466.02540), 20, 5.9722 * pow(10, 16));
		objectList.CreateNewPlanet(7000, sf::Color(0, 71, 171), sf::Vector2f(1000 - 110, 600), 20, 5.9722 * pow(10, 16));
		objectList.CreateNewPlanet(7000, sf::Color(137, 243, 54), sf::Vector2f(1200 - 110, 600), 20, 5.9722 * pow(10, 16));
		objCount++;
	}

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

	void CreateRandomConnectedCircles() {
		BaseShape* newObject = objectList.CreateNewCircle(options.gravity, gradient[gradientStep], currentMousePos, initialVel);
		objCount += 1;
		objectList.connectedObjects.AddObject(newObject);
		objectList.connectedObjects.ConnectRandom(10, typeOfLink);
	}

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

	void handleConnecting() override {
		if (connectingMode && connecttableBallPointer != previousConnecttableBallPointer)
		{
			objectList.connectedObjects.MakeNewLink(previousConnecttableBallPointer, connecttableBallPointer, typeOfLink);
		}
	}

	void createExplosionCircles() override {
		objectList.CreateNewCircle(options.gravity, explosionColor, sf::Vector2f(currentMousePos.x + 3, currentMousePos.y + 3), initialVel);
		for (size_t i = 0; i < 50; i++) {
			objectList.CreateNewCircle(options.gravity, explosionColor, currentMousePos, initialVel);
			objCount++;
		}
	}

	void createExplosionRectangles() override {
		objectList.CreateNewRectangle(options.gravity, explosionColor, sf::Vector2f(currentMousePos.x + 3, currentMousePos.y + 3));
		for (size_t i = 0; i < 50; i++) {
			objectList.CreateNewRectangle(options.gravity, explosionColor, currentMousePos);
			objCount++;
		}
	}

	void handleSliders() {
		for (auto& slider:slidersVec)
		{
			if (slider->containMouse(currentMousePos))
			{
				slider->handleClick(window, currentMousePos);
			}
		}
		UpdateValuesSliders();
	}

	void UpdateValuesSliders() {
		options.gravity = gravitySlider->getValue();
		oldGravity = options.gravity;
		objectList.ChangeGravityForAll(options.gravity);
		objectList.ChangeLineLengthForAll(lineLengthSlider->getValue() * 5);
	}

	//visuals
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

	void initializeUI() override {
		loadResources();
		setupText();
		SetupHeaders();
		SetupSideMenu();
		SetupButtons();
		SetupSliders();
	}

	void SetupSliders() {
		slidersVec.push_back(gravitySlider);
		slidersVec.push_back(collisionSlider);
		slidersVec.push_back(lineLengthSlider);

		gravitySlider->setValue(options.gravity);
		collisionSlider->setValue(0);
		lineLengthSlider->setValue(lineLength);
	}

	void initializeCursors() override {
		if (!defaultCursor.loadFromSystem(sf::Cursor::Arrow) ||
			!handCursor.loadFromSystem(sf::Cursor::Hand)) {
			throw std::runtime_error("Failed to load cursors");
		}
		window.setMouseCursor(defaultCursor);
	}

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

	void setupGradient() override {
		gradient = GenerateGradient(gradientStepMax);
	}

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
			if (!connectButtonTexture.loadFromFile("Visuals/Buttons/ConnectButton.png")) {
				throw std::runtime_error("Failed to load texture");
			}
			if (!chainButtonTexture.loadFromFile("Visuals/Buttons/ChainButton.png")) {
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
			connectButtonTexture.setSmooth(true);
			chainButtonTexture.setSmooth(true);
			paticleButtonTexture.setSmooth(true);
			explosionButtonTexture.setSmooth(true);
		}
		catch (const std::exception& error) {
			std::cerr << "Texture loading error: " << error.what() << std::endl;
		}
	}

	void updateFPS() override {
		frameCount++;
		float timeElapsed = fpsClock.getElapsedTime().asSeconds();

		if (timeElapsed >= 1.0f) {
			currentFPS = frameCount / timeElapsed;
			frameCount = 0;
			fpsClock.restart();
		}
	}

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

	void SetupSideMenu() {
		sideMenuRec.setFillColor(sideMenuColor);
		sideMenuRec.setSize(sf::Vector2f(110, window_height));
		//sideMenuRec.setOrigin(sf::Vector2f(sideMenuRec.getSize().x / 2, sideMenuRec.getSize().y / 2));
		sideMenuRec.setPosition(window_width - 110, 0);
	}

	void SetupHeaders() override {
		headerText.setSize(sf::Vector2f(400.f, 100.f));
		headerText.setPosition(
			options.window_width / 2.f - headerText.getSize().x / 2.f,
			100.f
		);
		headerText.setFillColor(buttonColor);
	}

	void SetupButtons() {
		Button addButton = Button(85 / textureResizer, 85 / textureResizer,
			sf::Vector2f(options.window_width - 55, 60), "CIR");
		addButton.SetTexture(addButtonTexture);

		Button explosionButton = Button(85 / textureResizer, 85 / textureResizer,
			sf::Vector2f(options.window_width - 55, 190), "EXPLOSION");
		explosionButton.SetTexture(explosionButtonTexture);

		Button connectButton = Button(85 / textureResizer, 85 / textureResizer,
			sf::Vector2f(options.window_width - 55, 320), "LINK");
		connectButton.SetTexture(connectButtonTexture);

		Button chainButton = Button(85 / textureResizer, 85 / textureResizer,
			sf::Vector2f(options.window_width - 55, 450), "CHAIN");
		chainButton.SetTexture(chainButtonTexture);

		Button planetButton = Button(85 / textureResizer, 85 / textureResizer,
			sf::Vector2f(options.window_width - 55, 580), "PLANET");
		planetButton.SetTexture(planetButtonTexture);

		Button trashButton = Button(85 / textureResizer, 85 / textureResizer,
			sf::Vector2f(options.window_width - 55, 710), "RESTART");
		trashButton.SetTexture(trashButtonTexture);


		buttons.push_back(addButton);
		buttons.push_back(explosionButton);
		buttons.push_back(connectButton);
		buttons.push_back(chainButton);
		buttons.push_back(planetButton);
		buttons.push_back(trashButton);
	}

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

	void RenderSliders() {
		for (auto& slider :slidersVec)
		{
			slider->update(1 / 60);
			slider->draw(window);
		}
	}

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

	void limitFrameRate() override {
		sf::Time elapsed = clock.restart();
		if (elapsed.asSeconds() < deltaTime) {
			sf::sleep(sf::seconds(deltaTime - elapsed.asSeconds()));
		}
	}

	//TODO: I dont need it i think
	std::vector<BaseShape> ConvertForSending() override {
		return std::vector<BaseShape>();
	}
};
