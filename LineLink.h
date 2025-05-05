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
#define PI 3.14159265358979323846 // Definition of Pi for angle calculations

// Class to manage connections (links) between shapes
class LineLink
{
private:
    float lineLength; // Default length of the links
    std::unordered_map<BaseShape*, std::vector<std::tuple<BaseShape*, float, float>>> fixedConnections; // Map of fixed connections with angle and length
    std::unordered_map<BaseShape*, std::vector<BaseShape*>> nonFixedConnections; // Map of non-fixed connections
    std::vector<BaseShape*> allObjects; // List of all objects to be connected
    std::mt19937 rng; // Random number generator for random connections

public:
    // Constructor: Initializes the LineLink with a default line length
    LineLink(float lineLength) : lineLength(lineLength) {
        rng.seed(std::time(nullptr)); // Seed the random number generator
    }

    // Destructor: Clears all objects
    ~LineLink() {
        allObjects.clear();
    }

    // Clears all connections and objects
    void ClearLinks() {
        allObjects.clear();
        fixedConnections.clear();
        nonFixedConnections.clear();
    }

    // Adds an object to the list of objects to be connected
    void AddObject(BaseShape* obj) {
        if (std::find(allObjects.begin(), allObjects.end(), obj) == allObjects.end()) {
            allObjects.push_back(obj);
        }
    }

    // Creates a new link between two objects
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

    // Connects all objects to each other
    void ConnectAll(int type) {
        for (size_t i = 0; i < allObjects.size(); i++) {
            for (size_t j = i + 1; j < allObjects.size(); j++) {
                MakeNewLink(allObjects[i], allObjects[j], type);
            }
        }
    }

    // Connects objects in a chain
    void ConnectChain(int type) {
        for (size_t i = 0; i < allObjects.size() - 1; i++) {
            MakeNewLink(allObjects[i], allObjects[i + 1], type);
        }
    }

    // Connects all objects to a central object (star topology)
    void ConnectStar(int type) {
        if (!allObjects.empty()) {
            for (size_t i = 1; i < allObjects.size(); i++) {
                MakeNewLink(allObjects[0], allObjects[i], type);
            }
        }
    }

    // Creates random connections between objects
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

            auto& connections = fixedConnections[obj1];
            auto it = std::find_if(connections.begin(), connections.end(),
                [obj2](const auto& tuple) {
                    return std::get<0>(tuple) == obj2;
                });

            if (it == connections.end()) {
                MakeNewLink(obj1, obj2, type);
            }
        }
    }

    // Applies a non-fixed link between two objects
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
        float maxCorrection = 50.f;
        float correctionMagnitude = std::min(std::abs(deltaLength * 0.5f), maxCorrection);

        // Calculate correction vector
        sf::Vector2f correction(
            normalizedVector.x * correctionMagnitude * (deltaLength > 0 ? 1 : -1),
            normalizedVector.y * correctionMagnitude * (deltaLength > 0 ? 1 : -1)
        );

        // Apply correction with damping
        float damping = 1;
        obj1->SetPosition(obj1->GetPosition() + sf::Vector2f(correction.x * damping, correction.y * damping));
        obj2->SetPosition(obj2->GetPosition() - sf::Vector2f(correction.x * damping, correction.y * damping));

        // Apply a small force instead of directly modifying velocity
        float forceMagnitude = 0.1f;
        sf::Vector2f force(normalizedVector.x * forceMagnitude, normalizedVector.y * forceMagnitude);
        obj1->addForce(sf::Vector2f(-force.x, -force.y));
        obj2->addForce(force);
    }

    // Applies a fixed link with a specific angle and length
    void ApplyLinkWithFixedAngle(BaseShape* obj1, BaseShape* obj2, float fixedAngle, float thisLineLength) {
        sf::Vector2f vector = obj2->GetPosition() - obj1->GetPosition();
        float currentLength = std::sqrt(vector.x * vector.x + vector.y * vector.y);

        // Prevent division by zero
        if (currentLength < 0.0001f) {
            return;
        }

        sf::Vector2f normalizedVector = vector / currentLength;
        float currentAngle = std::atan2(vector.y, vector.x);
        float targetAngleRad = fixedAngle * (PI / 180.0f);
        float angleDiff = targetAngleRad - currentAngle;

        // Normalize angle difference to [-PI, PI]
        while (angleDiff > PI) angleDiff -= 2.0f * PI;
        while (angleDiff < -PI) angleDiff += 2.0f * PI;

        float lengthDiff = currentLength - thisLineLength;

        // Calculate the desired position based on angle and length
        float desiredX = obj1->GetPosition().x + thisLineLength * std::cos(targetAngleRad);
        float desiredY = obj1->GetPosition().y + thisLineLength * std::sin(targetAngleRad);
        sf::Vector2f desiredPos(desiredX, desiredY);

        sf::Vector2f correction = desiredPos - obj2->GetPosition();

        // Limit correction magnitude
        float maxCorrection = 10.f;
        float correctionLength = std::sqrt(correction.x * correction.x + correction.y * correction.y);
        if (correctionLength > maxCorrection) {
            correction *= (maxCorrection / correctionLength);
        }

        float positionDamping = 0.3f;

        // Split correction between both objects
        sf::Vector2f obj1Correction = -correction * (positionDamping * 0.5f);
        sf::Vector2f obj2Correction = correction * (positionDamping * 0.5f);

        obj1->SetPosition(obj1->GetPosition() + obj1Correction);
        obj2->SetPosition(obj2->GetPosition() + obj2Correction);

        // Apply rotational forces
        float angularForce = angleDiff * 0.05f;
        float forceMagnitude = 0.05f;

        sf::Vector2f tangent(-normalizedVector.y, normalizedVector.x);
        sf::Vector2f rotationalForce = tangent * (angularForce * forceMagnitude);

        float forceDamping = 0.3f;
        obj1->addForce(-rotationalForce * forceDamping);
        obj2->addForce(rotationalForce * forceDamping);
    }

    // Applies all links (fixed and non-fixed)
    void ApplyAllLinks() {
        for (const auto& pair : nonFixedConnections) {
            BaseShape* obj1 = pair.first;
            for (BaseShape* obj2 : pair.second) {
                ApplyLink(obj1, obj2);
            }
        }

        for (const auto& pair : fixedConnections) {
            BaseShape* obj1 = pair.first;
            for (const auto& [obj2, angle, thisLineLength] : pair.second) {
                ApplyLinkWithFixedAngle(obj1, obj2, angle, thisLineLength);
            }
        }
    }

    // Sets the default line length for links
    void SetLineLength(float newLineLength) {
        lineLength = newLineLength;
    }

    // Draws all links on the window
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

    // Clears all connections and objects
    void Clear() {
        fixedConnections.clear();
        nonFixedConnections.clear();
        allObjects.clear();
    }
};