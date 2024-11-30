#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <deque>
#include <mutex>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <set>
#include <functional>
//#include "../PhysicSSimulator/Options.h"
//#include "../PhysicSSimulator/Options.cpp"
#include "../PhysicSSimulator/BaseShape.h"
#include "../PhysicSSimulator/Circle.h"
#include "../PhysicSSimulator/Serialization.h"
#include "../PhysicSSimulator/PhysicsSimulation.h"


using boost::asio::ip::tcp;
using boost::asio::ip::udp;

struct eptions {
	sf::VideoMode desktopSize = sf::VideoMode::getDesktopMode();
	int window_height = desktopSize.height;
	int window_width = desktopSize.width;
	bool fullscreen = false;
	float gravity = 0;
	double massLock = 0;
};

eptions poptions;

int BaseShape::objectCount = 0;

class ServerNetworking {
public:
	ServerNetworking(boost::asio::io_context& io_context, unsigned short tcpPort, unsigned short udpPort)
		: tcpAcceptor(io_context, tcp::endpoint(tcp::v4(), tcpPort)),
		udpSocket(io_context, udp::endpoint(udp::v4(), udpPort)),
		tcpPort(tcpPort),
		udpPort(udpPort) {
		std::cout << "Server started on TCP port " << tcpPort << " and UDP port " << udpPort << std::endl;
	}

	void Start() {
		AcceptTCPConnection();
		ReceiveUDP();
		StartConsoleInput();
	}

protected:
	class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
	public:
		tcp::socket& Socket() { return socket_; }
		const std::string& Address() const { return address_; }
		unsigned short Port() const { return port_; }
		int ID() const { return client_id_; }

		TcpConnection(tcp::socket socket, ServerNetworking& server)
			: socket_(std::move(socket)),
			server_(server),
			client_id_(++nextID) {
			address_ = socket_.remote_endpoint().address().to_string();
			port_ = socket_.remote_endpoint().port();
		}

		void start() {
			std::cout << "Client " << client_id_ << " connected from " << address_ << ":" << port_ << std::endl;
			read_message();
		}

		void send_message(const std::string& message) {
			bool write_in_progress = !message_queue_.empty();
			message_queue_.push_back(message + "\n");
			if (!write_in_progress) {
				do_write();
			}
		}

		void send_shapes(const std::vector<BaseShape*>& shapes) {
			std::string serializedShapes = Serialization::SerializeShapes(shapes);
			send_message(serializedShapes);
		}

	protected:
		void read_message() {
			auto self(shared_from_this());
			boost::asio::async_read_until(
				socket_,
				buffer_,
				'\n',
				[this, self](boost::system::error_code ec, std::size_t length) {
					if (!ec) {
						std::string message(boost::asio::buffers_begin(buffer_.data()),
							boost::asio::buffers_begin(buffer_.data()) + length);
						buffer_.consume(length);

						// Store the message (thread-safe)
						{
							std::lock_guard<std::mutex> lock(server_.storedMessagesMutex);
							server_.storedMessages.push_back(message);
						}

						// Check if the message is a serialized vector of BaseShape objects
						if (message.length() > 1 && message[0] == '$') {
							std::vector<BaseShape*> shapes = Serialization::DeserializeShapes(message.substr(1));
						}
						else {
							// Handle regular string message
							//std::cout << "Received message from client " << client_id << ": " << message << std::endl;
							server_.HandleClientMessage(message);
						}

						read_message();
					}
					else {
						server_.HandleClientDisconnect(client_id_);
					}
				});
		}

