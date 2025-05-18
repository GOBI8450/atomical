#pragma once
#include <vector>
#include <random>
#include "Grid.h"
#include "CircleBase.h"
#include "LineLink.h"
#include "Rectangle.h"
#include "Planet.h"
#include "ElectricalParticle.h"
#include <iostream>
#include <thread>
#include <functional>
#include <atomic>

class ObjectsList
{
private:
	int objCount = 0;
	std::mt19937 rnd;
	Grid* grid;
	std::vector<std::pair<Planet*, sf::VertexArray>> planetList;
	std::vector<ElectricalParticle*> electricalParticlesList;
	float lineLength;
	std::vector<BaseShape*> fixedObjects;
	int subSteps = 1;

public:
	// Object responsible for managing connections (links) between objects
	LineLink connectedObjects = LineLink(lineLength);

	// List of all objects in the simulation
	std::vector<BaseShape*> objList;

	// Constructor: Initializes the object list with a specified line length
	ObjectsList(float lineLength) : lineLength(lineLength) {
		rnd.seed(static_cast<unsigned>(std::time(nullptr))); // Seed the random number generator
		grid = new GridUnorderd();
	}

	// Destructor: Ensures all dynamically allocated memory is freed
	~ObjectsList() {
		DeleteAll(); // Clean up all objects
	}

	// Deletes all objects and clears the lists to prevent memory leaks
	void DeleteAll() {
		for (auto ball : objList) {
			delete ball;
		}
		objList.clear();
		planetList.clear();
		connectedObjects.Clear();
		fixedObjects.clear();
		electricalParticlesList.clear();
		objCount = 0;
		grid->clear();
		//std::cout << "After DeleteAll: "
		//	<< "objList = " << objList.size()
		//	<< ", fixedObjects = " << fixedObjects.size()
		//	<< ", electricalParticlesList = " << electricalParticlesList.size()
		//	<< std::endl;
	}

	// Creates a new circle object with specified properties and adds it to the object list
	BaseShape* CreateNewCircle(float gravity, sf::Color color, sf::Vector2f pos, sf::Vector2f initialVel) {
		std::uniform_int_distribution<int> radiusRange(20, 20); // Fixed radius range
		sf::Vector2f position(pos);
		int randomRadius = radiusRange(rnd);
		int mass = 1; // Default mass
		objCount += 1;
		BaseShape* ball = new Circle(randomRadius, color, position, gravity, mass, initialVel, objCount);
		objList.push_back(ball);
		return ball;
	}

	// Creates a new fixed (immovable) circle and adds it to both the object list and fixed objects list
	BaseShape* CreateNewFixedCircle(sf::Color color, sf::Vector2f pos) {
		std::uniform_int_distribution<int> radiusRange(20, 20); // Fixed radius range
		sf::Vector2f position(pos);
		int randomRadius = radiusRange(rnd);
		double mass = randomRadius * 3; // Arbitrary mass calculation
		objCount += 1;
		BaseShape* ball = new Circle(randomRadius, color, position, 0, mass, sf::Vector2f(0, 0), objCount);
		objList.push_back(ball);
		fixedObjects.push_back(ball);
		return ball;
	}

	// Adds a thick line (visual trail) between two points to a vertex array
	void addThickLine(sf::VertexArray& vertices, const sf::Vector2f& start, const sf::Vector2f& end, float thickness, const sf::Color& color) {
		sf::Vector2f direction = end - start;
		float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
		if (length != 0) {
			direction.x /= length;
			direction.y /= length;
		}
		sf::Vector2f perpendicular(-direction.y * thickness / 2, direction.x * thickness / 2);

		// Define the 4 corners of the rectangle
		sf::Vector2f topLeft = start + perpendicular;
		sf::Vector2f bottomLeft = start - perpendicular;
		sf::Vector2f topRight = end + perpendicular;
		sf::Vector2f bottomRight = end - perpendicular;

		// Append the 4 vertices to the VertexArray
		vertices.append(sf::Vertex(topLeft, color));
		vertices.append(sf::Vertex(bottomLeft, color));
		vertices.append(sf::Vertex(bottomRight, color));
		vertices.append(sf::Vertex(topRight, color));
	}

	// Rebuilds a vertex array to only keep the last `maxVertices` vertices
	void rebuildVertexArray(sf::VertexArray& vertices, size_t maxVertices) {
		size_t vertexCount = vertices.getVertexCount();
		if (vertexCount > maxVertices) {
			sf::VertexArray newVertices(sf::Quads);
			for (size_t i = vertexCount - maxVertices; i < vertexCount; ++i) {
				newVertices.append(vertices[i]);
			}
			vertices = newVertices;
		}
	}

