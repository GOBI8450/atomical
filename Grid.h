#pragma once
#include <unordered_map>
#include <iostream>
#include "Circle.h"
#include "Rectangle.h"
#include <random>  // For random number generation
#include <ctime>   // For seeding with current time

// Base class for managing spatial partitioning of objects in a grid
class Grid {
protected:
    std::mt19937 rnd; // Random number generator
    int ballCount = 0; // Counter for the number of objects in the grid

public:
    virtual ~Grid() = default; // Virtual destructor for polymorphism

    // Inserts an object into the grid (to be overridden by derived classes)
    virtual void InsertObj(BaseShape* obj) {}

    // Clears all objects from the grid (to be overridden by derived classes)
    virtual void clear() {}

    // Retrieves nearby objects for a given object (to be overridden by derived classes)
    virtual std::vector<BaseShape*> GetNerbyCellsObjects(BaseShape* obj) {
        return std::vector<BaseShape*>();
    }

    // Gets the grid column for a given object (to be overridden by derived classes)
    virtual int GetGridColumn(BaseShape* obj) {
        return 0;
    }

    // Gets the grid row for a given object (to be overridden by derived classes)
    virtual int GetGridRow(BaseShape* obj) {
        return 0;
    }

    // Converts a 2D integer vector to a 2D float vector
    virtual sf::Vector2f Vector2iToVector2f(sf::Vector2i pointPos) {
        return sf::Vector2f(static_cast<float>(pointPos.x), static_cast<float>(pointPos.y));
    }

    // Checks if a point is within the radius of any object in the grid
    virtual BaseShape* IsInGridRadius(sf::Vector2f pointPos) {
        return nullptr;
    }

    // Creates a visual representation of a grid cell
    virtual sf::RectangleShape createGridVisually(const sf::Vector2f& size, const sf::Vector2f& position, float outlineThickness, sf::Color outlineColor) {
        return sf::RectangleShape();
    }

    // Draws the grid (to be overridden by derived classes)
    virtual void DrawGrids(sf::RenderWindow& window) {}
};

// Derived class implementing an unordered grid for spatial partitioning
class GridUnorderd : public Grid {
private:
    std::unordered_map<int, std::vector<BaseShape*>> gridMap; // Map of grid cells to objects
    std::vector<int> hashKeyVec; // Vector of hash keys for grid cells
    float multiplier = 2.5; // Multiplier for determining grid cell size

    // Hash function to generate a unique key for a grid cell
    int hashFunction(int column, int row) const {
        return column + row * 1000000007;
    }

public:
    GridUnorderd() : Grid() {}

    // Inserts an object into the grid based on its position
    void InsertObj(BaseShape* obj) override {
        sf::Vector2f pos = obj->GetPosition();
        int gridColumn = GetGridColumn(obj);
        int gridRow = GetGridRow(obj);

        int hashKey = hashFunction(gridColumn, gridRow);
        hashKeyVec.push_back(hashKey);
        gridMap[hashKey].push_back(obj);
    }

    // Clears all objects from the grid
    void clear() override {
        // Clear all stored object pointers
        gridMap.clear();
        // Free any bucket memory held by the map
        gridMap.rehash(0);
        // Clear the hash key history
        hashKeyVec.clear();
        hashKeyVec.shrink_to_fit();
    }

    // Gets the size of the grid map
    int GetHashMapSize() {
        return gridMap.size();
    }

    // Retrieves all hash keys in the grid
    std::vector<int> GetAllHashKeys() {
        return hashKeyVec;
    }

    // Retrieves vectors of objects from a list of hash keys
    std::vector<std::vector<BaseShape*>> GetCircelsVectorOfVectorsFromKeyVectors(std::vector<int> hashKeysVec) {
        std::vector<std::vector<BaseShape*>> vectorsOfCircleBaseVector;
        for (size_t currentKey = 0; currentKey < hashKeysVec.size(); currentKey++) {
            vectorsOfCircleBaseVector.push_back(gridMap[hashKeysVec[currentKey]]);
        }
        return vectorsOfCircleBaseVector;
    }