		void do_write() {
			auto self(shared_from_this());
			boost::asio::async_write(
				socket_,
				boost::asio::buffer(message_queue_.front()),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
					if (!ec) {
						message_queue_.pop_front();
						if (!message_queue_.empty()) {
							do_write();
						}
					}
					else {
						server_.HandleClientDisconnect(client_id_);
					}
				});
		}

		tcp::socket socket_;
		ServerNetworking& server_;
		boost::asio::streambuf buffer_;
		std::deque<std::string> message_queue_;
		std::string address_;
		unsigned short port_;
		int client_id_;
		static int nextID;
	};

	tcp::acceptor tcpAcceptor;
	udp::socket udpSocket;
	udp::endpoint udpSenderEndpoint;
	enum { max_length = 1024 };
	char udpData[max_length];
	unsigned short tcpPort;
	unsigned short udpPort;
	std::map<int, std::shared_ptr<TcpConnection>> tcpConnections;
	std::vector<std::string> storedMessages; // Vector to store messages
	std::mutex storedMessagesMutex; // Mutex for thread-safe access to storedMessages

	void AcceptTCPConnection() {
		tcpAcceptor.async_accept(
			[this](boost::system::error_code ec, tcp::socket socket) {
				if (!ec) {
					auto connection = std::make_shared<TcpConnection>(std::move(socket), *this);
					tcpConnections[connection->ID()] = connection;
					connection->start();
				}
				AcceptTCPConnection();
			});
	}

	void HandleClientDisconnect(int client_id) {
		auto it = tcpConnections.find(client_id);
		if (it != tcpConnections.end()) {
			std::cout << "Client " << client_id << " disconnected from "
				<< it->second->Address() << ":" << it->second->Port() << std::endl;
			tcpConnections.erase(it);
		}
	}

	void HandleClientMessage(const std::string& message) {
		// Store the message (thread-safe)
		{
			std::lock_guard<std::mutex> lock(storedMessagesMutex);
			storedMessages.push_back(message);
			//std::cout << "Received message from client " << client_id << ": " << message << std::endl;
		}
	}
	std::vector<std::string> GetAllStoredMessages() {
		std::lock_guard<std::mutex> lock(storedMessagesMutex);
		return storedMessages;
	}

	// Helper method to clear stored messages
	void ClearStoredMessages() {
		std::lock_guard<std::mutex> lock(storedMessagesMutex);
		storedMessages.clear();
	}

	void BroadcastTcpMessage(const std::string& message) {
		for (auto& conn : tcpConnections) {
			conn.second->send_message(message);
		}
		std::cout << "Broadcasted to " << tcpConnections.size() << " clients: " << message << std::endl;
	}

	void BroadcastShapes(const std::vector<BaseShape*>& shapes) {
		std::string serializedShapes = Serialization::SerializeShapes(shapes);
		for (auto& conn : tcpConnections) {
			conn.second->send_shapes(shapes);
		}
		//std::cout << "Broadcasted " << shapes.size() << " shapes to " << tcpConnections.size() << " clients" << std::endl;
	}

	void SendToClient(int client_id, const std::string& message) {
		auto it = tcpConnections.find(client_id);
		if (it != tcpConnections.end()) {
			it->second->send_message(message);
			//std::cout << "Sent to client " << client_id << ": " << message << std::endl;
		}
		else {
			std::cout << "Client " << client_id << " not found" << std::endl;
		}
	}

	void StartConsoleInput() {
		std::thread input_thread([this]() {
			std::string input;
			while (std::getline(std::cin, input)) {
				if (input.substr(0, 2) == "b:") {
					// Broadcast message
					BroadcastTcpMessage(input.substr(2));
				}
				else if (input.substr(0, 2) == "s:") {
					// Broadcast shapes
					BaseShape* cir = new Circle(3.0, sf::Color(1, 2, 3), sf::Vector2f(69, 96), 9.8, 300.0, sf::Vector2f(4, 0), 0);
					BaseShape* cir2 = new Circle(3.30, sf::Color(13, 2, 33), sf::Vector2f(369, 96), 9.8, 300.0, sf::Vector2f(43, 0), 30);
					BaseShape* cir3 = new Circle(3.03, sf::Color(1, 333, 3), sf::Vector2f(6933, 963), 93.8, 3003.0, sf::Vector2f(34, 0), 3330);
					std::vector<BaseShape*> shapes = { cir,cir2,cir3 };
					BroadcastShapes(shapes);
				}
				else if (input.substr(0, 2) == "c:") {
					// Send to specific client
					size_t space_pos = input.find(' ', 2);
					if (space_pos != std::string::npos) {
						int client_id = std::stoi(input.substr(2, space_pos - 2));
						SendToClient(client_id, input.substr(space_pos + 1));
					}
				}
				else if (input == "list") {
					// List all connected clients
					std::cout << "Connected clients:" << std::endl;
					for (const auto& conn : tcpConnections) {
						std::cout << "Client " << conn.first << " - "
							<< conn.second->Address() << ":" << conn.second->Port() << std::endl;
					}
				}
			}
			});
		input_thread.detach();
	}

	// Thread-safe method to access stored messages
	std::vector<std::string> GetStoredMessages() {
		std::lock_guard<std::mutex> lock(storedMessagesMutex);
		return storedMessages; // Return a copy
	}

	void ReceiveUDP() {
		udpSocket.async_receive_from(
			boost::asio::buffer(udpData, max_length), udpSenderEndpoint,
			[this](boost::system::error_code ec, std::size_t bytes_recvd) {
				if (!ec) {
					// Convert received data to string
					std::string message(udpData, bytes_recvd);

					HandleClientMessage(message);

					// Log the message
					/*std::cout << "UDP received from "
						<< udpSenderEndpoint.address().to_string()
						<< ": " << message
						<< std::endl;*/

						// Echo the message back to the sender
					udpSocket.async_send_to(
						boost::asio::buffer(udpData, bytes_recvd),
						udpSenderEndpoint,
						[](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {});
				}
				// Continue receiving messages
				ReceiveUDP();
			});
	}
};

