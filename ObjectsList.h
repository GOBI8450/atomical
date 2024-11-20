#pragma once
#include <vector>
#include <random>
#include "Grid.h"
#include "CircleBase.h"
#include "LineLink.h"
#include "Rectangle.h"
#include "Planet.h"
#include <iostream>
#include <thread>
#include <functional>
#include <atomic>
#include "ThreadPool.h" // Include the ThreadPool header

class ObjectsList
{
private:
	int objCount = 0;
	std::mt19937 rnd;
	Grid* grid;
	std::vector<Planet*> planetList;
	float lineLength;

public:
	LineLink connectedObjects = LineLink(lineLength);
	std::vector<BaseShape*> objList;

	ObjectsList(float lineLength) :lineLength(lineLength) { // Adjust cell size as needed
		rnd.seed(static_cast<unsigned>(std::time(nullptr)));
	}

	~ObjectsList() { // Deleter or else memory leak ):
		DeleteAll();
	}

	void DeleteAll() {
		for (auto ball : objList) {
			delete ball;
		}
		objList.clear();
		planetList.clear();
		connectedObjects.Clear();
		objCount = 0;
	}

	BaseShape* CreateNewCircle(float gravity, sf::Color color, sf::Vector2f pos, sf::Vector2f initialVel) {
		std::uniform_int_distribution<int> radiusRange(30, 30);
		std::uniform_int_distribution<int> rndXRange(300, 500);  // Replace 920 with actual window width
		// std::uniform_int_distribution<int> rndYRange(50, 1280 - 50); // Replace 1280 with actual window height

		sf::Vector2f position(pos);
		int randomRadius = radiusRange(rnd);
		int mass = randomRadius * 3;//no real meaning for the multiply
		BaseShape* ball = new Circle(randomRadius, color, position, gravity, mass, initialVel, objCount);
		objList.push_back(ball); // Pushing back the BaseShape* into the vector
		objCount += 1;
		return ball;
		// std::cout << "Creating ball at position: (" << position.x << ", " << position.y << ")\n";
	}

	void CreateNewPlanet(float innerGravity, sf::Color color, sf::Vector2f pos, float radius, float mass) {
		float gravity = 0;
		Planet* planet = new Planet(radius, color, pos, gravity, mass, innerGravity, objCount);
		//^^^^^^float radius, sf::Color color, sf::Vector2f pos, float gravity, double mass, float innerGravity^^^^
		objList.push_back(planet); // Pushing back the BaseShape* into the vector
		planetList.push_back(planet); // Pushing back the BaseShape* into the vector
		objCount += 1;
	}

	void CreateNewRectangle(float gravity, sf::Color color, sf::Vector2f pos) {
		std::uniform_int_distribution<int> heightRange(50, 50);
		std::uniform_int_distribution<int> widthRange(50, 50);
		std::uniform_int_distribution<int> rndXRange(300, 500);  // Replace 920 with actual window width
		// std::uniform_int_distribution<int> rndYRange(50, 1280 - 50); // Replace 1280 with actual window height
		int randomHeight = heightRange(rnd);
		int randomWidth = widthRange(rnd);
		int mass = (randomWidth + randomHeight) * 2;//no real meaning for the multiply
		sf::Vector2f position(pos);

		BaseShape* ball = new RectangleClass(randomWidth, randomHeight, color, position, gravity, mass, objCount);
		objList.push_back(ball); // Pushing back the BaseShape* into the vector
		objCount += 1;

		// std::cout << "Creating ball at position: (" << position.x << ", " << position.y << ")\n";
	}

	void connectObjects(BaseShape* shape, BaseShape* target) {
		connectedObjects.MakeNewLink(shape, target);
	}

