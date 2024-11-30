#pragma once
#include <SFML/Graphics.hpp>
#include <iostream> 
#include "Circle.h"
#define K_ 8.987551792300000e+09
// Proton
#define PROTON_MASS 1.67262192369e-27  // kg
#define PROTON_CHARGE 1.602176634e-19  // C
// Neutron
#define NEUTRON_MASS 1.67492749804e-27  // kg
#define NEUTRON_CHARGE 0.0              // C
// Electron
#define ELECTRON_MASS 9.1093837015e-31  // kg
#define ELECTRON_CHARGE -1.602176634e-19 // C

class ElectricalParticle :public Circle
{
private:
	double charge;
	bool isFixed;

public:
	ElectricalParticle(float radius, sf::Color color, sf::Vector2f pos, float gravity, double mass, double charge, bool fixed, sf::Vector2f initialVel, int objCount) : Circle(radius, color, pos, gravity, mass, initialVel, objCount), charge(charge), isFixed(fixed) {

	}

	// Copy constructor
	ElectricalParticle(ElectricalParticle& other)
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

	double GetCharge() {
		return charge;
	}

	bool GetIsFixed() {
		return isFixed;
	}

	//Column law -> (K*q1*q2)/R^2
	sf::Vector2f coulombLaw(BaseShape* object) {
		sf::Vector2f distanceVec = GetPosition() - object->GetPosition();
		float distanceSquared = distanceVec.x * distanceVec.x + distanceVec.y * distanceVec.y;
		const float epsilon = 1e-6f;

		// Avoid division by zero and overlap
		if (distanceSquared < epsilon * epsilon || distanceSquared <= (4 * radius * radius + radius / 10)) {
			return sf::Vector2f(0, 0);
		}

		ElectricalParticle* otherParticle = dynamic_cast<ElectricalParticle*>(object);
		if (!otherParticle) {
			return sf::Vector2f(0, 0);
		}

		double forceScalar = (K_ * charge * otherParticle->GetCharge()) / distanceSquared;

		// Normalize the direction vector
		sf::Vector2f normalVector = distanceVec / std::sqrt(distanceSquared);

		// Compute the force vector
		sf::Vector2f force = normalVector * static_cast<float>(forceScalar);
		object->SetPosition(object->GetPosition() + object->GetVelocity());
		return force; // Repulsion and attraction are automatically correct
	}
};