int ServerNetworking::TcpConnection::nextID = 0;

class Server : public ServerNetworking, public PhysicsSimulationActions {
private:
#pragma region EssantialVariables
	sf::VideoMode desktopSize = sf::VideoMode::getDesktopMode();
	int window_height = desktopSize.height;
	int window_width = desktopSize.width;
	bool fullscreen = false;
	float gravity = 0;
	double massLock = 0;


	// Window and view settings
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
	bool enableCollison = false;
	bool borderless = true;

	// Physics and simulation parameters
	float lineLength = 150;
	ObjectsList objectList;
	float deltaTime = 1.0f / 60.0f;
	float elastic = 0.0;
	int objCount = 0;
	float radius = 50;

	// Visual settings
	sf::Color ball_color = sf::Color(238, 238, 238);
	sf::Color ball_color2 = sf::Color(50, 5, 11);
	sf::Color background_color = sf::Color(30, 30, 30);
	sf::Color buttonColor = sf::Color(55, 58, 64);
	sf::Color bb = sf::Color(44, 55, 100);
	sf::Color explosion = sf::Color(205, 92, 8);
	sf::Color outlineColor = sf::Color(255, 255, 255);
	sf::Color previousColor = sf::Color(0, 0, 0);

	// Gradient settings
	short int gradientStep = 0;
	short int gradientStepMax = 400;
	std::vector<sf::Color> gradient;

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

	// Spawn settings
	float posYStartingPoint = 200;
	int posXStartingPoint = radius;
	short int startingPointAdder = 31;
	sf::Vector2f spawnStartingPoint;
	sf::Vector2f initialVel = sf::Vector2f(4, 0);

	std::mutex objectListMutex;
#pragma endregion

public:
	Server(boost::asio::io_context& io_context, unsigned short tcpPort, unsigned short udpPort)
		: ServerNetworking(io_context, tcpPort, udpPort), PhysicsSimulationActions(),
		settings(8),
		objectList(lineLength),
		spawnStartingPoint(posXStartingPoint, posYStartingPoint)
	{
		setupGradient();
	}

	void Run() {
		while (true) {
			// Get events by const reference to avoid copying
			const auto& events = GetStoredMessages();

			// Process events
			TranslateEvents(events);

			// Clear the stored messages after processing
			{
				std::lock_guard<std::mutex> lock(storedMessagesMutex);
				storedMessages.clear();
			}

			// Update physics
			objectList.MoveObjects(window_width, window_height, currentFPS,
				elastic, planetMode, enableCollison, borderless);

			//for (auto obj :objectList.objList)
			//{
			//	/*std::cout << "\033[34m" << "amogus = " << obj->GetID();*/
			//}
			//std::cout << "\033[0m";
			// Broadcast using const reference
			const auto& shapes = objectList.objList;
			BroadcastShapes(shapes);

			std::this_thread::sleep_for(std::chrono::milliseconds(16));
		}
	}

