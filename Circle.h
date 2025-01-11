#pragma once
#include <SFML/Graphics.hpp>
#include <iostream> 
#include "BaseShape.h"

// Base_Shape class inheriting from sf::Transformable
class Circle :public BaseShape, public sf::CircleShape
{
protected:
	float radius; //float to not be too big

public:
	Circle() : BaseShape(), radius(0.0) {}; // must have deffult constructor for networking

	// Constructor with radius, color, gravity, mass
	Circle(sf::Color color, float gravity, double mass, float radius, int objCount)
		: BaseShape(color, gravity, mass, objCount), radius(radius)
	{
		setRadius(radius);
		setFillColor(color);
		setOrigin(radius, radius);
		setPosition(radius, radius);
		oldPosition = sf::Vector2f(radius, radius);
		acceleration = sf::Vector2f(0, gravity * 100); //(x axis, y axis)
		oldPosition = oldPosition - velocity * (1.f / 60.f);
		type = "Circle";
	}

	// Constructor with radius, color, gravity, mass, position
	Circle(float radius, sf::Color color, sf::Vector2f pos, float gravity, double mass, sf::Vector2f initialVel, int objCount)
		: BaseShape(color, gravity, mass, objCount), radius(radius)
	{
		setRadius(radius);
		setFillColor(color);
		setOrigin(radius, radius);
		setPosition(pos);
		oldPosition = pos;
		acceleration = sf::Vector2f(0, gravity * 100);//(x axis, y axis)
		SetVelocity(initialVel);
		oldPosition = oldPosition - velocity * (1.f / 60.f);
		//SetOutline(sf::Color(255, 255, 255), 0.5);  // cool visual
		type = "Circle";
	}

	//update the position based on verlet integration.
	void updatePositionVerlet(float dt) override
	{
		sf::Vector2f currentPos = getPosition();
		sf::Vector2f newPos = currentPos + (currentPos - oldPosition) + acceleration * (dt * dt);

		// Update velocity
		velocity = (newPos - oldPosition) / (2 * dt);

		oldPosition = currentPos;
		setPosition(newPos);
	}


	//update the position based on euler integration.
	void updatePositionEuler(float dt) override
	{
		sf::Vector2f currentPos = GetPosition();
		sf::Vector2f newPos = currentPos + velocity * dt;
		setPosition(newPos);
		// Update velocity for the next frame
		velocity = velocity + acceleration * dt;
	}

	//Function that handles the walls collisons:
	void handleWallCollision(int window_width, int window_height)
	{
		sf::Vector2f pos = GetPosition();
		float energyLossFactor = 0;// If you wanna add energy loss
		//as the origin point is set to the center of the circle the point will be always radius far away from its edges
		if (pos.x - radius < 0)
		{
			oldPosition.x = pos.x;
			pos.x = radius;
			oldPosition.x = pos.x + (pos.x - oldPosition.x) * energyLossFactor; // 0.9f Damping factor
		}
		else if (pos.x + radius > window_width)
		{
			oldPosition.x = pos.x;
			pos.x = window_width - radius;
			oldPosition.x = pos.x + (pos.x - oldPosition.x) * energyLossFactor;
		}

		if (pos.y - radius < 0)
		{
			oldPosition.y = pos.y;
			pos.y = radius;
			oldPosition.y = pos.y + (pos.y - oldPosition.y) * energyLossFactor;
		}
		else if (pos.y + radius > window_height)
		{
			oldPosition.y = pos.y;
			pos.y = window_height - radius;
			oldPosition.y = pos.y + (pos.y - oldPosition.y) * energyLossFactor;
		}

		setPosition(pos);
	}

	double DistanceOnly(Circle* otherShape) {
		sf::Vector2f pos = GetPosition();       // Position of this shape
		sf::Vector2f posOther = otherShape->GetPosition(); // Position of the other shape
		double x1 = pos.x;
		double y1 = pos.y;
		double x2 = posOther.x;
		double y2 = posOther.y;
		return std::sqrt(std::pow((y2 - y1), 2) + std::pow((x2 - x1), 2)); // Return distance
	}

	double DistanceToPoint(sf::Vector2f posOther) {
		sf::Vector2f pos = GetPosition();       // Position of this shape
		double x1 = pos.x;
		double y1 = pos.y;
		double x2 = posOther.x;
		double y2 = posOther.y;
		return std::sqrt(std::pow((y2 - y1), 2) + std::pow((x2 - x1), 2)); // Return distance
	}

	//Checks if there is any collision between two balls
	bool IsCollision(Circle* otherCir) {
		double distance = DistanceOnly(otherCir);
		if (distance <= radius + otherCir->radius) {
			return true;
		}
		return false;
	}

	bool IsInRadius(sf::Vector2f pos) {
		// Get the actual position of the circle (considering its center)
		sf::Vector2f circleCenter = GetPosition();

		// Calculate distance between point and circle center
		float dx = pos.x - circleCenter.x;
		float dy = pos.y - circleCenter.y;
		float distanceSquared = dx * dx + dy * dy;

		// Check if point is within circle
		return distanceSquared <= (radius * radius);
	}

