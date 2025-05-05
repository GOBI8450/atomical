#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <SFML/Graphics.hpp> // For sf::Color and sf::Vector2f

#include "Circle.h"
#include "Rectangle.h"
#include "BaseShape.h"

// Class for serializing and deserializing shapes
class Serialization
{
private:
    // Splits a string by a delimiter
    static std::vector<std::string> SplitString(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // Extracts RGB color from a string in the format "rgb(r,g,b)"
    static sf::Color ExtractColor(const std::string& colorStr) {
        size_t start = colorStr.find("(");
        size_t end = colorStr.find(")");

        // Validate the format of the color string
        if (start == std::string::npos || end == std::string::npos || start > end) {
            std::cerr << "Invalid color format: " << colorStr << std::endl;
            return sf::Color(0, 0, 0); // Default to black
        }

        // Extract the values inside the parentheses
        std::string values = colorStr.substr(start + 1, end - start - 1);
        auto colorComponents = SplitString(values, ',');

        // Ensure there are exactly 3 components (R, G, B)
        if (colorComponents.size() != 3) {
            std::cerr << "Invalid number of color components: " << colorStr << std::endl;
            return sf::Color(0, 0, 0); // Default to black
        }

        try {
            // Parse the RGB values and clamp them to the range [0, 255]
            int r = std::stoi(colorComponents[0]);
            int g = std::stoi(colorComponents[1]);
            int b = std::stoi(colorComponents[2]);

            r = std::clamp(r, 0, 255);
            g = std::clamp(g, 0, 255);
            b = std::clamp(b, 0, 255);

            return sf::Color(r, g, b);
        }
        catch (...) {
            std::cerr << "Error parsing RGB components in: " << colorStr << std::endl;
            return sf::Color(0, 0, 0); // Default to black
        }
    }

    // Creates a shape object based on its type
    static BaseShape* CreateShapeFromType(const std::string& type) {
        if (type == "Circle" || type == "Planet" || type == "ElectPart") {
            return new Circle();
        }
        else if (type == "Rectangle") {
            return new RectangleClass();
        }
        return nullptr; // Return nullptr if the type is unrecognized
    }

public:
    // Serializes a vector of shapes into a single string
    static std::string SerializeShapes(const std::vector<BaseShape*>& shapes) {
        std::stringstream ss;
        ss << "$" << shapes.size() << ";"; // Start marker and shape count

        // Serialize each shape into a string
        for (const BaseShape* shape : shapes) {
            if (shape) {
                ss << shape->ToString() << ";";
            }
        }

        return ss.str();
    }

    // Deserializes a string back into a vector of BaseShape pointers
    static std::vector<BaseShape*> DeserializeShapes(const std::string& serializedData) {
        std::vector<BaseShape*> shapes;

        // Check if the serialized data is empty
        if (serializedData.empty()) {
            std::cerr << "Invalid serialized data format" << std::endl;
            return shapes;
        }

        // Split the serialized data into tokens
        auto tokens = SplitString(serializedData, ';');
        if (tokens.empty()) {
            std::cerr << "No shape data found" << std::endl;
            return shapes;
        }

        try {
            // Parse the number of shapes
            int numShapes = std::stoi(tokens[0]);

            // Deserialize each shape
            for (int i = 1; i <= numShapes && i < tokens.size(); i++) {
                auto shapeData = SplitString(tokens[i], ':');
                if (shapeData.empty() || shapeData.size() != 12) {
                    std::cout << "corrupted obj" << "\n";
                    continue;
                }

                // Create a shape based on its type
                BaseShape* shape = CreateShapeFromType(shapeData[0]);
                if (!shape) {
                    std::cout << "corrupted obj - not an object" << "\n";
                    continue;
                }

                // Parse and set the shape's properties
                int currentIndex = 1;
                int id = std::stoi(shapeData[currentIndex++]);
                sf::Color color = ExtractColor(shapeData[currentIndex++]);
                double mass = std::stod(shapeData[currentIndex++]);
                sf::Vector2f pos(std::stof(shapeData[currentIndex++]), std::stof(shapeData[currentIndex++]));
                sf::Vector2f accel(std::stof(shapeData[currentIndex++]), std::stof(shapeData[currentIndex++]));
                int linked = std::stoi(shapeData[currentIndex++]);

                shape->SetMass(mass);
                shape->SetPosition(pos);
                shape->SetAcceleration(accel);
                shape->SetLinked(linked);
                shape->setColor(color);
                shape->SetID(id);

                // Handle Circle-specific properties
                if (Circle* circle = dynamic_cast<Circle*>(shape)) {
                    float radius = std::stof(shapeData[currentIndex++]);
                    sf::Vector2f velocity(std::stof(shapeData[currentIndex++]), std::stof(shapeData[currentIndex++]));
                    circle->SetRadius(radius);
                    circle->SetVelocity(velocity);
                }
                // Handle Rectangle-specific properties
                else if (RectangleClass* rect = dynamic_cast<RectangleClass*>(shape)) {
                    float height = std::stof(shapeData[currentIndex++]);
                    float width = std::stof(shapeData[currentIndex++]);
                    sf::Vector2f velocity(std::stof(shapeData[currentIndex++]), std::stof(shapeData[currentIndex++]));
                    rect->setSize(sf::Vector2f(width, height));
                    rect->SetVelocity(velocity);
                }

                shapes.push_back(shape); // Add the shape to the vector
            }
        }
        catch (...) {
            std::cerr << "Error deserializing shapes" << std::endl;
        }

        return shapes;
    }
};