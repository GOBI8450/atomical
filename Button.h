#pragma once
#include <SFML/Graphics.hpp>

// Class representing a button in the UI, inheriting from sf::RectangleShape
class Button : public sf::RectangleShape
{
private:
    float width; // Width of the button
    float height; // Height of the button
    std::string name; // Name or identifier of the button
    bool isHovered = false; // Flag to track if the button is being hovered over
    const float scaleAmount = 1.05f; // Scale factor for hover effect
    sf::Vector2f originalScale; // Original scale of the button

public:
    // Constructor: Initializes the button with width, height, position, and name
    Button(float width, float height, sf::Vector2f pos, std::string name)
        : width(width), height(height), name(name)
    {
        setSize(sf::Vector2f(width, height)); // Set the size of the button
        setOrigin(width / 2, height / 2); // Set the origin to the center
        setPosition(pos); // Set the position of the button
        originalScale = getScale(); // Store the original scale
    }

    // Constructor: Initializes the button with width, height, and position (no name)
    Button(float width, float height, sf::Vector2f pos)
        : width(width), height(height)
    {
        setSize(sf::Vector2f(width, height)); // Set the size of the button
        setOrigin(width / 2, height / 2); // Set the origin to the center
        setPosition(pos); // Set the position of the button
        originalScale = getScale(); // Store the original scale
        name = ""; // Default name is an empty string
    }

    // Sets the texture of the button
    void SetTexture(const sf::Texture& newTexture) {
        setTexture(&newTexture);
    }

    // Checks if the mouse position is within the button's bounds
    bool IsInRadius(sf::Vector2f mousePos) {
        sf::Vector2f pos = getPosition(); // Get the button's position
        float scaledWidth = width * getScale().x; // Calculate the scaled width
        float scaledHeight = height * getScale().y; // Calculate the scaled height

        // Check if the mouse position is within the button's bounds
        return mousePos.x >= pos.x - scaledWidth / 2 &&
            mousePos.x <= pos.x + scaledWidth / 2 &&
            mousePos.y >= pos.y - scaledHeight / 2 &&
            mousePos.y <= pos.y + scaledHeight / 2;
    }

    // Handles hover behavior for the button
    bool MouseHover(sf::Vector2f mousePos, bool& hovering) {
        bool wasHovered = isHovered; // Store the previous hover state
        isHovered = IsInRadius(mousePos); // Check if the mouse is hovering over the button

        // Only change scale if the hover state has changed
        if (isHovered != wasHovered) {
            if (isHovered) {
                // Scale up the button when hovered
                setScale(originalScale.x * scaleAmount, originalScale.y * scaleAmount);
                hovering = true;
            }
            else {
                // Reset to the original scale when not hovered
                setScale(originalScale);
                hovering = false;
            }
        }

        return isHovered;
    }

    // Returns the name of the button
    std::string GetName() {
        return name;
    }

    // Updates the button's hover state based on the mouse position
    void Update(sf::Vector2f mousePos, bool& hovering) {
        MouseHover(mousePos, hovering);
    }

    // Sets the scale of the button
    void SetScale(float scale) {
        originalScale = sf::Vector2f(scale, scale); // Update the original scale
        setScale(originalScale); // Apply the new scale
    }

    // Draws the button on the window
    void draw(sf::RenderWindow& window) {
        window.draw(*this);
    }
};