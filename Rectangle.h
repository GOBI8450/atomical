#pragma once
#include <SFML/Graphics.hpp>
#include <iostream> 
#include "BaseShape.h"
#include "Circle.h"
#define M_PI   3.14159265358979323846264338327950288

class RectangleClass : public BaseShape, public sf::RectangleShape
{
private:
	float width;
	float height;
	float angularVelocity;    // Angular velocity in radians per second
	float rotation;           // Current rotation in degrees
	float momentOfInertia;    // Moment of inertia for the rectangle
	float torque;             // Current rotational force

public:
	RectangleClass() :BaseShape(), width(0.0), height(0.0) {}; // must have deffult constructor for networking
	// Constructor with radius, color, gravity, mass	
	RectangleClass(float width, float height, sf::Color color, float gravity, double mass, int objCount)
		: BaseShape(color, gravity, mass, objCount), width(width), height(height), angularVelocity(0.0f), rotation(0.0f), torque(0.0f)
	{
		setSize(sf::Vector2f(width, height));
		setFillColor(color);
		setOrigin(width / 2, height / 2);
		setPosition(width / 2, height / 2);
		oldPosition = sf::Vector2f(width, height);
		acceleration = sf::Vector2f(0, gravity * 100); //(x axis, y axis)
		type = "Rectangle";
		// Calculate moment of inertia for a rectangle
		// I = (1/12) * mass * (width^2 + height^2)
		momentOfInertia = (mass * (width * width + height * height)) / 12.0f;
	}

	// Constructor with radius, color, gravity, mass, position
	RectangleClass(float width, float height, sf::Color color, sf::Vector2f pos, float gravity, double mass, int objCount)
		: BaseShape(color, gravity, mass, objCount), width(width), height(height), angularVelocity(0.0f), rotation(0.0f), torque(0.0f)
	{
		setSize(sf::Vector2f(width, height));
		setFillColor(color);
		setOrigin(width / 2, height / 2);
		setPosition(pos);
		oldPosition = pos;
		acceleration = sf::Vector2f(0, gravity * 100);//(x axis, y axis)
		type = "Rectangle";
		// Calculate moment of inertia for a rectangle
		// I = (1/12) * mass * (width^2 + height^2)
		momentOfInertia = (mass * (width * width + height * height)) / 12.0f;
	}

	// Modify the updatePosition method:
	//update the position based on verlet integration.
	void updatePosition_SubSteps(float dt, int numSubsteps) override
	{
		sf::Vector2f currentPos = getPosition();
		sf::Vector2f newPos = currentPos + (currentPos - oldPosition) + acceleration * (dt * dt);

		// Update velocity
		velocity = (newPos - oldPosition) / (2 * dt);

		oldPosition = currentPos;
		setPosition(newPos);

		// Angular motion update
		float currentRotation = getRotation();
		angularVelocity += (torque / momentOfInertia) * dt;
		float newRotation = currentRotation + angularVelocity * dt * (180.0f / 3.14); // Convert to degrees
		setRotation(std::fmod(newRotation, 360.0f));

		// Apply damping to prevent infinite rotation
		angularVelocity *= 0.98f;
		torque = 0.0f;
	}


	//update the position based on verlet integration.
	void updatePosition(float dt) override
	{
		sf::Vector2f currentPos = getPosition();
		sf::Vector2f newPos = currentPos + (currentPos - oldPosition) + acceleration * (dt * dt);

		// Update velocity
		velocity = (newPos - oldPosition) / (2 * dt);

		oldPosition = currentPos;
		setPosition(newPos);
	}


	// Set shape color
	void setColor(sf::Color newColor) override
	{
		color = newColor;  // Update the member variable
		setFillColor(newColor);
	}

	// Function to draw the rectangle
	void draw(sf::RenderWindow& window)
	{
		window.draw(*this);
	}

	void handleWallCollision(int window_width, int window_height) {
		sf::Vector2f pos = getPosition();
		float energyLossFactor = 0.0f; // For velocity damping
		float angularDampingFactor = 0.99f; // For angular velocity damping

		// Store current velocity (calculated from positions)
		sf::Vector2f vel = pos - oldPosition;

		// Check for horizontal collision
		if (pos.x - width / 2 < 0 || pos.x + width / 2 > window_width) {
			// Reverse X velocity and apply damping
			vel.x = -vel.x * energyLossFactor;

			// Reflect angular velocity (simulate tangential collision force)
			float tangentEffect = vel.y * 0.05f; // Small factor based on tangential velocity
			angularVelocity += tangentEffect;

			// Clamp position to stay within bounds
			pos.x = (pos.x - width / 2 < 0) ? width / 2 : window_width - width / 2;
		}

		// Check for vertical collision
		if (pos.y - height / 2 < 0 || pos.y + height / 2 > window_height) {
			// Reverse Y velocity and apply damping
			vel.y = -vel.y * energyLossFactor;

			// Reflect angular velocity (simulate tangential collision force)
			float tangentEffect = vel.x * 0.05f; // Small factor based on tangential velocity
			angularVelocity += tangentEffect;

			// Clamp position to stay within bounds
			pos.y = (pos.y - height / 2 < 0) ? height / 2 : window_height - height / 2;
		}

		// Update oldPosition to create the bouncing effect
		oldPosition = pos - vel;
		setPosition(pos);

		// Apply angular damping
		angularVelocity *= angularDampingFactor;
	}

