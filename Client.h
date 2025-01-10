#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <string>
#include <deque>
#include <mutex>
#include <SFML/Graphics.hpp>;
#include <SFML/Window.hpp>
#include "BaseShape.h"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "PhysicsSimulation.h"
#include "Options.h"
#include "UI.h"
#include "HandleNetworkingClient.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

int BaseShape::objectCount = 0;

class Client : public HandleNetworkingClient, public PhysicsSimulationActions, public PhysicsSimulationVisual {
public:
	Client(sf::RenderWindow& window, boost::asio::io_context& io_context, const std::string& host, unsigned short tcp_port, unsigned short udp_port) :
		window(window),
		settings(8),
		objectList(lineLength),
		PhysicsSimulationVisual(),
		PhysicsSimulationActions(),
		HandleNetworkingClient(io_context, host, tcp_port, udp_port)
	{
		view = window.getDefaultView();  // Initialize view from window
		initializeCursors();
		loadResources();
		initializeUI();
		setupGradient();
		InitializeKeyActions();
	}

	std::string Run() {
		currentMousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
		handleAllEvents();
		renderSimulation();
		return screen;
	}

	void TranslateMessage(const std::string& message) override {
		// Check if the message contains serialized shapes
		if (message.length() > 1 && message[0] == '$') {
			try {
				std::vector<BaseShape*> shapes = Serialization::DeserializeShapes(message.substr(1));

				// Update the object list with the received shapes
				objectList.objList = shapes;

			}
			catch (const std::exception& e) {
				std::cerr << "Error deserializing shapes: " << e.what() << std::endl;
			}
		}
		// Handle regular string messages
		else if (message.starts_with("broadcast:")) {
			std::string broadcastMessage = message.substr(10); // Remove "broadcast:" prefix
			/*std::cout << "[Broadcast] " << broadcastMessage << std::endl;*/
		}
		else if (message.starts_with("client:")) {
			std::string clientMessage = message.substr(7); // Remove "client:" prefix
			//std::cout << "[Client Message] " << clientMessage << std::endl;
			SaveMessage(clientMessage);
		}
		else {
			// General case for unknown or regular string messages
			std::cout << "Received message: " << message << std::endl;
			SaveMessage(message);
		}
	}

private:
#pragma region EssantialVariables
	sf::VideoMode desktopSize = sf::VideoMode::getDesktopMode();
	int window_height = desktopSize.height;
	int window_width = desktopSize.width;
	bool fullscreen = false;
	int oldGravity = options.gravity;
	double massLock = 0;
	// Window and view settings
	sf::RenderWindow& window;
	sf::View view;
	sf::ContextSettings settings;
	const float ZOOM_FACTOR = 1.5f;
	// Cursors
	sf::Cursor handCursor;
	sf::Cursor defaultCursor;
	std::string screen = "START";
	bool hovering = false;
	bool connectingMode = false;
	bool createConnectedObjMode = false;
	bool createChain = false;
	bool planetMode = false;
	bool enableCollison = false;
	bool borderless = true;
	bool renderButtonsMenu = true;
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
	int mouseFlagScroll = 0;
	bool mouseFlagScrollUp = false;
	bool mouseFlagScrollDown = false;
	int mouseScrollPower = 5;
	bool scaleFlag = false;
	bool TouchedOnceLeftClick = false;
	bool TouchedOnceRightClick = false;
	float moveSpeedScreen = 15.f;
	bool freeze = false;
	int typeOfLink = 1;
	float textureResizer = 1.2;
	// Object pointers for interaction
	BaseShape* thisBallPointer = nullptr;
	int previousBallID = -1;
	int thisObjID = -1;
	BaseShape* connecttableBallPointer = nullptr;
	int connecttableObjID = -1;
	int previousConnecttableObjID = -1;
	// Visual settings
	sf::Color ball_color = sf::Color(238, 238, 238);
	sf::Color proton_color = sf::Color(255, 222, 33);
	sf::Color electron_color = sf::Color(106, 102, 157);
	sf::Color ball_color2 = sf::Color(50, 5, 11);
	sf::Color background_color = sf::Color(30, 30, 30);
	sf::Color buttonColor = sf::Color(55, 58, 64);
	sf::Color bb = sf::Color(44, 55, 100);
	sf::Color explosion = sf::Color(205, 92, 8);
	sf::Color outlineColor = sf::Color(255, 255, 255);
	sf::Color previousColor = sf::Color(0, 0, 0);
	sf::Color sideMenuColor = sf::Color(23, 23, 23, 204);
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
	sf::Vector2f electronInitialVel = sf::Vector2f(0, 0);
#pragma endregion



