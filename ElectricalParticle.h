#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Circle.h"

// Constants for Coulomb's law and particle properties
#define K_ 8.987551792300000e+09 // Coulomb's constant (N·m²/C²)
// Proton properties
#define PROTON_MASS 1.67262192369e-27  // Mass of a proton (kg)
#define PROTON_CHARGE 1.602176634e-19  // Charge of a proton (C)
// Neutron properties
#define NEUTRON_MASS 1.67492749804e-27  // Mass of a neutron (kg)
#define NEUTRON_CHARGE 0.0              // Charge of a neutron (C)
// Electron properties
#define ELECTRON_MASS 9.1093837015e-31  // Mass of an electron (kg)
#define ELECTRON_CHARGE -1.602176634e-19 // Charge of an electron (C)

// Class representing an electrical particle, inheriting from Circle
class ElectricalParticle : public Circle
{
private:
    double charge; // Electric charge of the particle
    bool isFixed;  // Flag to indicate if the particle is fixed in place

public:
    // Constructor: Initializes an electrical particle with specific properties
    ElectricalParticle(float radius, sf::Color color, sf::Vector2f pos, float gravity, double mass, double charge, bool fixed, sf::Vector2f initialVel, int objCount)
        : Circle(radius, color, pos, gravity, mass, initialVel, objCount), charge(charge), isFixed(fixed) {
    }

    // Copy constructor: Creates a copy of another ElectricalParticle
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
        type = "ElectPart"; // Set the type to "ElectricalParticle"
    }

    virtual ~ElectricalParticle() = default;

    // Gets the charge of the particle
    double GetCharge() {
        return charge;
    }

    // Checks if the particle is fixed in place
    bool GetIsFixed() {
        return isFixed;
    }

    // Calculates the force exerted on another object using Coulomb's law
    // Coulomb's law: F = (K * q1 * q2) / r²
    sf::Vector2f coulombLaw(BaseShape* object) {
        // Calculate the distance vector between this particle and the other object
        sf::Vector2f distanceVec = GetPosition() - object->GetPosition();
        float distanceSquared = distanceVec.x * distanceVec.x + distanceVec.y * distanceVec.y;
        const float epsilon = 1e-6f; // Small value to avoid division by zero

        // Avoid division by zero and overlap
        if (distanceSquared < epsilon * epsilon || distanceSquared <= (4 * radius * radius + radius / 10)) {
            return sf::Vector2f(0, 0); // No force if too close
        }

        // Check if the other object is an ElectricalParticle
        ElectricalParticle* otherParticle = dynamic_cast<ElectricalParticle*>(object);
        if (!otherParticle) {
            return sf::Vector2f(0, 0); // No force if the object is not an ElectricalParticle
        }

        // Calculate the force magnitude using Coulomb's law
        double forceScalar = (K_ * charge * otherParticle->GetCharge()) / distanceSquared;

        // Normalize the direction vector
        sf::Vector2f normalVector = distanceVec / std::sqrt(distanceSquared);

        // Compute the force vector
        sf::Vector2f force = normalVector * static_cast<float>(forceScalar);

        // Update the position of the other object based on its velocity
        object->SetPosition(object->GetPosition() + object->GetVelocity());

        return force; // Return the force vector (repulsion or attraction)
    }
};