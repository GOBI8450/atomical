#pragma once
#include <fstream>
#include <vector>
#include "Circle.h"
#include "Rectangle.h"
#include "BaseShape.h"

#include <sstream>
#include <vector>
#include <string>
#include <algorithm> 

// Serializes a vector of BaseShape pointers into a single string
std::string SerializeShapes(const std::vector<BaseShape*>& shapes) {
    std::stringstream ss;
    ss << "$"; // Starting marker

    // First, write the number of shapes
    ss << shapes.size() << ";";

    // Serialize each shape
    for (const BaseShape* shape : shapes) {
        if (shape == nullptr) continue;
        ss << shape->ToString() << ";";
    }

    return ss.str();
}

// Creates appropriate shape based on type string
BaseShape* CreateShapeFromType(const std::string& type) {
    if (type == "Circle") {
        return new Circle();
    }
    else if (type == "Rectangle") {
        return new RectangleClass();
    }
    return nullptr;
}

// Splits a string by delimiter
std::vector<std::string> SplitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Extracts RGB color from string format "rgb(r,g,b)"
sf::Color ExtractColor(const std::string& colorStr) {
    size_t start = colorStr.find("(");
    size_t end = colorStr.find(")");

    if (start == std::string::npos || end == std::string::npos || start > end) {
        // Invalid format, return a default color
        std::cerr << "Invalid color format: " << colorStr << std::endl;
        return sf::Color(0, 0, 0); // Default to black
    }

    std::string values = colorStr.substr(start + 1, end - start - 1);
    auto colorComponents = SplitString(values, ',');

    if (colorComponents.size() != 3) {
        // Invalid number of components
        std::cerr << "Invalid number of color components: " << colorStr << std::endl;
        return sf::Color(0, 0, 0); // Default to black
    }

    try {
        int r = std::stoi(colorComponents[0]);
        int g = std::stoi(colorComponents[1]);
        int b = std::stoi(colorComponents[2]);

        // Clamp values to valid RGB range (0-255)
        r = std::clamp(r, 0, 255);
        g = std::clamp(g, 0, 255);
        b = std::clamp(b, 0, 255);

        return sf::Color(r, g, b);
    }
    catch (const std::invalid_argument& e) {
        // If std::stoi fails, handle the error
        std::cerr << "Invalid RGB value: " << e.what() << " in color string: " << colorStr << std::endl;
        return sf::Color(0, 0, 0); // Default to black
    }
    catch (const std::out_of_range& e) {
        // If the value is out of the valid range for integers
        std::cerr << "RGB value out of range: " << e.what() << " in color string: " << colorStr << std::endl;
        return sf::Color(0, 0, 0); // Default to black
    }
}


// Deserializes a string back into a vector of BaseShape pointers
std::vector<BaseShape*> DeserializeShapes(const std::string& serializedData) {
    std::vector<BaseShape*> shapes;

    // Check for valid start marker
    if (serializedData.empty() || serializedData[0] != '$') {
        return shapes;
    }

    // Split the entire string into tokens
    auto tokens = SplitString(serializedData.substr(1), ';');

    // First token is the number of shapes
    int numShapes = std::stoi(tokens[0]);

    // Process each shape
    for (int i = 1; i <= numShapes && i < tokens.size(); i++) {
        auto shapeData = SplitString(tokens[i], ':');

        // Create appropriate shape type
        BaseShape* shape = CreateShapeFromType(shapeData[0]);
        if (!shape) continue;

        // Parse common BaseShape properties
        int currentIndex = 1;
        int id = std::stoi(shapeData[currentIndex++]);
        sf::Color color = ExtractColor(shapeData[currentIndex++]);
        double mass = std::stod(shapeData[currentIndex++]);
        sf::Vector2f oldPos(std::stof(shapeData[currentIndex++]), std::stof(shapeData[currentIndex++]));
        sf::Vector2f accel(std::stof(shapeData[currentIndex++]), std::stof(shapeData[currentIndex++]));
        int linked = std::stoi(shapeData[currentIndex++]);

        // Set common properties
        shape->SetMass(mass);
        shape->SetOldPosition(oldPos);
        shape->SetAcceleration(accel);
        shape->SetLinked(linked);
        shape->setColor(color);

        // Handle specific shape properties
        if (Circle* circle = dynamic_cast<Circle*>(shape)) {
            float radius = std::stof(shapeData[currentIndex++]);
            sf::Vector2f velocity(std::stof(shapeData[currentIndex++]), std::stof(shapeData[currentIndex++]));
            circle->SetRadius(radius);
            circle->SetVelocity(velocity);
            circle->SetPosition(oldPos);
        }
        else if (RectangleClass* rect = dynamic_cast<RectangleClass*>(shape)) {
            float height = std::stof(shapeData[currentIndex++]);
            float width = std::stof(shapeData[currentIndex++]);
            sf::Vector2f velocity(std::stof(shapeData[currentIndex++]), std::stof(shapeData[currentIndex++]));
            rect->setSize(sf::Vector2f(width, height));
            rect->SetVelocity(velocity);
            rect->SetPosition(oldPos);
            rect->setOrigin(width / 2, height / 2);
        }

        shapes.push_back(shape);
    }

    return shapes;
}

int main() {
    // Create some shapes
    std::vector<BaseShape*> shapes;

    // Create a Circle and set its properties
    Circle* circle = new Circle();
    circle->SetMass(5.0);
    circle->SetOldPosition(sf::Vector2f(10, 20));
    circle->SetAcceleration(sf::Vector2f(0, 9.8));
    circle->SetLinked(1);
    circle->setColor(sf::Color(255, 0, 0));  // Red
    circle->SetRadius(15.0);
    circle->SetVelocity(sf::Vector2f(2.0, 3.0));
    shapes.push_back(circle);

    // Create a Rectangle and set its properties
    RectangleClass* rect = new RectangleClass();
    rect->SetMass(10.0);
    rect->SetOldPosition(sf::Vector2f(30, 40));
    rect->SetAcceleration(sf::Vector2f(1, 0));
    rect->SetLinked(2);
    rect->setColor(sf::Color(0, 255, 0));  // Green
    rect->setSize(sf::Vector2f(50, 100));
    rect->SetVelocity(sf::Vector2f(4.0, 5.0));
    shapes.push_back(rect);

    // Serialize the shapes to a string
    std::string serializedData = SerializeShapes(shapes);
    std::cout << "Serialized Data: " << serializedData << std::endl;

    // Deserialize the string back into shapes
    std::vector<BaseShape*> deserializedShapes = DeserializeShapes(serializedData);

    // Output the deserialized shapes for verification
    std::cout << "\nDeserialized Shapes:" << std::endl;
    for (BaseShape* shape : deserializedShapes) {
        // Assuming BaseShape has a method ToString() that gives a string representation of the shape
        std::cout << shape->ToString() << std::endl;
    }

    // Clean up dynamically allocated memory
    for (BaseShape* shape : shapes) {
        delete shape;
    }
    for (BaseShape* shape : deserializedShapes) {
        delete shape;
    }

    return 0;
}