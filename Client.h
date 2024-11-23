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
	}

	std::string Run() {
		currentMousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
		handleAllEvents();
		renderSimulation();
		return screen;
	}

	void TranslateMessage(const std::string& message) override {
		if (message.length() > 1 && message[0] == '$') {
			std::vector<BaseShape*> shapes = Serialization::DeserializeShapes(message.substr(1));
			//std::cout << "Received " << shapes.size() << " shapes from server " << std::endl;
			objectList.objList = shapes;
		}
		else {
			// Handle regular string messages
			std::cout << "Received message: " << message << std::endl;
			SaveMessage(message);
		}
	}

private:
	//Local:
	void handleMouseClick() override {
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !mouseClickFlag) {
			thisObjID = objectList.checkIfPointInObjectArea(currentMousePos);
			thisBallPointer = objectList.FindByID(thisObjID);

			// Store the previous object before checking for a new one
			if (previousBallID == -1) {
				previousBallID = thisObjID;
			}

			if (thisObjID != -1) {
				mouseClickFlag = true; // Set flag if circle found
				TouchedOnce = true;

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
	}

	void handleMouseRelase(sf::Event event) override {
		if (event.type == sf::Event::MouseButtonReleased) {
			int releasedObjID = objectList.checkIfPointInObjectArea(currentMousePos);

			// Only reset visual state if we're releasing the same object we initially clicked
			if (releasedObjID == thisObjID && thisObjID != -1) {
				window.setMouseCursor(defaultCursor);
				thisBallPointer->setColor(previousColor);
				thisBallPointer->SetOutline(outlineColor, 0);
			}

			mouseClickFlag = false;
			scaleFlag = false;
			TouchedOnce = false;
		}
	}

	void handleMouseInteraction() override {
		if (mouseClickFlag && thisBallPointer != nullptr) {
			// Set position of the found ball
			send_udp_message("NEWP^" + thisBallPointer->GetIDStr() + "#" + Vector2fToString(currentMousePos));

			// Handle linking objects if in connecting mode
			if (connectingMode && thisObjID != previousBallID) {
				send_udp_message("LINK&" + objectList.FindByID(previousBallID)->GetIDStr() + "&" + objectList.FindByID(thisObjID)->GetIDStr());
			}

			handleScaling();
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
					mouseFlagScroll = 1;
				}

				else if (event.mouseWheelScroll.delta < 0) {
					if (!scaleFlag) {
						view.zoom(ZOOM_FACTOR);
					}
					mouseFlagScroll = -1;
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


	//Sends to server:

	void handleKeyPress(sf::Event event) override {
		if (event.type == sf::Event::KeyPressed) {
			//Sends to server:
			if (event.key.code == sf::Keyboard::Escape) {
				screen = "MAIN MENU";
				send_tcp_message("EXIT;");
			}
			if (event.key.code == sf::Keyboard::A) {
				send_udp_message("CIR,10;");
			}
			if (event.key.code == sf::Keyboard::T) {
				send_udp_message("REC,10;");
			}
			if (event.key.code == sf::Keyboard::BackSpace && mouseClickFlag) {
				send_tcp_message("DEL^" + thisBallPointer->GetIDStr() + ";");
			}
			if (event.key.code == sf::Keyboard::L) {
				send_udp_message("PLANET*" + Vector2fToString(currentMousePos) + ";");
			}
			if (event.key.code == sf::Keyboard::F) {
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				send_udp_message("EXPLOSION:CIR*" + Vector2iToString(mousePos) + ";");
			}
			if (event.key.code == sf::Keyboard::J) {
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				send_udp_message("EXPLOSION:REC*" + Vector2iToString(mousePos) + ";");
			}
			if (event.key.code == sf::Keyboard::H) {
				send_udp_message("LINK:RND;");
			}
			if (event.key.code == sf::Keyboard::R) {
				send_udp_message("RESTART;");

			}

#pragma region Local Keys With Sending:
			if (event.key.code == sf::Keyboard::C) {
				//send_tcp_message("LINK->MODE");
				if (!connectingMode)
				{
					connectingMode = true;
					previousBallID = thisObjID;
					linkingText.setString("ACTIVATED");
					linkingText.setFillColor(sf::Color::Magenta);
				}
				else {
					connectingMode = false;
					linkingText.setString("DEACTIVATED");
					linkingText.setFillColor(sf::Color::White);
				}
			}
			if (event.key.code == sf::Keyboard::S && mouseClickFlag)
			{
				scaleFlag = true;
			}
			if (event.key.code == sf::Keyboard::Left) {
				view.move(-moveSpeedScreen, 0.f);
			}
			if (event.key.code == sf::Keyboard::Right) {
				view.move(moveSpeedScreen, 0.f);
			}
			if (event.key.code == sf::Keyboard::Up) {
				view.move(0.f, -moveSpeedScreen);
			}
			if (event.key.code == sf::Keyboard::Down) {
				view.move(0.f, moveSpeedScreen);
			}
			if (event.key.code == sf::Keyboard::F11)
			{
				if (!options.fullscreen)
				{
					window.create(options.desktopSize, "Fullscreen Mode", sf::Style::Fullscreen);
					options.fullscreen = true;
				}
				else {
					window.create(sf::VideoMode(1920, 980), "Tomy Mode", sf::Style::Default);
					options.fullscreen = false;
				}
			}
#pragma endregion
		}
	}

	void handleScaling() override {
		if (scaleFlag && (mouseFlagScroll == 1 || mouseFlagScroll == -1)) {
			if (Circle* circle = dynamic_cast<Circle*>(thisBallPointer)) {
				send_udp_message("SCALE@CIR@" + circle->GetIDStr() + "@" + std::to_string(mouseScrollPower) + "@" + std::to_string(mouseFlagScroll));
			}
			else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(thisBallPointer))
			{
				send_udp_message("SCALE@REC@" + rectangle->GetIDStr() + "@" + std::to_string(mouseScrollPower) +"@" + std::to_string(mouseFlagScroll));
			}
			mouseFlagScroll = 0;
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

#pragma region EssantialVariables
	sf::VideoMode desktopSize = sf::VideoMode::getDesktopMode();
	int window_height = desktopSize.height;
	int window_width = desktopSize.width;
	bool fullscreen = false;


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
	bool planetMode = false;
	bool enableCollison = true;
	bool borderless = true;

	// Physics and simulation parameters
	float lineLength = 150;
	ObjectsList objectList;
	float deltaTime = 1.0f / 60.0f;
	float elastic = 0.0;
	int objCount = 0;
	float radius = 50;

	// Mouse and interaction state
	sf::Vector2f currentMousePos;
	bool mouseClickFlag = false;
	int mouseFlagScroll = 0;
	int mouseScrollPower = 5;
	bool scaleFlag = false;
	bool TouchedOnce = false;
	float moveSpeedScreen = 15.f;

	// Object pointers for interaction
	BaseShape* thisBallPointer = nullptr;
	int previousBallID = -1;
	int thisObjID = -1;

	// Visual settings

	sf::Color background_color = sf::Color(30, 30, 30);
	sf::Color buttonColor = sf::Color(55, 58, 64);
	sf::Color outlineColor = sf::Color(255, 255, 255);
	sf::Color previousColor = sf::Color(0, 0, 0);

	// Gradient settings
	short int gradientStep = 0;
	short int gradientStepMax = 400;

	// UI Elements
	sf::Font font;
	sf::Text ballsCountText;
	sf::Text fpsText;
	sf::Text linkingText;

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
#pragma endregion

};




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
		else if (screen == "FULLSCREEN") {
			toggleFullscreen(window);
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