#pragma once
#include <unordered_map>
#include <iostream>
#include "Circle.h"
#include "Rectangle.h"
#include <random>  // For random number generation
#include <ctime>   // For seeding with current time

class Grid {
protected:
    std::mt19937 rnd; // random variable
    int ballCount = 0;

public:
    virtual ~Grid() = default;

    virtual void InsertObj(BaseShape* obj) {}

    virtual void clear() {}

    virtual std::vector<BaseShape*> GetNerbyCellsObjects(BaseShape* obj) {
        return std::vector<BaseShape*>();
    }

    virtual int GetGridColumn(BaseShape* obj) {
        return 0;
    }

    virtual int GetGridRow(BaseShape* obj) {
        return 0;
    }

    virtual sf::Vector2f Vector2iToVector2f(sf::Vector2i pointPos) {
        return sf::Vector2f(static_cast<float>(pointPos.x), static_cast<float>(pointPos.y));
    }

    virtual BaseShape* IsInGridRadius(sf::Vector2f pointPos) {
        return nullptr;
    }

    virtual sf::RectangleShape createGridVisually(const sf::Vector2f& size, const sf::Vector2f& position, float outlineThickness, sf::Color outlineColor) {
        return sf::RectangleShape();
    }

    virtual void DrawGrids(sf::RenderWindow& window) {}
};

class GridUnorderd : public Grid {
private:
    std::unordered_map<int, std::vector<BaseShape*>> gridMap;
    std::vector<int> hashKeyVec;
    float multiplier = 2.5;

    int hashFunction(int column, int row) const {
        return column + row * 1000000007;
    }

public:
    GridUnorderd() : Grid() {};

    void InsertObj(BaseShape* obj) override {
        sf::Vector2f pos = obj->GetPosition();
        int gridColumn = GetGridColumn(obj);
        int gridRow = GetGridRow(obj);

        int hashKey = hashFunction(gridColumn, gridRow);
        hashKeyVec.push_back(hashKey);
        gridMap[hashKey].push_back(obj);
    }

    void clear() override {
        gridMap.clear();
    }

    int GetHashMapSize() {
        return gridMap.size();
    }

    std::vector<int> GetAllHashKeys() {
        return hashKeyVec;
    }

    std::vector<std::vector<BaseShape*>> GetCircelsVectorOfVectorsFromKeyVectors(std::vector<int> hashKeysVec) {
        std::vector<std::vector<BaseShape*>> vectorsOfCircleBaseVector;
        for (size_t currentKey = 0; currentKey < hashKeysVec.size(); currentKey++) {
            vectorsOfCircleBaseVector.push_back(gridMap[hashKeysVec[currentKey]]);
        }
        return vectorsOfCircleBaseVector;
    }

    std::vector<BaseShape*> GetNerbyCellsObjects(BaseShape* obj) override {
        std::vector<BaseShape*> nerbyCellsVector;
        sf::Vector2f pos = obj->GetPosition();
        int gridColumn = GetGridColumn(obj);
        int gridRow = GetGridRow(obj);

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

    sf::RectangleShape createGridVisually(const sf::Vector2f& size, const sf::Vector2f& position, float outlineThickness, sf::Color outlineColor) override {
        sf::RectangleShape rectangle(size);
        rectangle.setPosition(position);
        rectangle.setOutlineThickness(outlineThickness);
        rectangle.setOutlineColor(outlineColor);
        rectangle.setFillColor(sf::Color::Transparent);
        return rectangle;
    }

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