	// Creates a new planet with specified properties and adds it to the object and planet lists
	void CreateNewPlanet(float innerGravity, sf::Color color, sf::Vector2f pos, float radius, float mass) {
		float gravity = 0;
		Planet* planet = new Planet(radius, color, pos, gravity, mass, innerGravity, objCount);
		objList.push_back(planet);
		sf::VertexArray trackingLine(sf::Quads);
		planetList.push_back(std::make_pair(planet, trackingLine));
		objCount += 1;
	}

	// Creates a new electrical particle and adds it to the object and electrical particle lists
	void CreateNewElectricalParticle(double charge, bool isFixed, sf::Vector2f initialVel, sf::Color color, sf::Vector2f pos, float radius, float mass) {
		float gravity = 0;
		ElectricalParticle* particle = new ElectricalParticle(radius, color, pos, gravity, mass, charge, isFixed, initialVel, objCount);
		objList.push_back(particle);
		electricalParticlesList.push_back(particle);
		objCount += 1;
	}

	// Creates a new rectangle object and adds it to the object list
	void CreateNewRectangle(float gravity, sf::Color color, sf::Vector2f pos) {
		std::uniform_int_distribution<int> heightRange(20, 20);
		std::uniform_int_distribution<int> widthRange(20, 20);
		int randomHeight = heightRange(rnd);
		int randomWidth = widthRange(rnd);
		int mass = 1; // Default mass
		sf::Vector2f position(pos);
		BaseShape* ball = new RectangleClass(randomWidth, randomHeight, color, position, gravity, mass, objCount);
		objList.push_back(ball);
		objCount += 1;
	}

	// Connects two objects using the LineLink class
	void connectObjects(BaseShape* shape, BaseShape* target, int type) {
		connectedObjects.MakeNewLink(shape, target, type);
	}

	// Creates a new circle and links it to an existing object
	BaseShape* createNewLinkedCircle(BaseShape* target, int type, float gravity, sf::Color color, sf::Vector2f pos, sf::Vector2f initialVel) {
		CreateNewCircle(gravity, color, pos, initialVel);
		connectedObjects.MakeNewLink(objList[objCount - 1], target, type);
		return objList[objCount - 1];
	}