	// Splits a string by a delimiter
	static std::vector<std::string> SplitString(const std::string& str, char delimiter) {
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(str);
		while (std::getline(tokenStream, token, delimiter)) {
			tokens.push_back(token);
		}
		return tokens;
	}

	sf::Vector2f StringToVector2f(std::string str) {
		str = str.substr(1, str.length() - 2);
		auto splitedStr = SplitString(str, ',');
		return sf::Vector2f(std::stof(splitedStr[0]), std::stof(splitedStr[1]));
	}

	sf::Vector2i StringToVector2i(std::string str) {
		str = str.substr(1, str.length() - 2);
		auto splitedStr = SplitString(str, ',');
		return sf::Vector2i(std::stoi(splitedStr[0]), std::stoi(splitedStr[1]));
	}

	std::vector<sf::Color> GenerateGradient(sf::Color startColor, sf::Color endColor, int steps) {
		std::vector<sf::Color> newGradient;
		float stepR = (endColor.r - startColor.r) / static_cast<float>(steps - 1);
		float stepG = (endColor.g - startColor.g) / static_cast<float>(steps - 1);
		float stepB = (endColor.b - startColor.b) / static_cast<float>(steps - 1);

		for (int i = 0; i < steps; ++i) {
			newGradient.push_back(sf::Color(
				startColor.r + stepR * i,
				startColor.g + stepG * i,
				startColor.b + stepB * i
			));
		}
		return newGradient;
	}

	void setupGradient() {
		sf::Color startColor(128, 0, 128);  // purple
		sf::Color endColor(0, 0, 255);      // blue
		gradient = GenerateGradient(startColor, endColor, gradientStepMax);
	}

	// Modify GetStoredMessages to return const reference
	const std::vector<std::string>& GetStoredMessages() {
		std::lock_guard<std::mutex> lock(storedMessagesMutex);
		return storedMessages; // Return reference to stored messages
	}

	//Signs Meaning:
	// ; end of message part
	// , type , number -> meaning create object of type num times
	// : action : identifier -> meaning do something who has different varients like link random/in some structure
	// * type * vector -> for spawning in position something or giving velocity.
	// & action & vector/objects & vector/objects -> do some action for two vectors or objects. like linking
	// ^ action ^ ID -> do an action specific object id
	// # ID # vetor -> manipulate vector on specific object id
	// @ event @ obj_type @ ID @ number
	void TranslateEvents(std::vector<std::string> events) {
		for (const auto& event : events) {
			auto tokens = SplitString(event, ';');
			if (!tokens.empty()) {
				for (const auto& token : tokens) {
					if (token.find(',') != std::string::npos) {
						auto splitToken = SplitString(token, ','); // Only split what we need
						SpawnObjectsNonSpecific(splitToken);
					}
					if (token.find(':') != std::string::npos) {
						auto eventAndIdentifier = SplitString(token, ':');
						if (token.find('*') != std::string::npos)
						{
							auto actionAndPos = SplitString(eventAndIdentifier[1], '*');
							SpawnObjectsIdentifierAndSpecific(eventAndIdentifier[0], actionAndPos);
						}
						else
						{
							SpawnObjectsIdentifier(eventAndIdentifier);
						}
					}
					if (token.find('@') != std::string::npos)
					{
						auto splitedToken = SplitString(token, '@');
						EventWithIdentifierOnID(splitedToken);
					}
					if (token.find('*') != std::string::npos) {
						auto splitToken = SplitString(token, '*');
						SpawnObjectsSpecific(splitToken);
					}
					if (token.find('&') != std::string::npos) {
						auto splitToken = SplitString(token, '&');
						EventBetweenTwo(splitToken);
					}
					if (token.find('^') != std::string::npos) {
						auto actionAndID = SplitString(token, '^');
						if (token.find('#') != std::string::npos)
						{
							auto idAndVec = SplitString(actionAndID[1], '#');
							VectorEventOnID(actionAndID[0], idAndVec);
						}
						else {
							EventAndID(actionAndID);
						}
					}
					else if (event == "EXIT") {
						// TODO: make disconnect from client
					}
				}
			}
		}

	}

