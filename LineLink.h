#pragma once
#include "Circle.h"
#include "Rectangle.h"
#include "BaseShape.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <random>
#include <ctime>
#include <iostream>
#include <functional>
#define PI       3.14159265358979323846   // pi


class LineLink
{
private:
	float lineLength;
	std::unordered_map<BaseShape*, std::vector<std::tuple<BaseShape*, float,float>>> fixedConnections;
	std::unordered_map<BaseShape*, std::vector<BaseShape*>> nonFixedConnections;
	std::vector<BaseShape*> allObjects;
	std::mt19937 rng; // Random number generator

public:

	LineLink(float lineLength) : lineLength(lineLength) {
		rng.seed(std::time(nullptr));
	}

	~LineLink() {
		allObjects.clear();
	}

	void ClearLinks() {
		allObjects.clear();
		fixedConnections.clear();
		nonFixedConnections.clear();
	}

	void AddObject(BaseShape* obj) {
		if (std::find(allObjects.begin(), allObjects.end(), obj) == allObjects.end()) {//if no connections then add to the objects that need to be connected
			allObjects.push_back(obj);
		}
	}

	void MakeNewLink(BaseShape* obj1, BaseShape* obj2, int type) {
		AddObject(obj1);
		AddObject(obj2);

		if (type == 1) { // Fixed connection
			sf::Vector2f delta = obj2->GetPosition() - obj1->GetPosition();
			float thisLineLength = std::sqrt(delta.x * delta.x + delta.y * delta.y);
			float angle = std::atan2(delta.y, delta.x) * (180.0f / PI); // Convert to degrees
			fixedConnections[obj1].emplace_back(obj2, angle, thisLineLength);
			fixedConnections[obj2].emplace_back(obj1, angle + 180.0f, thisLineLength); // Add 180 degrees for reverse direction
		}
		else if (type == 2) { // Non-fixed connection
			nonFixedConnections[obj1].push_back(obj2);
			nonFixedConnections[obj2].push_back(obj1);
		}
	}

	void ConnectAll(int type) {
		for (size_t i = 0; i < allObjects.size(); i++) {
			for (size_t j = i + 1; j < allObjects.size(); j++) {
				MakeNewLink(allObjects[i], allObjects[j], type);
			}
		}
	}

	void ConnectChain(int type) {
		for (size_t i = 0; i < allObjects.size() - 1; i++) {
			MakeNewLink(allObjects[i], allObjects[i + 1], type);
		}
	}

	void ConnectStar(int type) {
		if (!allObjects.empty()) {
			for (size_t i = 1; i < allObjects.size(); i++) {
				MakeNewLink(allObjects[0], allObjects[i], type);
			}
		}
	}

	void ConnectRandom(int numConnections, int type) {
		if (allObjects.size() < 2) return; // Need at least 2 objects to make connections

		std::uniform_int_distribution<int> dist(0, allObjects.size() - 1);

		for (int i = 0; i < numConnections; ++i) {
			int index1 = dist(rng);
			int index2;
			do {
				index2 = dist(rng);
			} while (index2 == index1); // Ensure we don't connect an object to itself

			BaseShape* obj1 = allObjects[index1];
			BaseShape* obj2 = allObjects[index2];

			auto & connections = fixedConnections[obj1];
			auto it = std::find_if(connections.begin(), connections.end(),
				[obj2](const auto& tuple) {
					return std::get<0>(tuple) == obj2; // Compare only the BaseShape* part
				});

			if (it == connections.end()) {
				MakeNewLink(obj1, obj2, type);
			}
		}
	}

	void ApplyLink(BaseShape* obj1, BaseShape* obj2) {
		sf::Vector2f vector = obj2->GetPosition() - obj1->GetPosition();
		float vectorLength = std::sqrt(vector.x * vector.x + vector.y * vector.y);
		// Prevent division by zero
		if (vectorLength < 0.0001f) {
			return;
		}
		sf::Vector2f normalizedVector = vector / vectorLength;
		float deltaLength = vectorLength - lineLength;
		// Limit the maximum correction to prevent extreme movements
		float maxCorrection = 50.f; // Adjust this value as needed
		float correctionMagnitude = std::min(std::abs(deltaLength * 0.5f), maxCorrection);
		// Calculate correction vector component-wise
		sf::Vector2f correction(
			normalizedVector.x * correctionMagnitude * (deltaLength > 0 ? 1 : -1), // if deltaLength >0 then 1 else -1
			normalizedVector.y * correctionMagnitude * (deltaLength > 0 ? 1 : -1)
		);
		// Apply correction with damping
		float damping = 1; // Adjust this value between 0 and 1
		obj1->SetPosition(obj1->GetPosition() + sf::Vector2f(correction.x * damping, correction.y * damping));
		obj2->SetPosition(obj2->GetPosition() - sf::Vector2f(correction.x * damping, correction.y * damping));
		// Apply a small force instead of directly modifying velocity
		float forceMagnitude = 0.1f; // Adjust this value as needed
		sf::Vector2f force(normalizedVector.x * forceMagnitude, normalizedVector.y * forceMagnitude);
		obj1->applyForce(sf::Vector2f(-force.x, -force.y));
		obj2->applyForce(force);
	}