	//The collision handeling is done by seperating the two circles with a vector between the two of them, and the overlap of them. the speed that will be created is done by verlet integretion
	void HandleCollision(Circle* otherCir) {
		if (IsCollision(otherCir)) {
			// Get positions of both circles
			sf::Vector2f pos = GetPosition();
			sf::Vector2f posOther = otherCir->GetPosition();

			// Calculate the distance and overlap
			double distance = DistanceOnly(otherCir);
			double overlap = (radius + otherCir->radius) - distance;

			if (overlap > 0) {
				sf::Vector2f direction = pos - posOther; // The direction vector, between center points of the circles
				float length = sqrt(direction.x * direction.x + direction.y * direction.y); // The length between the circles in scalar

				if (length > 0) {
					direction /= length; // Normalize the direction vector
				}

				// Calculate mass ratio (more influence for the heavier object)
				float massRatio = mass / otherCir->mass;

				// Adjust displacement calculation to account for both sizes more precisely
				sf::Vector2f displacement = direction * static_cast<float>(overlap / 2.0f); // Split overlap
				pos += displacement * (1 / massRatio);  // Move this circle
				posOther -= displacement * massRatio; // Move the other circle

				// Ensure small circles aren't just being "ignored"
				if (radius < 1.0f || otherCir->radius < 1.0f) {
					// For very small circles, adjust the threshold for how much they can move
					displacement *= 0.5f; // You can tweak this factor based on your desired behavior
				}

				// Update positions
				setPosition(pos);
				otherCir->setPosition(posOther);
			}
		}
	}


	//handeling collision with euler integretion, using momentum equations
	void HandleCollisionElastic(Circle* otherCir, float elastic) {
		if (IsCollision(otherCir)) {
			// Current positions
			sf::Vector2f pos = GetPosition();
			sf::Vector2f posOther = otherCir->GetPosition();

			// Calculate the normal vector
			sf::Vector2f normal = pos - posOther;
			float distance = std::sqrt(normal.x * normal.x + normal.y * normal.y);
			normal /= distance;  // Normalize

			// Calculate relative velocity using position difference
			sf::Vector2f velocity = pos - oldPosition;
			sf::Vector2f velocityOther = posOther - otherCir->oldPosition;
			sf::Vector2f relativeVelocity = velocity - velocityOther;

			// Calculate impulse scalar
			//(0.8 = slightly bouncy)
			float impulseScalar = -(1 + elastic) * (relativeVelocity.x * normal.x + relativeVelocity.y * normal.y) /
				(1 / mass + 1 / otherCir->mass);// This is like friction, if this is 0 means tottaly elastic. if more than 0 than the friction will be more and more noticable.

			// Apply impulse
			sf::Vector2f impulse = normal * impulseScalar;
			sf::Vector2f newVelocity = velocity + impulse / static_cast<float>(mass);
			sf::Vector2f newVelocityOther = velocityOther - impulse / static_cast<float>(otherCir->mass);

			// Update positions and oldPositions
			oldPosition = pos;
			otherCir->oldPosition = posOther;
			setPosition(pos + newVelocity);//!This is the part that do the hit physicly accurate, we add the new velocity to the position like euler integration!
			otherCir->setPosition(posOther + newVelocityOther);//!This is the part that do the hit physicly accurate, we add the new velocity to the position like euler integration!

			// Separate circles to prevent sticking
			float overlap = (radius + otherCir->radius) - distance;
			sf::Vector2f separation = normal * (overlap / 2.0f);
			setPosition(GetPosition() + separation);
			otherCir->setPosition(otherCir->GetPosition() - separation);
		}
	}

	//Set the radius to a new one, and centers the origin point according to the new radius
	void SetRadiusAndCenter(int newRadius) {
		setRadius(newRadius);
		radius = newRadius;
		setOrigin(sf::Vector2f(radius, radius));
	}

	// Set shape color
	void setColor(sf::Color newColor) override
	{
		color = newColor;  // Update the member variable
		setFillColor(newColor);
	}

	void SetPosition(sf::Vector2f newPos) override
	{
		setPosition(newPos);
	}

	void SetRadius(float newRadius)
	{
		setRadius(newRadius);
		radius = newRadius;
		setOrigin(sf::Vector2f(radius, radius));
	}

	float GetRadius() {
		return radius;
	}

	sf::Vector2f GetPosition() const override {
		return getPosition();
	}

	std::string GetPositionStr() const override {
		std::stringstream ss;
		ss << "X=" << GetPosition().x << "Y=" << GetPosition().y;
		return ss.str();
	}

	void SetOutline(sf::Color color, float thickness) override {
		setOutlineThickness(thickness);
		setOutlineColor(color);
	}

	std::string ToString() const override {
		std::stringstream ss;

		// Call BaseShape::ToString() to include base properties
		ss << BaseShape::ToString() << ":"
			<< radius << ":"          // Circle-specific property: Radius
			<< velocity.y << ":" << velocity.x;  // Circle-specific property: Velocity

		return ss.str();
	}

	sf::FloatRect GetGlobalBounds()  override {
		return getGlobalBounds();
	}

	float GetEstimatedSize() override {
		return radius;
	}

	// Function to draw the circle
	void draw(sf::RenderWindow& window)
	{
		window.draw(*this);
	}
};