	// SIGN ,
	void SpawnObjectsNonSpecific(std::vector<std::string> EventAndNum) {
		std::string event = EventAndNum[0];
		int num = std::stoi(EventAndNum[1]);
		if (event == "CIR") {
			for (size_t i = 0; i < num; i++)
			{
				BaseShape* newObject = objectList.CreateNewCircle(poptions.gravity, gradient[gradientStep], spawnStartingPoint, initialVel);
				objCount += 1;
				gradientStep += 1;
				spawnStartingPoint.x += startingPointAdder;
				if (gradientStep == gradientStepMax) {
					std::reverse(gradient.begin(), gradient.end());
					gradientStep = 0;
				}
				if (spawnStartingPoint.x >= poptions.window_width - radius || spawnStartingPoint.x <= radius)
				{
					startingPointAdder *= -1;
				}
				objectList.connectedObjects.AddObject(newObject);
			}
		}
		else if (event == "REC") {
			for (size_t i = 0; i < num; i++)
			{
				objectList.CreateNewRectangle(poptions.gravity, gradient[gradientStep], spawnStartingPoint);
				objCount += 1;
				gradientStep += 1;
				spawnStartingPoint.x += startingPointAdder;
				if (gradientStep == gradientStepMax) {
					std::reverse(gradient.begin(), gradient.end());
					gradientStep = 0;
				}
				if (spawnStartingPoint.x >= poptions.window_width - radius || spawnStartingPoint.x <= radius)
				{
					startingPointAdder *= -1;
				}
			}
		}
	}

	// SIGN ^
	void EventAndID(std::vector<std::string> EventAndID) {
		std::string event = EventAndID[0];
		std::string id = EventAndID[1];
		if (event == "DEL") {
			BaseShape* obj = objectList.FindByIDStr(id);
			objectList.DeleteThisObj(obj);
		}
	}

	// SIGN *
	void SpawnObjectsSpecific(std::vector<std::string> EventAndPos) {
		std::string event = EventAndPos[0];
		sf::Vector2f pos = StringToVector2f(EventAndPos[1]);
		if (event == "PLANET") {
			createPlanet(pos);
		}
	}

	// SIGN :
	void SpawnObjectsIdentifier(std::vector<std::string> eventAndIdentifier) {
		std::string event = eventAndIdentifier[0];
		std::string identifier = eventAndIdentifier[1];

		if (event == "LINK")
		{
			if (identifier == "RND")
			{
				objCount += 1;
				objectList.connectedObjects.ConnectRandom(10);
			}
		}
	}

	// SIGN : AND *
	void SpawnObjectsIdentifierAndSpecific(std::string event, std::vector<std::string> typeAndPos) {
		std::string type = typeAndPos[0];
		sf::Vector2f pos = StringToVector2f(typeAndPos[1]);
		if (event == "EXPLOSION") {
			if (type == "CIR")
			{
				objectList.CreateNewCircle(poptions.gravity, explosion, sf::Vector2f(pos.x + 3, pos.y + 3), initialVel);
				for (size_t i = 0; i < 50; i++)
				{
					objectList.CreateNewCircle(poptions.gravity, explosion, pos, initialVel);
					objCount += 1;
				}
			}
			if (type == "REC")
			{
				objectList.CreateNewRectangle(poptions.gravity, explosion, sf::Vector2f(pos.x + 3, pos.y + 3));
				for (size_t i = 0; i < 50; i++)
				{
					objectList.CreateNewRectangle(poptions.gravity, explosion, pos);
					objCount += 1;
				}
			}
		}
	}

	// SIGN &
	void EventBetweenTwo(std::vector<std::string> splitedVec) {
		std::string event = splitedVec[0];
		std::string id1 = splitedVec[1];
		std::string id2 = splitedVec[2];

		if (event == "LINK")
		{
			BaseShape* obj = objectList.FindByIDStr(id1);
			BaseShape* obj2 = objectList.FindByIDStr(id2);
			objectList.connectObjects(obj, obj2);
		}
	}