	void HandleCollisionsInRange(sf::RenderWindow& window, float elastic, std::vector<std::vector<BaseShape*>> vecOfVecObj) {
		if (elastic == 0) { // Verlet integration
			for (auto& vecObj : vecOfVecObj) {
				for (auto& obj : vecObj) {
					// Check if obj is a Circle
					if (Circle* circle = dynamic_cast<Circle*>(obj)) {
						circle->handleWallCollision(window);
					}
					// Check if obj is a Rectangle
					else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
						rectangle->handleWallCollision(window);
					}

					// Get nearby objects for collision handling
					std::vector<BaseShape*> potentialCollisions = grid->GetNerbyCellsObjects(obj);

					for (auto& otherObj : potentialCollisions) {
						// Handle Circle to Circle collision
						if (Circle* circle = dynamic_cast<Circle*>(obj)) {
							if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj)) {
								if (circle != otherCircle) {
									circle->HandleCollision(otherCircle);
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
							std::cout << "recCir";
						}
						else if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj)) {
							if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj))
							{
								rectangle->HandleCollision(otherCircle); // Handle collision with any other shape
							}
							std::cout << "recCir";
						}
					}
				}
			}
		}
		else { // Euler integration
			for (auto& vecObj : vecOfVecObj) {
				for (auto& obj : vecObj) {
					// Check if obj is a Circle
					if (Circle* circle = dynamic_cast<Circle*>(obj)) {
						circle->handleWallCollision(window);
					}
					// Check if obj is a Rectangle
					else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
						rectangle->handleWallCollision(window);
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
	}

	void HandleAllCollisions(sf::RenderWindow& window, float elastic) {
		if (elastic == 0) { // Verlet integration
			for (auto& obj : objList) {
				// Check if obj is a Circle
				if (Circle* circle = dynamic_cast<Circle*>(obj)) {
					circle->handleWallCollision(window);
				}
				// Check if obj is a Rectangle
				else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
					rectangle->handleWallCollision(window);
				}

				// Get nearby objects for collision handling
				std::vector<BaseShape*> potentialCollisions = grid->GetNerbyCellsObjects(obj);

				for (auto& otherObj : potentialCollisions) {
					// Handle Circle to Circle collision
					if (Circle* circle = dynamic_cast<Circle*>(obj)) {
						if (Circle* otherCircle = dynamic_cast<Circle*>(otherObj)) {
							if (circle != otherCircle) {
								circle->HandleCollision(otherCircle);
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
				// Check if obj is a Circle
				if (Circle* circle = dynamic_cast<Circle*>(obj)) {
					circle->handleWallCollision(window);
				}
				// Check if obj is a Rectangle
				else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
					rectangle->handleWallCollision(window);
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

	BaseShape* IsInRadius(sf::Vector2f pointPos) {
		return grid->IsInGridRadius(pointPos); // Return nullptr if no ball contains the point
	}

	void DeleteThisObj(BaseShape* obj) {
		auto potentialErased = std::find(objList.begin(), objList.end(), obj);

		// If found, erase it from the vector
		if (potentialErased != objList.end()) {
			objList.erase(potentialErased);
		}
		delete obj;
	}

	std::vector<BaseShape> ConvertForSending() {
		std::vector<BaseShape> objectsVec_NON_POINTER;
		for (auto& obj : objList)
		{
			objectsVec_NON_POINTER.push_back(*obj);
		}
		return objectsVec_NON_POINTER;
	}

	void DrawObjVec(sf::RenderWindow& window, float fps) {
		float deltaTime = 1 / fps;
		for (auto& ball : objList) {
			ball->updatePositionVerlet(deltaTime);
			ball->draw(window);
		}
		connectedObjects.Draw(window);
	}

	void MoveAndDraw(sf::RenderWindow& window, float fps, float elastic, bool planetMode, bool enableCollison, bool borderless) {
		if (borderless)
		{
			grid = new GridUnorderd();
		}
		else
		{
			grid = new GridFixed();
		}

		grid->clear(); // Clear the grid

		for (auto& ball : objList) {
			grid->InsertObj(ball); // Inserting BaseShape* objects
		}

		if (fps <= 0) {
			fps = 60;
		}
		float deltaTime = 1 / fps; // Calculate deltaTime for movement
		if (enableCollison)
		{
			HandleAllCollisions(window, elastic);
		}
		for (auto& planet : planetList)
		{
			for (auto& ball : objList) {
				if (typeid(*ball) != typeid(*planet))
				{
					planet->Gravitate(ball);
				}
			}
		}
		connectedObjects.Draw(window);
		//grid->DrawGrids(window);
		if (planetMode)
		{
			for (auto& ball : objList) {
				ball->updatePositionEuler(deltaTime);
				ball->draw(window);
			}
		}
		else
		{
			for (auto& ball : objList) {
				ball->updatePositionVerlet(deltaTime);
				ball->draw(window);
			}
		}
	}
};