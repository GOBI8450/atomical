#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>

// Base class for all shapes in the simulation
class BaseShape
{
protected:
    sf::Vector2f oldPosition;  // Previous position of the shape (used for physics calculations)
    sf::Vector2f acceleration; // Current acceleration of the shape
    sf::Vector2f velocity;     // Current velocity of the shape
    sf::Color color;           // Color of the shape
    float gravity;             // Gravity affecting the shape
    double mass;               // Mass of the shape
    double fps;                // Frames per second (used for time-based calculations)
    int linked;                // Indicates if the shape is linked to another object
    int id;                    // Unique identifier for the shape
    std::string type = "BaseShape"; // Type of the shape (default is "BaseShape")

public:
    static int objectCount; // Static counter to track the number of objects created

    // Default constructor: Initializes the shape with default values
    BaseShape()
        : oldPosition(0.f, 0.f),
        acceleration(0.f, 0.f),
        color(sf::Color::White),
        gravity(0.f),
        mass(0.0),
        fps(0.0),
        linked(-1),
        id(-1),
        type("BaseShape")
    {
        // This constructor sets default values
    }

    // Parameterized constructor: Initializes the shape with specific values
    BaseShape(sf::Color color, float gravity, double mass, int objCount)
        : color(color), gravity(gravity), mass(mass)
    {
        linked = -1;
        acceleration = sf::Vector2f(0, gravity); // Set acceleration based on gravity
        objectCount = objCount;                 // Update object count
        id = objectCount;                       // Assign a unique ID
        type = "BaseShape";                     // Set the type
    }

    // Copy constructor: Creates a copy of another BaseShape object
    BaseShape(const BaseShape& other)
        : oldPosition(other.oldPosition),
        acceleration(other.acceleration),
        color(other.color),
        gravity(other.gravity),
        mass(other.mass),
        fps(other.fps),
        linked(other.linked),
        type("BaseShape")
    {
    }

    // Destructor: Decrements the object count when a shape is destroyed
    virtual ~BaseShape() { objectCount--; }

    // Updates the position using Verlet integration (to be overridden by derived classes)
    virtual void updatePosition_SubSteps(float dt, int numSubsteps) {}

    // Updates the position using a simpler method (to be overridden by derived classes)
    virtual void updatePosition(float dt) {}

    // Applies a single force to the shape, updating its acceleration
    void applyOneForce(sf::Vector2f force) {
        acceleration = sf::Vector2f(force.x / mass, force.y / mass);
    }

    // Applies gravity to the shape, adjusting its acceleration
    void applyGravity(sf::Vector2f gravityForce) {
        acceleration += sf::Vector2f(gravityForce.x / mass, gravityForce.y / mass) - sf::Vector2f(0, gravity / mass);
    }

    // Adds a force to the shape, modifying its acceleration
    void addForce(sf::Vector2f force) {
        acceleration += sf::Vector2f(force.x / mass, force.y / mass);
    }

    // Sets the color of the shape (to be overridden by derived classes)
    virtual void setColor(sf::Color newColor) {}

    // Sets the mass of the shape
    void SetMass(double newMass) { mass = newMass; }

    // Sets the unique ID of the shape
    void SetID(int newID) { id = newID; }

    // Gets the mass of the shape
    double GetMass() { return mass; }

    // Draws the shape (to be overridden by derived classes)
    virtual void draw(sf::RenderWindow& window) {}

    // Calculates the distance between this shape and another shape
    double Distance(BaseShape* otherShape) {
        sf::Vector2f pos = GetPosition();       // Position of this shape
        sf::Vector2f posOther = otherShape->GetPosition(); // Position of the other shape
        double x1 = pos.x;
        double y1 = pos.y;
        double x2 = posOther.x;
        double y2 = posOther.y;
        return std::sqrt(std::pow((y2 - y1), 2) + std::pow((x2 - x1), 2)); // Return distance
    }

    // Calculates the distance between this shape and a specific point
    double DistanceTwoPoints(sf::Vector2f otherPos) {
        sf::Vector2f pos = GetPosition(); // Position of this shape
        double x1 = pos.x;
        double y1 = pos.y;
        double x2 = otherPos.x;
        double y2 = otherPos.y;
        return std::sqrt(std::pow((y2 - y1), 2) + std::pow((x2 - x1), 2)); // Return distance
    }

    // Gets the position of the shape (to be overridden by derived classes)
    virtual sf::Vector2f GetPosition() const { return sf::Vector2f(-1, -1); }

    // Gets the position of the shape as a string (to be overridden by derived classes)
    virtual std::string GetPositionStr() const { return ""; }

    // Gets the gravity affecting the shape
    float GetGravity() {
        return gravity;
    }

    // Sets the gravity affecting the shape
    void SetGravity(float newGravity) {
        applyGravity(sf::Vector2f(0, gravity));
        gravity = newGravity;
    }

    // Sets the position of the shape (to be overridden by derived classes)
    virtual void SetPosition(sf::Vector2f newPos) {}

    // Sets the velocity of the shape
    void SetVelocity(const sf::Vector2f& newVelocity) {
        velocity = newVelocity;
        oldPosition = oldPosition - velocity * (1.f / 60.f); // Adjust old position based on velocity
    }

    // Sets the velocity of the shape using individual components
    void SetVelocity(float x, float y) {
        velocity = sf::Vector2f(x, y);
        oldPosition = oldPosition - velocity * (1.f / 60.f); // Adjust old position based on velocity
    }

    // Gets the velocity of the shape
    sf::Vector2f GetVelocity() const { return velocity; }

    // Sets the acceleration of the shape
    void SetAcceleration(sf::Vector2f newAcceleration) {
        acceleration = newAcceleration;
    }

    // Gets the acceleration of the shape
    sf::Vector2f GetAcceleration() {
        return acceleration;
    }

    // Gets the previous position of the shape
    sf::Vector2f GetOldPosition() const {
        return oldPosition;
    }

    // Sets the previous position of the shape
    void SetOldPosition(const sf::Vector2f& newOldPosition) {
        oldPosition = newOldPosition;
    }

    // Sets whether the shape is linked to another object
    void SetLinked(bool isLinked) { linked = isLinked; }

    // Gets whether the shape is linked to another object
    int GetLinked() { return linked; }

    // Sets the outline of the shape (to be overridden by derived classes)
    virtual void SetOutline(sf::Color color, float thickness) {}

    // Gets the color of the shape
    sf::Color GetColor() { return color; }

    // Gets the unique ID of the shape
    int GetID() { return id; }

    // Gets the unique ID of the shape as a string
    std::string GetIDStr() { return std::to_string(id); }

    // Gets the color of the shape as a string
    std::string GetColorAsString() {
        return "R->" + std::to_string(color.r) + ", G->" + std::to_string(color.g) + ", B->" + std::to_string(color.b);
    }

    // Gets the type of the shape
    virtual std::string GetType() const {
        return type;
    }

    // Sets the type of the shape
    void SetType(std::string newType) {
        type = newType;
    }

    // Gets the global bounds of the shape (to be overridden by derived classes)
    virtual sf::FloatRect GetGlobalBounds() {
        return sf::FloatRect();
    }

    // Gets an estimated size of the shape (to be overridden by derived classes)
    virtual float GetEstimatedSize() {
        return 0;
    }

    // Converts the shape's properties to a string representation
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