	void ApplyLinkWithFixedAngle(BaseShape* obj1, BaseShape* obj2, float fixedAngle, float thisLineLength) {
		// Calculate current vector between objects
		sf::Vector2f vector = obj2->GetPosition() - obj1->GetPosition();
		float currentLength = std::sqrt(vector.x * vector.x + vector.y * vector.y);

		// Prevent division by zero
		if (currentLength < 0.0001f) {
			return;
		}

		// Normalize the current vector
		sf::Vector2f normalizedVector = vector / currentLength;

		// Calculate current angle (in radians)
		float currentAngle = std::atan2(vector.y, vector.x);

		// Convert fixed angle to radians if it's in degrees
		float targetAngleRad = fixedAngle * (PI / 180.0f);

		// Calculate angle difference
		float angleDiff = targetAngleRad - currentAngle;

		// Normalize angle difference to PI or -PI
		while (angleDiff > PI) angleDiff -= 2.0f * PI;
		while (angleDiff < -PI) angleDiff += 2.0f * PI;

		// Calculate position corrections
		float lengthDiff = currentLength - thisLineLength;

		// Calculate the desired position based on both angle and length constraints
		float desiredX = obj1->GetPosition().x + thisLineLength * std::cos(targetAngleRad);
		float desiredY = obj1->GetPosition().y + thisLineLength * std::sin(targetAngleRad);
		sf::Vector2f desiredPos(desiredX, desiredY);

		// Calculate total correction vector
		sf::Vector2f correction = desiredPos - obj2->GetPosition();

		// Limit correction magnitude with a more conservative max correction
		float maxCorrection = 10.f; // Reduced from 50.f to 10.f for more stability
		float correctionLength = std::sqrt(correction.x * correction.x + correction.y * correction.y);
		if (correctionLength > maxCorrection) {
			correction *= (maxCorrection / correctionLength);
		}

		// Apply position correction with increased damping
		float positionDamping = 0.3f; // Reduced from 0.5f for more stability

		// Split correction between both objects
		sf::Vector2f obj1Correction = -correction * (positionDamping * 0.5f);
		sf::Vector2f obj2Correction = correction * (positionDamping * 0.5f);

		// Apply position corrections
		obj1->SetPosition(obj1->GetPosition() + obj1Correction);
		obj2->SetPosition(obj2->GetPosition() + obj2Correction);

		// Apply gentler rotational forces
		float angularForce = angleDiff * 0.05f; // Reduced from 0.1f
		float forceMagnitude = 0.05f; // Reduced from 0.1f

		// Calculate perpendicular force vectors
		sf::Vector2f tangent(-normalizedVector.y, normalizedVector.x);
		sf::Vector2f rotationalForce = tangent * (angularForce * forceMagnitude);

		// Apply forces with additional damping
		float forceDamping = 0.3f;
		obj1->applyForce(-rotationalForce * forceDamping);
		obj2->applyForce(rotationalForce * forceDamping);
	}

	void ApplyAllLinks() {
		// Apply non-fixed connections
		for (const auto& pair : nonFixedConnections) {
			BaseShape* obj1 = pair.first;
			for (BaseShape* obj2 : pair.second) {
				ApplyLink(obj1, obj2);
			}
		}

		// Apply fixed connections
		for (const auto& pair : fixedConnections) {
			BaseShape* obj1 = pair.first;
			for (const auto& [obj2, angle, thisLineLength] : pair.second) {
				ApplyLinkWithFixedAngle(obj1, obj2, angle, thisLineLength);
			}
		}
	}

	void Draw(sf::RenderWindow& window) {
		sf::VertexArray lines(sf::Lines);
		for (const auto& pair : fixedConnections) {
			BaseShape* obj1 = pair.first;
			for (const auto& [obj2, angle, thisLineLength] : pair.second) {
				lines.append(sf::Vertex(obj1->GetPosition(), sf::Color::White));
				lines.append(sf::Vertex(obj2->GetPosition(), sf::Color::White));
			}
		}
		for (const auto& pair : nonFixedConnections) {
			BaseShape* obj1 = pair.first;
			for (BaseShape* obj2 : pair.second) {
				lines.append(sf::Vertex(obj1->GetPosition(), sf::Color::White));
				lines.append(sf::Vertex(obj2->GetPosition(), sf::Color::White));
			}
		}
		window.draw(lines);
	}

	void Clear() {
		fixedConnections.clear();
		nonFixedConnections.clear();
		allObjects.clear();
	}
};