	// Handles all collisions between objects in the simulation
	void HandleAllCollisions(int window_width, int window_height, float elastic, bool borderless, float fps) {
		float dt = 1 / fps;
		if (elastic == 0) { // Verlet integration
			for (BaseShape* obj : objList){
				if (!borderless)
				{
					// Check if obj is a Circle
					if (Circle* circle = dynamic_cast<Circle*>(obj)) {
						circle->handleWallCollision(window_width, window_height);
					}
					// Check if obj is a Rectangle
					else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
						rectangle->handleWallCollision(window_width, window_height);
					}
				}

				// Get nearby objects for collision handling
				std::vector<BaseShape*> potentialCollisions = grid->GetNerbyCellsObjects(obj);

				for (auto& otherObj : potentialCollisions) {
					// Handle Circle to Circle collision
					if (Circle* circle = dynamic_cast<Circle*>(obj)) {
						if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj)) {
							if (circle != otherCircle) {
								circle->HandleCollision(otherCircle, dt, subSteps);
							}
						}
					}
					// Handle Rectangle to other object collision
					else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
						if (RectangleClass* otherRectangle = dynamic_cast<RectangleClass*>(otherObj))
						{
							if (rectangle != otherRectangle) {
								rectangle->HandleCollision(otherRectangle); // Handle collision with any other shape
							}
						}
					}
					if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
						if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj))
						{
							rectangle->HandleCollision(otherCircle); // Handle collision with any other shape
						}
					}
					else if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj)) {
						if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj))
						{
							rectangle->HandleCollision(otherCircle); // Handle collision with any other shape
						}
					}
				}
			}
		}
		else { // Euler integration
			for (auto& obj : objList) {
				if (!borderless)
				{
					// Check if obj is a Circle
					if (Circle* circle = dynamic_cast<Circle*>(obj)) {
						circle->handleWallCollision(window_width, window_height);
					}
					// Check if obj is a Rectangle
					else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
						rectangle->handleWallCollision(window_width, window_height);
					}
				}

				// Get nearby objects for collision handling
				std::vector<BaseShape*> potentialCollisions = grid->GetNerbyCellsObjects(obj);

				for (auto& otherObj : potentialCollisions) {
					// Handle Circle to Circle collision
					if (Circle* circle = dynamic_cast<Circle*>(obj)) {
						if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj)) {
							if (circle != otherCircle) {
								circle->HandleCollisionElastic(otherCircle, elastic);
							}
						}
					}
					// Handle Rectangle to other object collision
					else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
						if (RectangleClass* otherRectangle = dynamic_cast<RectangleClass*>(otherObj))
						{
							if (rectangle != otherRectangle) {
								rectangle->HandleCollisionElastic(otherRectangle, elastic); // Handle collision with any other shape
							}
						}

					}
				}
			}
		}
	}

	// Checks if a point is within the radius of any object
	BaseShape* IsInRadius(sf::Vector2f pointPos) {
		return grid->IsInGridRadius(pointPos); // Return nullptr if no ball contains the point
	}

	// Deletes a specific object from the object list
	void DeleteThisObj(BaseShape* obj) {
		auto potentialErased = std::find(objList.begin(), objList.end(), obj);

		// If found, erase it from the vector
		if (potentialErased != objList.end()) {
			objList.erase(potentialErased);
		}
		delete obj;
	}

	// Finds an object by its string ID
	BaseShape* FindByIDStr(std::string id) { //TODO: better serch???
		for (auto obj : objList)
		{
			if (obj->GetIDStr() == id)
			{
				return obj;
			}
		}
		return nullptr;
	}

	// Finds an object by its integer ID
	BaseShape* FindByID(int id) { //TODO: better serch???
		for (auto obj : objList)
		{
			if (obj->GetID() == id)
			{
				return obj;
			}
		}
		return nullptr;
	}

	// Changes the gravity for all objects
	void ChangeGravityForAll(float gravity) {
		for (auto& obj : objList)
		{
			obj->SetAcceleration(sf::Vector2f( 0,gravity *100));
			//obj->SetOldPosition(obj->GetOldPosition() - sf::Vector2f(0, obj->GetGravity()) + sf::Vector2f(0, gravity));
		}
	}

	// Changes the collision behavior for all objects (TODO: Implement)
	void ChangeCollisonForAll(float gravity) {
		//TODO:
	}

	// Changes the line length for all connections
	void ChangeLineLengthForAll(float lineLength) {
		connectedObjects.SetLineLength(lineLength);
	}

	// Changes the velocity for all objects
	void ChangeVelocityForAll(sf::Vector2f newVelocity) {
		for (auto& obj : objList)
		{
			if (obj->GetType() == "Circle" || obj->GetType() == "Rectangle")
			{
				obj->SetVelocity(newVelocity);
			}
		}
	}

	// Checks if a point is within the area of any object
	int checkIfPointInObjectArea(sf::Vector2f pos) {
		for (auto& obj : objList) {
			if (Circle* circle = dynamic_cast<Circle*>(obj)) {
				if (circle->IsInRadius(pos)) {
					return obj->GetID();
				}
			}
			else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
				sf::FloatRect bounds = rectangle->GetGlobalBounds();

				if (bounds.contains(pos)) {
					return obj->GetID();
				}
			}
		}
		return -1;
	}

	// Combines all objects into a single list
	std::vector<BaseShape*> CombineAllObjects() {
		std::vector<BaseShape*> combinedObjects;
		combinedObjects.insert(combinedObjects.end(), objList.begin(), objList.end());
		combinedObjects.insert(combinedObjects.end(), fixedObjects.begin(), fixedObjects.end());
		for (auto planet : planetList) {
			BaseShape* planetPointer = planet.first;
			combinedObjects.push_back(planetPointer);
		}
		for (auto particle : electricalParticlesList) {
			BaseShape* particlePointer = particle;
			combinedObjects.push_back(particlePointer);
		}
		return combinedObjects;
	}

	// Draws all objects and their connections
	void DrawObjects(sf::RenderWindow& window, float fps, bool planetMode) {
		float deltaTime = 1 / fps;
		connectedObjects.Draw(window);
		//grid->DrawGrids(window);
		if (planetMode)
		{
			for (auto& planet : planetList) {
				window.draw(planet.second);
			}
		}
		for (auto& ball : objList) {
			ball->draw(window);
		}

	}

	// Moves objects when the simulation is frozen
	void MoveWhenFreeze(int window_width, int window_height, float fps, bool borderless) {
		/*if (borderless)
		{
			grid = new GridUnorderd();
		}
		else
		{
			grid = new GridFixed();
		}*/
		grid->clear(); // Clear the grid

		for (auto& ball : objList) {
			grid->InsertObj(ball); // Inserting BaseShape* objects
			ball->SetOldPosition(ball->GetPosition());
		}

		if (fps <= 0) {
			fps = 60;
		}
		float deltaTime = 1 / fps; // Calculate deltaTime for movement
		connectedObjects.ApplyAllLinks();

	}

	// Moves objects and handles physics updates
	void MoveObjects(int window_width, int window_height, float fps, float elastic, bool enableCollison, bool borderless) {
		// Grid setup (temporarily forced to GridUnorderd regardless of 'borderless' flag)
		// if (borderless) {
		//     grid = new GridUnorderd();
		// } else {
		//     grid = new GridFixed();
		// }

		grid->clear(); // Clear the current spatial partitioning grid

		// Insert all objects into the grid for spatial partitioning (collision or other proximity checks)
		for (auto& ball : objList) {
			grid->InsertObj(ball); // Inserting BaseShape* objects
		}

		// Ensure a valid FPS value
		if (fps <= 0) {
			fps = 60;
		}

		float dt = 1 / fps; // Calculate deltaTime for movement updates

		// Handle collisions if the flag is enabled
		if (enableCollison) {
			HandleAllCollisions(window_width, window_height, elastic, borderless, fps);
		}

		// Apply gravitational force from planets to all objects in objList (excluding type-matching objects)
		for (auto& ball : objList) {
			sf::Vector2f allForces = sf::Vector2f(0, 0);
			for (int i = 0; i < planetList.size(); i++) {
				if (ball != nullptr && planetList[i].first != nullptr && typeid(*ball) != typeid(*planetList[i].first)) {
					allForces += planetList[i].first->Gravitate(ball, dt); // Accumulate gravitational force
				}
			}
			ball->addForce(allForces); // Apply the net gravitational force
		}

		// Update planets' behavior and visual trails
		for (int i = 0; i < planetList.size(); i++) {
			sf::Vector2f allForces = sf::Vector2f(0, 0);
			for (int j = 0; j < planetList.size(); j++) {
				if (i != j) {
					allForces += planetList[i].first->GravitateAccurate(planetList[j].first); // Calculate mutual gravitational force
				}
			}
			if (planetList[i].first != nullptr) {
				planetList[i].first->applyOneForce(allForces); // Apply total gravitational force from other planets

				// Draw motion trail using thick lines and vertex array updates
				addThickLine(
					planetList[i].second,
					planetList[i].first->GetOldPosition(),
					planetList[i].first->GetPosition(),
					planetList[i].first->GetRadius() / 1.5,
					planetList[i].first->GetColor()
				);
				rebuildVertexArray(planetList[i].second, 252);

				// Fade the trail over time by decreasing alpha on each vertex rectangle (RGBA)
				for (int alphaChange = planetList[i].second.getVertexCount() - 4; alphaChange >= 0; alphaChange -= 4) {
					sf::Color newColor = planetList[i].first->GetColor();
					newColor.a = std::max<sf::Uint8>(0u, newColor.a - (planetList[i].second.getVertexCount() - alphaChange - 4));

					// Apply fading color to all 4 corners of the trail segment
					planetList[i].second[alphaChange].color = newColor;
					planetList[i].second[alphaChange + 1].color = newColor;
					planetList[i].second[alphaChange + 2].color = newColor;
					planetList[i].second[alphaChange + 3].color = newColor;
				}
				// planetList[i].first->SetOldPosition(planetList[i].first->GetPosition()); // (Optional) update old position if needed
			}
		}

		// Compute and apply electrostatic forces between all electrical particles (O(n^2) complexity)
		for (int i = 0; i < electricalParticlesList.size(); i++) {
			if (!electricalParticlesList[i]->GetIsFixed()) { // Only process non-fixed particles
				sf::Vector2f allForces = sf::Vector2f(0, 0);
				for (int j = 0; j < electricalParticlesList.size(); j++) {
					if (i != j || !electricalParticlesList[j]->GetIsFixed()) {
						allForces += electricalParticlesList[i]->coulombLaw(electricalParticlesList[j]); // Coulomb interaction
					}
				}
				electricalParticlesList[i]->applyOneForce(allForces); // Apply net electric force
			}
		}

		// Update constraints or links between connected objects
		connectedObjects.ApplyAllLinks();

		// Update positions of all objects using sub-stepping
		for (auto& ball : objList) {
			ball->updatePosition_SubSteps(dt, subSteps);
		}

		// Reset fixed objects to their previous state so they stay fixed
		for (auto& ball : fixedObjects) {
			ball->SetPosition(ball->GetOldPosition());
			ball->SetAcceleration(sf::Vector2f(0, 0));
			ball->SetVelocity(sf::Vector2f(0, 0));
		}
	}
};