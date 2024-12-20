#pragma once
#include <SFML/Graphics.hpp>
#include <iostream> 
#include "Circle.h"

class Planet :public Circle
{
private:
	float innerGravity;

public:
	Planet(float radius, sf::Color color, sf::Vector2f pos, float gravity, double mass, float innerGravity, int objCount) : Circle(radius, color, pos, gravity, mass, sf::Vector2f(0, 0), objCount), innerGravity(innerGravity) {

	}

	// Copy constructor
	Planet(Planet& other)
		: Circle(other) {
		// Copy SFML shape properties
		setRadius(other.getRadius());
		setFillColor(other.getFillColor());
		setOrigin(other.getOrigin());
		setPosition(other.getPosition());

		// Copy Circle class properties
		oldPosition = other.oldPosition;
		acceleration = other.acceleration;
		mass = other.mass;
		gravity = other.gravity;
	}

	std::string GetType() const override {
		return "Planet";
	}


	sf::Vector2f GravitateAccurate(BaseShape* object) {
		// Calculate the vector from this object to the other object
		sf::Vector2f distanceVec = object->GetPosition() - GetPosition(); // Reversed direction

		// Calculate the distance between the two objects
		float distanceLength = std::sqrt(distanceVec.x * distanceVec.x + distanceVec.y * distanceVec.y);

		// Avoid division by zero or extremely small distances
		const float epsilon = 1e-6f;
		float distanceSquared = distanceLength * distanceLength;

		// More robust proximity check
		if (distanceLength < epsilon || distanceSquared <= (4 * radius * radius)) {
			return sf::Vector2f(0, 0);
		}

		// Normalize the distance vector to get the direction of the gravitational force
		sf::Vector2f normalVector = distanceVec / distanceLength;

		// Gravitational constant
		const float G = 6.67430e-11f;

		// Calculate the gravitational force magnitude using the formula:
		// F = G * (m1 * m2) / d^2
		float forceMagnitude = (G * GetMass() * object->GetMass()) / distanceSquared;

		// Return the force in the direction of the normalized vector
		// Positive sign to work with Verlet's force accumulation
		return normalVector * forceMagnitude;
	}

	//For planet and non planet interaction
	void Gravitate(BaseShape* object, float dt)
	{
		// Calculate the vector from this object (Planet) to the other object
		float distance_x = GetPosition().x - object->GetPosition().x;
		float distance_y = GetPosition().y - object->GetPosition().y;

		// Calculate the distance between the two objects
		float distance = sqrt(distance_x * distance_x + distance_y * distance_y);

		// Avoid division by zero in case the objects are at the same position. and if the object too close to the planet do not calculate so it wont fly away
		if (distance == 0 || distance <= std::abs(object->GetEstimatedSize() * 2)) return;

		// Normalize the direction vector
		float inverse_distance = 1.f / distance;
		float normalized_x = inverse_distance * distance_x;
		float normalized_y = inverse_distance * distance_y;

		// Calculate gravitational force with inverse square law
		float inverse_square_dropoff = inverse_distance * inverse_distance;

		// Calculate acceleration due to gravity based on innerGravity (strength of gravity)
		float acceleration_x = normalized_x * innerGravity * inverse_square_dropoff;
		float acceleration_y = normalized_y * innerGravity * inverse_square_dropoff;

		object->SetAcceleration(sf::Vector2f(acceleration_x, acceleration_y));
	}

};