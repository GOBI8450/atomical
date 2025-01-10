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

        if (start == std::string::npos || end == std::string::npos || start > end) {
            std::cerr << "Invalid color format: " << colorStr << std::endl;
            return sf::Color(0, 0, 0); // Default to black
        }

        std::string values = colorStr.substr(start + 1, end - start - 1);
        auto colorComponents = SplitString(values, ',');

        if (colorComponents.size() != 3) {
            std::cerr << "Invalid number of color components: " << colorStr << std::endl;
            return sf::Color(0, 0, 0); // Default to black
        }

        try {
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
        return nullptr;
    }
public:

    // Serializes a vector of shapes into a single string
    static std::string SerializeShapes(const std::vector<BaseShape*>& shapes) {
        std::stringstream ss;
        ss << "$" << shapes.size() << ";"; // Start marker and shape count

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

        if (serializedData.empty()) {
            std::cerr << "Invalid serialized data format" << std::endl;
            return shapes;
        }

        auto tokens = SplitString(serializedData, ';');
        if (tokens.empty()) {
            std::cerr << "No shape data found" << std::endl;
            return shapes;
        }

        try {
            int numShapes = std::stoi(tokens[0]);

            for (int i = 1; i <= numShapes && i < tokens.size(); i++) {
                auto shapeData = SplitString(tokens[i], ':');
                if (shapeData.empty() || shapeData.size() != 12) {
                    std::cout << "corrupted obj" << "\n";
                    continue;
                }
                

                BaseShape* shape = CreateShapeFromType(shapeData[0]);
                if (!shape) {
                    std::cout << "corrupted obj - not an object" << "\n";
                    continue;
                }

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

                if (Circle* circle = dynamic_cast<Circle*>(shape)) {
                    float radius = std::stof(shapeData[currentIndex++]);
                    sf::Vector2f velocity(std::stof(shapeData[currentIndex++]), std::stof(shapeData[currentIndex++]));
                    circle->SetRadius(radius);
                    circle->SetVelocity(velocity);
                }
                else if (RectangleClass* rect = dynamic_cast<RectangleClass*>(shape)) {
                    float height = std::stof(shapeData[currentIndex++]);
                    float width = std::stof(shapeData[currentIndex++]);
                    sf::Vector2f velocity(std::stof(shapeData[currentIndex++]), std::stof(shapeData[currentIndex++]));
                    rect->setSize(sf::Vector2f(width, height));
                    rect->SetVelocity(velocity);
                }

                shapes.push_back(shape);
            }
        }
        catch (...) {
            std::cerr << "Error deserializing shapes" << std::endl;
        }

        return shapes;
    }
};

//int main() {
//    // Create shapes
//    std::vector<BaseShape*> shapes;
//
//    Circle* circle = new Circle();
//    circle->SetMass(5.0);
//    circle->SetOldPosition(sf::Vector2f(10, 20));
//    circle->SetAcceleration(sf::Vector2f(0, 9.8));
//    circle->SetLinked(1);
//    circle->setColor(sf::Color(255, 33, 0)); // Red
//    circle->SetRadius(15.0);
//    circle->SetVelocity(sf::Vector2f(2.0, 3.0));
//    shapes.push_back(circle);
//
//    RectangleClass* rect = new RectangleClass();
//    rect->SetMass(10.0);
//    rect->SetOldPosition(sf::Vector2f(30, 40));
//    rect->SetAcceleration(sf::Vector2f(1, 0));
//    rect->SetLinked(2);
//    rect->setColor(sf::Color(0, 255, 1)); // Green
//    rect->setSize(sf::Vector2f(50, 100));
//    rect->SetVelocity(sf::Vector2f(4.0, 5.0));
//    shapes.push_back(rect);
//
//    // Serialize shapes
//    std::string serializedData = Serialization::SerializeShapes(shapes);
//    std::cout << "Serialized Data: " << serializedData << std::endl;
//
//    // Deserialize shapes
//    std::vector<BaseShape*> deserializedShapes = Serialization::DeserializeShapes(serializedData.substr(1));//cause I changed the structure that the server checks the signs
//
//    // Verify deserialized shapes
//    std::cout << "\nDeserialized Shapes:" << std::endl;
//    for (BaseShape* shape : deserializedShapes) {
//        if (shape) {
//            std::cout << shape->ToString() << std::endl;
//        }
//    }
//
//    // Cleanup
//    for (BaseShape* shape : shapes) {
//        delete shape;
//    }
//    for (BaseShape* shape : deserializedShapes) {
//        delete shape;
//    }
//
//    return 0;
//}
