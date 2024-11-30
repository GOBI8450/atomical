#pragma once
#include <SFML/Graphics.hpp>
#include <iostream> 
#include <sstream>


class BaseShape
{
protected:
	sf::Vector2f oldPosition; //Vector for X axis and Y axis
	sf::Vector2f acceleration; //Vector for X axis and Y axis
	sf::Vector2f velocity;
	sf::Color color;
	float gravity; // float to not be too strong
	double mass;//be as fat as you want brotha
	double fps;//my worst enemy
	int linked;
	int id;

public:
	static int objectCount;

	BaseShape() // must have deffult constructor for networking
		: oldPosition(0.f, 0.f),
		acceleration(0.f, 0.f),
		color(sf::Color::White),
		gravity(0.f),
		mass(0.0),
		fps(0.0),
		linked(-1),
		id(-1)
	{
		// This constructor sets default values
	}

	BaseShape(sf::Color color, float gravity, double mass, int objCount)
		: color(color), gravity(gravity), mass(mass)
	{
		linked = -1;
		acceleration = sf::Vector2f(0, gravity); //(x axis, y axis)
		objectCount = objCount;
		id = objectCount;
	}
	// Copy constructor
	BaseShape(const BaseShape& other)
		: oldPosition(other.oldPosition),
		acceleration(other.acceleration),
		color(other.color),
		gravity(other.gravity),
		mass(other.mass),
		fps(other.fps),
		linked(other.linked)
	{
	}

	virtual ~BaseShape() { objectCount--; }


	//Updates the position following varlet integration. meaning we calculate the next position based on the previos one and with time
	virtual void updatePositionVerlet(float dt) {}
	virtual void updatePositionEuler(float dt) {}

	//If you wanna apply force to the circle:
	void applyForce(sf::Vector2f force) {
		acceleration = sf::Vector2f(force.x / mass, force.y / mass);
	}

	// Set shape color
	virtual void setColor(sf::Color newColor) {}

	//Set the mass of the circle
	void SetMass(double newMass) { mass = newMass; }

	void SetID(int newID) { id = newID; }

	//Get the mass of the circle. if I will use it on your mother I will get an out of bounderies error!
	double GetMass() { return mass; }

	// Function to draw the circle
	virtual void draw(sf::RenderWindow& window) {}

	double Distance(BaseShape* otherShape) {
		sf::Vector2f pos = GetPosition();       // Position of this shape
		sf::Vector2f posOther = otherShape->GetPosition(); // Position of the other shape
		double x1 = pos.x;
		double y1 = pos.y;
		double x2 = posOther.x;
		double y2 = posOther.y;
		return std::sqrt(std::pow((y2 - y1), 2) + std::pow((x2 - x1), 2)); // Return distance
	}

	double DistanceTwoPoints(sf::Vector2f otherPos) {
		sf::Vector2f pos = GetPosition();// Position of this shape
		double x1 = pos.x;
		double y1 = pos.y;
		double x2 = otherPos.x;
		double y2 = otherPos.y;
		return std::sqrt(std::pow((y2 - y1), 2) + std::pow((x2 - x1), 2)); // Return distance
	}

	virtual sf::Vector2f GetPosition() const { return sf::Vector2f(-1, -1); }

	virtual std::string GetPositionStr() const { return ""; }

	float GetGravity() {
		return gravity;
	}

	void SetGravity(float newGravity) {
		gravity = newGravity;
	}

	virtual void SetPosition(sf::Vector2f newPos) {}

	void SetVelocity(const sf::Vector2f& newVelocity) { velocity = newVelocity; }
	void SetVelocity(float x, float y) { velocity = sf::Vector2f(x, y); }
	sf::Vector2f GetVelocity() const { return velocity; }

	void SetAcceleration(sf::Vector2f newAcceleration) {
		acceleration = newAcceleration;
	}

	sf::Vector2f GetAcceleration() {
		return acceleration;
	}
	// Getter for oldPosition
	sf::Vector2f GetOldPosition() const {
		return oldPosition;
	}

	// Setter for oldPosition
	void SetOldPosition(const sf::Vector2f& newOldPosition) {
		oldPosition = newOldPosition;
	}

	void SetLinked(bool isLinked) { linked = isLinked; }

	int GetLinked() { return linked; }

	virtual void SetOutline(sf::Color color, float thickness) {}

	sf::Color GetColor() { return color; }

	int GetID() { return id; };

	std::string GetIDStr() { return std::to_string(id); }

	std::string GetColorAsString() {
		return "R->" + std::to_string(color.r) + ", G->" + std::to_string(color.g) + ", B->" + std::to_string(color.b);
	}

	virtual std::string GetType() const {
		return "BaseShape";
	}

	virtual void gdetGlobalBounds() {
		return;
	}

	virtual std::string ToString() const {
		std::stringstream ss;
		sf::Vector2f pos = GetPosition();
		ss << GetType() << ":"       // Shape type
			<< id << ":"              // ID
			<< "(" << std::to_string(color.r) << "," << std::to_string(color.g) << "," << std::to_string(color.b) << "):"  // Color
			<< mass << ":"            // Mass
			<< pos.y << ":" << pos.x << ":"  // Position
			<< acceleration.x << ":" << acceleration.y << ":"  // Acceleration
			<< linked;                // Linked flag
		return ss.str();
	}
};