	double FindOverlap(RectangleClass* otherRec) {
		sf::Vector2f pos = GetPosition();
		sf::Vector2f otherPos = otherRec->GetPosition();

		float halfW1 = width / 2, halfH1 = height / 2;
		float halfW2 = otherRec->width / 2, halfH2 = otherRec->height / 2;

		// Calculate the distances between centers along each axis
		float distX = std::abs(pos.x - otherPos.x);
		float distY = std::abs(pos.y - otherPos.y);

		// Calculate the combined half-widths and half-heights
		float combinedHalfWidth = halfW1 + halfW2;
		float combinedHalfHeight = halfH1 + halfH2;

		// Calculate the overlap in each axis (if any)
		float overlapX = combinedHalfWidth - distX;
		float overlapY = combinedHalfHeight - distY;

		// If both overlaps are positive, rectangles are overlapping
		if (overlapX > 0 && overlapY > 0) {
			// Return the smaller overlap, which represents the minimum push required to separate the rectangles
			return std::min(overlapX, overlapY);
		}

		// No overlap
		return 0.0;
	}

	double FindOverlap(sf::CircleShape* circle) {
		sf::Vector2f circlePos = circle->getPosition();  // Circle's center
		sf::Vector2f rectPos = GetPosition();      // Rectangle's center

		float halfRectWidth = width / 2;
		float halfRectHeight = height / 2;

		// Calculate the closest point on the rectangle to the circle's center
		float closestX = std::max(rectPos.x - halfRectWidth, std::min(circlePos.x, rectPos.x + halfRectWidth));
		float closestY = std::max(rectPos.y - halfRectHeight, std::min(circlePos.y, rectPos.y + halfRectHeight));

		// Calculate the distance from the circle's center to this closest point
		float distanceX = circlePos.x - closestX;
		float distanceY = circlePos.y - closestY;
		float distanceSquared = distanceX * distanceX + distanceY * distanceY;

		float radius = circle->getRadius();

		// If the distance is less than the radius, they overlap
		if (distanceSquared <= radius * radius) {
			// Calculate overlap as the difference between radius and distance
			float distance = std::sqrt(distanceSquared);
			return radius - distance;
		}

		// No overlap
		return 0.0;
	}

	//Checks if there is any collision between two Rectangles
	bool IsCollision(RectangleClass* otherRec) {
		return GetGlobalBounds().intersects(otherRec->GetGlobalBounds());
	}

	//Check if a point intersects with a rectangle
	bool IsCollision(sf::Vector2f otherPos) {
		sf::Vector2f pos = getPosition();
		float x = pos.x;
		float y = pos.y;
		float xMouse = otherPos.x;
		float yMouse = otherPos.y;
		if (xMouse <= x + width / 2 && xMouse >= x - width / 2 && yMouse <= y + height / 2 && yMouse >= y - height / 2)
		{
			return true;
		}
		return false;
	}

	void HandleCollision(RectangleClass* otherRec) {
		if (!IsCollision(otherRec)) return;

		sf::Vector2f pos1 = GetPosition();
		sf::Vector2f pos2 = otherRec->GetPosition();

		// Calculate collision normal and point
		sf::Vector2f normal = pos1 - pos2;
		float distance = std::sqrt(normal.x * normal.x + normal.y * normal.y);
		if (distance < 0.0001f) return; // Prevent division by zero

		normal /= distance; // Normalize

		// Find collision point (approximate)
		sf::Vector2f collisionPoint;
		float rot1 = getRotation() * 3.14 / 180.0f; // Convert to radians
		float rot2 = otherRec->getRotation() * 3.14 / 180.0f;

		// Calculate corners considering rotation
		sf::Vector2f corner1(width / 2 * std::cos(rot1), height / 2 * std::sin(rot1));
		sf::Vector2f corner2(width / 2 * std::cos(rot2), height / 2 * std::sin(rot2));

		// Estimate collision point using closest corners
		collisionPoint = pos1 + corner1;

		// Calculate radius vectors (from center to collision point)
		sf::Vector2f r1 = collisionPoint - pos1;
		sf::Vector2f r2 = collisionPoint - pos2;

		// Calculate relative velocity at collision point
		sf::Vector2f v1 = velocity + sf::Vector2f(-r1.y * angularVelocity, r1.x * angularVelocity);
		sf::Vector2f v2 = otherRec->velocity + sf::Vector2f(-r2.y * otherRec->angularVelocity,
			r2.x * otherRec->angularVelocity);
		sf::Vector2f relativeVel = v1 - v2;

		// Calculate impulse magnitude
		float restitution = 0.8f;
		float velocityAlongNormal = relativeVel.x * normal.x + relativeVel.y * normal.y;

		// Skip if objects are moving apart
		if (velocityAlongNormal > 0) return;

		float j = -(1.0f + restitution) * velocityAlongNormal;
		j /= (1.0f / mass + 1.0f / otherRec->mass) +
			(r1.x * r1.x + r1.y * r1.y) / momentOfInertia +
			(r2.x * r2.x + r2.y * r2.y) / otherRec->momentOfInertia;

		// Apply linear impulse
		sf::Vector2f impulse = normal * j;
		velocity += impulse * static_cast<float>(1.0f / mass);
		otherRec->velocity -= impulse * static_cast<float>(1.0f / otherRec->mass);

		// Apply angular impulse
		float angularImpulse1 = (r1.x * impulse.y - r1.y * impulse.x) / momentOfInertia;
		float angularImpulse2 = (r2.x * impulse.y - r2.y * impulse.x) / otherRec->momentOfInertia;

		angularVelocity += angularImpulse1;
		otherRec->angularVelocity -= angularImpulse2;

		// Add some random torque for visible rotation effect
		float randomTorque = (std::rand() % 1000 - 500) * 0.1f;
		torque += randomTorque;
		otherRec->torque -= randomTorque;

		// Separate objects
		float overlap = FindOverlap(otherRec);
		sf::Vector2f separation = normal * (overlap * 0.5f);
		setPosition(pos1 + separation);
		otherRec->setPosition(pos2 - separation);
	}

