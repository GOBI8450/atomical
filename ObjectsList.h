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
#include "ThreadPool.h" // Include the ThreadPool header

class ObjectsList
{
private:
	int objCount = 0;
	std::mt19937 rnd;
	Grid* grid;
	std::vector<Planet*> planetList;
	std::vector<ElectricalParticle*> electricalParticlesList;
	float lineLength;
	std::vector<BaseShape*> fixedObjects;

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
		std::uniform_int_distribution<int> radiusRange(20, 20);

		sf::Vector2f position(pos);
		int randomRadius = radiusRange(rnd);
		int mass = randomRadius * 3;//no real meaning for the multiply
		objCount += 1;
		BaseShape* ball = new Circle(randomRadius, color, position, gravity, mass, initialVel, objCount);
		objList.push_back(ball); // Pushing back the BaseShape* into the vector
		return ball;
		// std::cout << "Creating ball at position: (" << position.x << ", " << position.y << ")\n";
	}

	BaseShape* CreateNewFixedCircle(sf::Color color, sf::Vector2f pos) {
		std::uniform_int_distribution<int> radiusRange(20, 20);

		sf::Vector2f position(pos);
		int randomRadius = radiusRange(rnd);
		int mass = randomRadius * 3;//no real meaning for the multiply
		objCount += 1;
		BaseShape* ball = new Circle(randomRadius, color, position, 0, mass, sf::Vector2f(0, 0), objCount);
		objList.push_back(ball); // Pushing back the BaseShape* into the vector of all objects
		fixedObjects.push_back(ball); // Pushing back the BaseShape* into the vector of fixed objects
		return ball;
		// std::cout << "Creating ball at position: (" << position.x << ", " << position.y << ")\n";
	}

	void CreateNewPlanet(float innerGravity, sf::Color color, sf::Vector2f pos, float radius, float mass) {
		float gravity = 0;
		Planet* planet = new Planet(radius, color, pos, gravity, mass, innerGravity, objCount);
		objList.push_back(planet); // Pushing back the BaseShape* into the vector of all objects
		planetList.push_back(planet); // Pushing back the BaseShape* into the vector of planets
		objCount += 1;
	}

	void CreateNewElectricalParticle(double charge, bool isFixed, sf::Vector2f initialVel, sf::Color color, sf::Vector2f pos, float radius, float mass) {
		float gravity = 0;
		ElectricalParticle* particle = new ElectricalParticle(radius, color, pos, gravity, mass, charge, isFixed, initialVel, objCount);
		objList.push_back(particle); // Pushing back the BaseShape* into the vector of all objects
		electricalParticlesList.push_back(particle); // Pushing back the BaseShape* into the vector of electrical particles
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

	void connectObjects(BaseShape* shape, BaseShape* target, int type) {
		connectedObjects.MakeNewLink(shape, target, type);
	} //TODO : Use this because it is more OOP way

	BaseShape* createNewLinkedCircle(BaseShape* target, int type, float gravity, sf::Color color, sf::Vector2f pos, sf::Vector2f initialVel) {
		CreateNewCircle(gravity, color, pos, initialVel);
		connectedObjects.MakeNewLink(objList[objCount - 1], target, type);
		return objList[objCount - 1];
	}

	void HandleCollisionsInRange(int window_width, int window_height, float elastic, std::vector<std::vector<BaseShape*>> vecOfVecObj) {
		if (elastic == 0) { // Verlet integration
			for (auto& vecObj : vecOfVecObj) {
				for (auto& obj : vecObj) {
					// Check if obj is a Circle
					if (Circle* circle = dynamic_cast<Circle*>(obj)) {
						circle->handleWallCollision(window_width, window_height);
					}
					// Check if obj is a Rectangle
					else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
						rectangle->handleWallCollision(window_width, window_height);
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
						circle->handleWallCollision(window_width, window_height);
					}
					// Check if obj is a Rectangle
					else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
						rectangle->handleWallCollision(window_width, window_height);
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

	void HandleAllCollisions(int window_width, int window_height, float elastic) {
		if (elastic == 0) { // Verlet integration
			for (auto& obj : objList) {
				// Check if obj is a Circle
				if (Circle* circle = dynamic_cast<Circle*>(obj)) {
					circle->handleWallCollision(window_width, window_height);
				}
				// Check if obj is a Rectangle
				else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
					rectangle->handleWallCollision(window_width, window_height);
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
					circle->handleWallCollision(window_width, window_height);
				}
				// Check if obj is a Rectangle
				else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
					rectangle->handleWallCollision(window_width, window_height);
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

	void ChangeGravityForAll(float gravity) {
		for (auto& obj : objList)
		{
			obj->SetGravity(gravity);
		}
	}

	// In ObjectsList class:
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
	std::vector<BaseShape> ConvertForSending() {
		std::vector<BaseShape> objectsVec_NON_POINTER;
		for (auto& obj : objList)
		{
			objectsVec_NON_POINTER.push_back(*obj);
		}
		return objectsVec_NON_POINTER;
	}

	void DrawObjects(sf::RenderWindow& window, float fps, bool planetMode) {
		float deltaTime = 1 / fps;
		connectedObjects.Draw(window);
		//grid->DrawGrids(window);
		if (planetMode)
		{
			for (auto& ball : objList) {
				ball->draw(window);
			}
		}
		else
		{
			for (auto& ball : objList) {
				ball->draw(window);
			}
		}
	}

	void MoveWhenFreeze(int window_width, int window_height, float fps, bool borderless) {
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
			ball->SetOldPosition(ball->GetPosition());
		}

		if (fps <= 0) {
			fps = 60;
		}
		float deltaTime = 1 / fps; // Calculate deltaTime for movement
		connectedObjects.ApplyAllLinks();

	}

	void MoveObjects(int window_width, int window_height, float fps, float elastic, bool planetMode, bool enableCollison, bool borderless) {
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
			HandleAllCollisions(window_width, window_height, elastic);
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
		for (int i = 0; i < electricalParticlesList.size(); i++)// o(n^2) so not optimal but must do.
		{
			if (!electricalParticlesList[i]->GetIsFixed())
			{
				sf::Vector2f allForces = sf::Vector2f(0, 0);
				for (int j = 0; j < electricalParticlesList.size(); j++)
				{
					if (i != j || !electricalParticlesList[j]->GetIsFixed()) {
						allForces += electricalParticlesList[i]->coulombLaw(electricalParticlesList[j]);
					}
				}
				electricalParticlesList[i]->applyForce(allForces);
			}
		}
		connectedObjects.ApplyAllLinks();
		if (planetMode)
		{
			for (auto& ball : objList) {
				ball->updatePositionEuler(deltaTime);
			}
		}
		else
		{
			for (auto& ball : objList) {
				ball->updatePositionVerlet(deltaTime);
			}
		}
		for (auto& ball : fixedObjects) {
			ball->SetPosition(ball->GetOldPosition());
			ball->SetAcceleration(sf::Vector2f(0, 0));
		}
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
			HandleAllCollisions(window.getSize().x, window.getSize().y, elastic);
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
		for (int i = 0; i < electricalParticlesList.size(); i++)
		{
			for (int j = 0; j < electricalParticlesList.size(); j++)
			{
				if (i != j) {
					electricalParticlesList[i]->coulombLaw(electricalParticlesList[j]);
				}
			}
		}
		if (planetMode)
		{
			for (auto& shape : objList) {
				shape->updatePositionEuler(deltaTime);
				shape->draw(window);
			}
		}
		else
		{
			for (auto& shape : objList) {
				shape->updatePositionVerlet(deltaTime);
				shape->draw(window);
			}
		}
	}
};