	// SIGN ^ AND #
	void VectorEventOnID(std::string event, std::vector<std::string> idAndVector) {
		std::string id = idAndVector[0];
		sf::Vector2f pos = StringToVector2f(idAndVector[1]);
		if (event == "NEWP")
		{
			BaseShape* objPointer = objectList.FindByIDStr(id);
			if (objPointer != nullptr)
			{
				objPointer->SetPosition(pos);

			}
		}
	}

	// SIGN @ AND @ AND @ AND @
	void EventWithIdentifierOnID(std::vector<std::string> splitedToken) {
		std::string event = splitedToken[0];
		std::string type = splitedToken[1];
		std::string id = splitedToken[2];
		int power = std::stoi(splitedToken[3]);
		int mouseFlagScroll = std::stoi(splitedToken[4]);
		BaseShape* objPointer = objectList.FindByIDStr(id);
		if (event == "SCALE")
		{
			handleScaling(objPointer, power, mouseFlagScroll);
		}
	}
	void createPlanet(sf::Vector2f(currentMousePos)) {
		planetMode = true;
		objectList.CreateNewPlanet(7000, ball_color, currentMousePos, 20, 5.9722 * pow(10, 15));
		objCount++;
	}

	void handleScaling(BaseShape* objPointer, int power, int mouseFlagScroll) {
		if (mouseFlagScroll == 1 || mouseFlagScroll == -1) {
			if (Circle* circle = dynamic_cast<Circle*>(objPointer)) {
				scaleCircle(circle, power, mouseFlagScroll);
			}
			else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(objPointer))
			{
				scaleRectangle(rectangle, power , mouseFlagScroll);
			}
		}
	}

	void scaleCircle(Circle* circle, int mouseScrollPower, int mouseFlagScroll) {
		if (mouseFlagScroll ==-1 && circle->getRadius() > 0.0001) {
			circle->SetRadiusAndCenter(circle->getRadius() - mouseScrollPower);
			circle->SetMass(circle->GetMass() - mouseScrollPower * 10);
		}
		else {
			circle->SetRadiusAndCenter(circle->getRadius() + mouseScrollPower);
			circle->SetMass(circle->GetMass() + mouseScrollPower * 10);
		}
	}

	void scaleRectangle(RectangleClass* rectangle, int mouseScrollPower, int mouseFlagScroll) {
		if (mouseFlagScroll==-1 && rectangle->GetHeight() > 0.0001 && rectangle->GetWidth() > 0.0001) {
			rectangle->SetSizeAndOrigin(rectangle->GetWidth() - mouseScrollPower, rectangle->GetHeight() - mouseScrollPower);
			rectangle->SetMass(rectangle->GetMass() - mouseScrollPower * 10);
		}
		else {
			rectangle->SetSizeAndOrigin(rectangle->GetWidth() + mouseScrollPower, rectangle->GetHeight() + mouseScrollPower);
			rectangle->SetMass(rectangle->GetMass() + mouseScrollPower * 10);
		}
	}
};

int main() {
	try {
		boost::asio::io_context io_context;
		unsigned short tcp_port = 8080;
		unsigned short udp_port = 8081;

		// Create server without window dependency
		Server server(io_context, tcp_port, udp_port);
		server.Start();

		std::cout << "\nServer commands:" << std::endl;
		std::cout << "b:<message> - Broadcast message to all clients" << std::endl;
		std::cout << "s: - broadcast shapes to all clients" << std::endl;
		std::cout << "c:<client_id> <message> - send message to specific client" << std::endl;
		std::cout << "list - list all connected clients" << std::endl;

		// Create threads
		std::vector<std::thread> threads;
		const int thread_count = std::thread::hardware_concurrency();

		// Physics thread
		threads.emplace_back([&server]() {
			try {
				server.Run();
			}
			catch (const std::exception& e) {
				std::cerr << "Exception in physics thread: " << e.what() << std::endl;
			}
			});

		// Network threads
		for (int i = 1; i < thread_count; ++i) {
			threads.emplace_back([&io_context]() {
				try {
					io_context.run();
				}
				catch (const std::exception& e) {
					std::cerr << "Exception in network thread: " << e.what() << std::endl;
				}
				});
		}

		// Wait for all threads
		for (auto& thread : threads) {
			thread.join();
		}
	}
	catch (const std::exception& error) {
		std::cerr << "Exception in main: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}