	void applyTorque(float t) {
		torque += t;
	}

	float getAngularVelocity() const {
		return angularVelocity;
	}

	void HandleCollision(Circle* circle) {
		if (isCollison(circle))
		{
			// Get positions of both circles
			sf::Vector2f pos = GetPosition();
			sf::Vector2f posOther = circle->GetPosition();

			// Calculate the distance and overlap
			double distance = Distance(circle);
			double overlap = FindOverlap(circle);

			if (overlap > 0) {
				sf::Vector2f direction = pos - posOther;//The direction vector, between center points of the circles
				float length = sqrt(direction.x * direction.x + direction.y * direction.y); // The length between the circles in scalar
				if (length > 0) { //if there is any length between them we should devide the direction by the length so it will give only the direction, like you dont say go right 5km you say go right.
					direction /= length; // Normalize the direction vector
				}
				float massRatio = mass / circle->GetMass();
				// Move circles apart based on the overlap so they will no longer be in contact
				sf::Vector2f displacement = direction * static_cast<float>(overlap / 2.0f); // Split overlap
				pos += displacement;  // Move this circle
				posOther -= displacement * massRatio; // Move the other circle

				// Update positions
				setPosition(pos);
				circle->setPosition(posOther);
			}
		}
	}

	bool isCollison(Circle* circle) {
		sf::Vector2f pos = GetPosition();
		sf::Vector2f otherPos = circle->GetPosition();
		float xRec = pos.x;
		float yRec = pos.y;
		float xCir = otherPos.x;
		float yCir = otherPos.y;
		float testX = xCir;
		float testY = yCir;
		if (xCir < xRec) { testX = xRec; } // left edge
		else if (xCir > xRec + width) { testX = xRec + width; } // right edge
		if (yCir < yRec) { testY = yRec; } // top edge
		else if (yCir > yRec + height) { testY = yRec + height; } // bottom edge
		float distX = xCir - testX;
		float distY = yCir - testY;
		float distance = sqrt((distX * distX) + (distY * distY));
		if (distance <= circle->getRadius()) {
			return true;
		}
		return false;
	}

	void HandleCollisionElastic(RectangleClass* otherRec, float elastic) {}///TODO: make it

	float GetHeight() {
		return height;
	}

	float GetWidth() {
		return width;
	}

	void SetPosition(sf::Vector2f newPos) override
	{
		setPosition(newPos);
	}

	void SetSizeAndOrigin(float newWidth, float newHeight) {
		width = newWidth;
		height = newHeight;
		setOrigin(width, height);
	}

	void SetOutline(sf::Color color, float thickness) override {
		setOutlineThickness(thickness);
		setOutlineColor(color);
	}

	sf::Vector2f GetPosition() const override {
		return getPosition();
	}

	std::string GetPositionStr() const override {
		std::stringstream ss;
		ss << "X=" << GetPosition().x << "Y=" << GetPosition().y;
		return ss.str();
	}

	sf::FloatRect GetGlobalBounds()  override {
		return getGlobalBounds();
	}

	float GetEstimatedSize() override {
		return std::max(height, width);
	}

	std::string ToString() const override {
		std::stringstream ss;

		// Call BaseShape::ToString() to include base properties
		ss << BaseShape::ToString() << ":"
			<< height << ":" << width << ":"  // Rectangle-specific properties: Height and Width
			<< velocity.y << ":" << velocity.x;  // Rectangle-specific property: Velocity

		return ss.str();
	}

};