    // Retrieves objects in nearby grid cells for a given object
    std::vector<BaseShape*> GetNerbyCellsObjects(BaseShape* obj) override {
        std::vector<BaseShape*> nerbyCellsVector;
        sf::Vector2f pos = obj->GetPosition();
        int gridColumn = GetGridColumn(obj);
        int gridRow = GetGridRow(obj);

        // Iterate through neighboring cells
        for (short int otherRow = -1; otherRow <= 1; otherRow++) {
            for (short int otherColumn = -1; otherColumn <= 1; otherColumn++) {
                int hashKey = hashFunction(gridColumn + otherColumn, gridRow + otherRow);
                if (gridMap.find(hashKey) != gridMap.end()) {
                    nerbyCellsVector.insert(nerbyCellsVector.end(), gridMap[hashKey].begin(), gridMap[hashKey].end());
                }
            }
        }
        return nerbyCellsVector;
    }

    // Gets the grid column for a given object based on its position
    int GetGridColumn(BaseShape* obj) override {
        int gridColumn;
        sf::Vector2f pos = obj->GetPosition();
        if (Circle* circle = dynamic_cast<Circle*>(obj)) {
            gridColumn = static_cast<int>(pos.x / (circle->getRadius() * multiplier));
        }
        else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
            gridColumn = static_cast<int>(pos.x / (rectangle->GetWidth() * multiplier));
        }
        return gridColumn;
    }

    // Gets the grid row for a given object based on its position
    int GetGridRow(BaseShape* obj) override {
        int gridRow;
        sf::Vector2f pos = obj->GetPosition();
        if (Circle* circle = dynamic_cast<Circle*>(obj)) {
            gridRow = static_cast<int>(pos.y / (circle->getRadius() * multiplier));
        }
        else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
            gridRow = static_cast<int>(pos.y / (rectangle->GetHeight() * multiplier));
        }
        return gridRow;
    }

    // Checks if a point is within the radius of any object in a specific grid cell
    BaseShape* IsInSpecificGridRadius(sf::Vector2f pointPosf, int hashKey) {
        std::vector<BaseShape*> objVec = gridMap[hashKey];
        for (auto& obj : objVec) {
            sf::Vector2f objPos = obj->GetPosition();
            double distance = std::sqrt(std::pow(objPos.x - pointPosf.x, 2) + std::pow(objPos.y - pointPosf.y, 2));
            if (Circle* circle = dynamic_cast<Circle*>(obj)) {
                if (distance <= circle->getRadius()) {
                    return obj;
                }
            }
            else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
                if (rectangle->IsCollision(pointPosf)) {
                    return obj;
                }
            }
        }
        return nullptr;
    }

    // Checks if a point is within the radius of any object in the grid
    BaseShape* IsInGridRadius(sf::Vector2f pointPos) override {
        BaseShape* shapePointer;
        for (auto& keyAndCircle : gridMap) {
            int hashKey = keyAndCircle.first;
            shapePointer = IsInSpecificGridRadius(pointPos, hashKey);
            if (shapePointer != nullptr) {
                return shapePointer;
            }
        }
        return nullptr;
    }

    // Creates a visual representation of a grid cell
    sf::RectangleShape createGridVisually(const sf::Vector2f& size, const sf::Vector2f& position, float outlineThickness, sf::Color outlineColor) override {
        sf::RectangleShape rectangle(size);
        rectangle.setPosition(position);
        rectangle.setOutlineThickness(outlineThickness);
        rectangle.setOutlineColor(outlineColor);
        rectangle.setFillColor(sf::Color::Transparent);
        return rectangle;
    }

    // Draws the grid cells and their boundaries
    void DrawGrids(sf::RenderWindow& window) override {
        for (auto& keyAndObject : gridMap) {
            int hashKey = keyAndObject.first;
            std::vector<BaseShape*> objVec = keyAndObject.second;
            BaseShape* obj = objVec.front();
            sf::RectangleShape gridRect;
            if (Circle* circle = dynamic_cast<Circle*>(obj)) {
                gridRect = createGridVisually(sf::Vector2f(circle->GetRadius() * 2, circle->GetRadius() * 2), obj->GetPosition(), 3.0, sf::Color(255, 0, 0));
                gridRect.setOrigin(circle->GetRadius(), circle->GetRadius());
            }
            else if (RectangleClass* rectangle = dynamic_cast<RectangleClass*>(obj)) {
                gridRect = createGridVisually(sf::Vector2f(rectangle->getSize().x, rectangle->getSize().y), obj->GetPosition(), 3.0, sf::Color(255, 0, 0));
            }
            window.draw(gridRect);
        }
    }
};