	//Local:
	void handleMouseClick() override {
		//Left click:
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !leftMouseClickFlag) {
			thisObjID = objectList.checkIfPointInObjectArea(currentMousePos);
			thisBallPointer = objectList.FindByID(thisObjID);

			// Store the previous object before checking for a new one
			if (previousBallID == -1) {
				previousBallID = thisObjID;
			}

			if (thisObjID != -1) {
				leftMouseClickFlag = true; // Set flag if circle found
				TouchedOnceLeftClick = true;

				window.setMouseCursor(handCursor);
				thisBallPointer->SetOutline(outlineColor, 5);
				// Store the current color
				previousColor = thisBallPointer->GetColor();
				// Darken the color
				sf::Color currentColor = previousColor;
				currentColor.r = std::max(0, currentColor.r - 15);
				currentColor.g = std::max(0, currentColor.g - 15);
				currentColor.b = std::max(0, currentColor.b - 15);
				thisBallPointer->setColor(currentColor);
			}
		}

		//Right click:
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && !rightMouseClickFlag) {
			connecttableObjID = objectList.checkIfPointInObjectArea(currentMousePos); // Check if an object is within radius and get its ID
			connecttableBallPointer = objectList.FindByID(connecttableObjID); // transfer from id to pointer
			if (connecttableBallPointer != nullptr) { // Check if a circle was found
				if (previousConnecttableObjID == -1)
				{
					previousConnecttableObjID = connecttableBallPointer->GetID();
				}
				rightMouseClickFlag = true; // Set flag if circle found
				window.setMouseCursor(handCursor);
			}
		}
	}

	void handleMouseRelase(sf::Event event) override {
		//Left mouse button:
		if (event.type == sf::Event::MouseButtonReleased) {
			int releasedObjID = objectList.checkIfPointInObjectArea(currentMousePos);

			// Only reset visual state if we're releasing the same object we initially clicked
			if (releasedObjID == thisObjID && thisObjID != -1) {
				window.setMouseCursor(defaultCursor);
				thisBallPointer->setColor(previousColor);
				thisBallPointer->SetOutline(outlineColor, 0);
			}

			leftMouseClickFlag = false;
			scaleFlag = false;
			TouchedOnceLeftClick = false;
		}

		//Right mouse button:
		if (event.mouseButton.button == sf::Mouse::Right) {
			window.setMouseCursor(defaultCursor);
			rightMouseClickFlag = false;
			TouchedOnceRightClick = false;
		}
	}

	void handleMouseInteraction() override {
		if (leftMouseClickFlag && thisBallPointer != nullptr) {
			// Set position of the found ball
			send_udp_message("NEWP^" + thisBallPointer->GetIDStr() + "#" + Vector2fToString(currentMousePos));

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

	void toggleConnectingMode() override {
		connectingMode = !connectingMode;
		linkingText.setString(connectingMode ? "ACTIVATED" : "DEACTIVATED");
		linkingText.setFillColor(connectingMode ? sf::Color::Magenta : sf::Color::White);
	}

	void handleEventsFromPollEvent(sf::Event event) override {
		if (event.type == sf::Event::Closed) { window.close(); } // need to do that it will close the connection
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

	void InitializeKeyActions() {
		// ** General Actions **
		keyActions.push_back({ sf::Keyboard::Escape, [&]() {
			screen = "MAIN MENU";
			objectList.DeleteAll();
			objCount = 0;
			disconnect_from_server();
		} });

		keyActions.push_back({ sf::Keyboard::BackSpace, [&]() {
			if (leftMouseClickFlag) {
				objectList.DeleteThisObj(thisBallPointer);
				send_tcp_message("DEL^" + thisBallPointer->GetIDStr() + ";");
			}
		} });

		keyActions.push_back({ sf::Keyboard::Space, [&]() {
			send_tcp_message("TOGGLE_FREEZE;");
		} });

		keyActions.push_back({ sf::Keyboard::R, [&]() {
			send_udp_message("RESTART;");
		} });

		// ** Object Creation **
		keyActions.push_back({ sf::Keyboard::A, [&]() {
			send_udp_message("CIR,10;"); // Add circles
		} });

		keyActions.push_back({ sf::Keyboard::T, [&]() {
			send_udp_message("REC,10;"); // Add rectangles
		} });

		keyActions.push_back({ sf::Keyboard::L, [&]() {
			createPlanet();
			send_udp_message("PLANET*" + Vector2fToString(currentMousePos) + ";");
		} });

		keyActions.push_back({ sf::Keyboard::Q, [&]() {
			send_udp_message("FIXEDCIRCLE*" + Vector2fToString(currentMousePos) + ";");
		} });

		// ** Particle Actions **
		keyActions.push_back({ sf::Keyboard::E, [&]() {
			send_udp_message("PARTICLE:" + std::to_string(particleType) + "*" + Vector2fToString(currentMousePos) + ";");
		} });

		keyActions.push_back({ sf::Keyboard::F, [&]() {
			send_udp_message("EXPLOSION:CIR*" + Vector2fToString(currentMousePos) + ";");
		} });

		keyActions.push_back({ sf::Keyboard::J, [&]() {
			send_udp_message("EXPLOSION:REC*" + Vector2fToString(currentMousePos) + ";");
		} });

		// ** Linking and Simulation **
		keyActions.push_back({ sf::Keyboard::H, [&]() {
			send_udp_message("LINK:RND;");
		} });

		keyActions.push_back({ sf::Keyboard::Num0, [&]() {
			send_udp_message("3BODYPROBLEM;");
		} });


		// ** Toggles **
		keyActions.push_back({ sf::Keyboard::Num6, [&]() {
			enableCollison = !enableCollison;
			send_udp_message("COLLISION!" + std::to_string(enableCollison) + ";");
		} });

		keyActions.push_back({ sf::Keyboard::Num7, [&]() {
			borderless = !borderless;
			send_udp_message("BORDER!" + std::to_string(borderless) + ";");
		} });

		keyActions.push_back({ sf::Keyboard::S, [&]() {
			if (leftMouseClickFlag) {
				scaleFlag = !scaleFlag;
				send_tcp_message("SCALE;");
			}
		} });


		//Local events:
		keyActions.push_back({ sf::Keyboard::X, [&]() {
			ToggleChainMode();
		} });

		keyActions.push_back({ sf::Keyboard::K, [&]() {
			createConnectedObjMode = !createConnectedObjMode;
			previousBallID = thisBallPointer->GetID();
		} });

		// ** Particle Type Switches **
		keyActions.push_back({ sf::Keyboard::Num1, [&]() { typeOfLink = 1; } });
		keyActions.push_back({ sf::Keyboard::Num2, [&]() { typeOfLink = 2; } });
		keyActions.push_back({ sf::Keyboard::Num3, [&]() { particleType = 3; } });
		keyActions.push_back({ sf::Keyboard::Num4, [&]() { particleType = 4; } });
		keyActions.push_back({ sf::Keyboard::Num5, [&]() { particleType = 5; } });

		// ** View Movement **
		keyActions.push_back({ sf::Keyboard::Left, [&]() { view.move(-moveSpeedScreen, 0.f); } });
		keyActions.push_back({ sf::Keyboard::Right, [&]() { view.move(moveSpeedScreen, 0.f); } });
		keyActions.push_back({ sf::Keyboard::Up, [&]() { view.move(0.f, -moveSpeedScreen); } });
		keyActions.push_back({ sf::Keyboard::Down, [&]() { view.move(0.f, moveSpeedScreen); } });

		// ** UI and Fullscreen **
		keyActions.push_back({ sf::Keyboard::F11, [&]() {
			toggleFullscreen(window);
		} });
	}

	void ToggleChainMode() {
		createChain = !createChain;
		previousBallID = thisBallPointer->GetID();
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



	void handleScaling() override {
		if (scaleFlag && (mouseFlagScroll == 1 || mouseFlagScroll == -1)) {
			if (Circle* circle = dynamic_cast<Circle*>(thisBallPointer)) {
				send_udp_message("SCALE@CIR@" + circle->GetIDStr() + "@" + std::to_string(mouseScrollPower) + "@" + std::to_string(mouseFlagScroll));
			}
			else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(thisBallPointer))
			{
				send_udp_message("SCALE@REC@" + rectangle->GetIDStr() + "@" + std::to_string(mouseScrollPower) + "@" + std::to_string(mouseFlagScroll));
			}
			mouseFlagScroll = 0;
		}
	}

	void handleConnecting() override {
		if (connectingMode && connecttableBallPointer->GetID() != previousConnecttableObjID)
		{
			std::cout << objectList.FindByID(previousConnecttableObjID)->GetIDStr() << "\n";
			std::cout << objectList.FindByID(connecttableObjID)->GetIDStr() ;
			send_tcp_message("LINK&" + objectList.FindByID(previousConnecttableObjID)->GetIDStr() + "&" + objectList.FindByID(connecttableObjID)->GetIDStr());
		}
	}

#pragma region Convertors
	std::string Vector2iToString(const sf::Vector2i& vector) {
		std::ostringstream oss;  // create a string stream
		oss << "(" << vector.x << ", " << vector.y << ")";  // format the vector as a string
		return oss.str();  // return the formatted string
	}

	std::string Vector2fToString(const sf::Vector2f& vector) {
		std::ostringstream oss;  // create a string stream
		oss << "(" << vector.x << ", " << vector.y << ")";  // format the vector as a string
		return oss.str();  // return the formatted string
	}
#pragma endregion 

#pragma region Viusals
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

	void renderSimulation() override {
		updateFPS();
		window.clear(background_color);
		window.setView(view);

		MoveAndDrawObjects();

		window.setView(window.getDefaultView());
		renderTexts();

		window.display();

		limitFrameRate();

		//for (auto obj : objectList.objList)
		//{
		//	//std::cout << "\033[34m" << obj->GetPositionStr() << "\n";
		//}
		////std::cout << "\033[0m";
	}

	void MoveAndDrawObjects() override {
		objectList.DrawObjects(window, currentFPS, planetMode);
	}

	void initializeUI() override {
		setupText();
		setupHeaders();
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
		loadTextures();
	}

	void loadTextures() override {
		// Since no textures are used in the original code, this method is kept minimal
		// but provides a hook for future texture loading if needed
		try {
			// Currently no textures are loaded as the simulation uses basic shapes
			// If textures are needed in the future, they can be loaded here:
			/*
			sf::Texture texture;
			if (!texture.loadFromFile("path/to/texture.png")) {
				throw std::runtime_error("Failed to load texture");
			}
			*/
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

	void setupHeaders() override {
		headerText.setSize(sf::Vector2f(400.f, 100.f));
		headerText.setPosition(
			options.window_width / 2.f - headerText.getSize().x / 2.f,
			100.f
		);
		headerText.setFillColor(buttonColor);
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

	std::vector<BaseShape> ConvertForSending() override {
		return std::vector<BaseShape>();
	}
#pragma endregion
};




void initializeWindow(sf::RenderWindow& window, sf::View view, sf::ContextSettings settings) {
	window.create(
		sf::VideoMode(options.window_width, options.window_height), "TomySim", sf::Style::Default, settings);
	view = window.getDefaultView();
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);
}

void Run(std::string& screen, Client& simulation, MainMenu& mainMenu, Settings& settingsClass, sf::RenderWindow& window) {
	while (window.isOpen()) {
		if (screen == "START") {
			screen = simulation.Run();
		}
		else if (screen == "MAIN MENU") {
			screen = mainMenu.handleMainMenu();
		}
		else if (screen == "SETTINGS") {
			screen = settingsClass.handleSettings();
		}
		else {
			window.close();
			break;  //  break to exit the loop when closing
		}
	}
}


int main() {
	try {
		//NETWORKING:
		const std::string server_ip = "127.0.0.1";  // or "localhost"
		unsigned short tcp_port = 8080;
		unsigned short udp_port = 8081;

		std::cout << "Starting client..." << std::endl;
		std::cout << "Attempting to connect to:" << std::endl;
		std::cout << "Server IP: " << server_ip << std::endl;
		std::cout << "TCP port: " << tcp_port << std::endl;
		std::cout << "UDP port: " << udp_port << std::endl;

		boost::asio::io_context io_context;


		//ENGINE:
		sf::RenderWindow window;
		sf::ContextSettings settings;
		settings.antialiasingLevel = 8;
		sf::View view = window.getDefaultView();
		initializeWindow(window, view, settings);

		MainMenu mainMenu(window);
		Settings settingsClass(window);
		std::string screen = "START";

		Client client(window, io_context, server_ip, tcp_port, udp_port);
		client.connect();

		// start a thread to run the IO service
		std::thread io_thread([&io_context]() {
			io_context.run();
			});

		std::cout << "\nAvailable commands:" << std::endl;
		std::cout << "- Type a message to send via TCP" << std::endl;
		std::cout << "- Type 'udp:' followed by a message to send via UDP" << std::endl;
		std::cout << "- Type 'stop' to stop connection attempts" << std::endl;
		std::cout << "- Type 'connect' to start connection attempts" << std::endl;
		std::cout << "- Type 'quit' to exit" << std::endl;

		Run(screen, client, mainMenu, settingsClass, window);


		// Main loop to get user input and send messages
		std::string input;
		while (std::getline(std::cin, input)) {

			if (input == "quit") {
				std::cout << "Shutting down client..." << std::endl;
				client.stop_connecting();
				break;
			}
			if (input == "stop") {
				client.stop_connecting();
				std::cout << "Stopped connection attempts." << std::endl;
				continue;
			}
			if (input == "connect") {
				std::cout << "Starting connection attempts..." << std::endl;
				client.connect();
				continue;
			}

			if (input.substr(0, 4) == "udp:") {
				client.send_udp_message(input.substr(4));
			}
			else {
				client.send_tcp_message(input);
			}
		}
		io_context.stop();
		io_thread.join();
	}
	catch (std::exception& e) {
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}