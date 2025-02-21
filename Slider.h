#pragma once
#include <SFML/Graphics.hpp>

class Slider {
public:
	Slider(float x, float y, float width, float height,
		sf::Color startColor, sf::Color endColor,
		float minValue, float maxValue)
		: sliderX(x), sliderY(y), barWidth(width), barHeight(height),
		minValue(minValue), maxValue(maxValue), value(minValue), isDragging(false),
		smoothingFactor(0.2f)
	{
		// Set up the base bar (the unfilled track) in black.
		baseBar.setPosition(sliderX, sliderY);
		baseBar.setSize(sf::Vector2f(barWidth, barHeight));
		baseBar.setFillColor(sf::Color::Black);

		// Set up the gradient fill bar.
		// Initially, the filled portion is 0 width (i.e. no fill).
		gradientBar = sf::VertexArray(sf::Quads, 4);
		gradientBar[0].position = sf::Vector2f(sliderX, sliderY);
		gradientBar[1].position = sf::Vector2f(sliderX, sliderY);
		gradientBar[2].position = sf::Vector2f(sliderX, sliderY + barHeight);
		gradientBar[3].position = sf::Vector2f(sliderX, sliderY + barHeight);
		gradientBar[0].color = startColor;
		gradientBar[1].color = endColor;
		gradientBar[2].color = endColor;
		gradientBar[3].color = startColor;

		// Create a neat border around the slider bar.
		barOutline.setPosition(sliderX, sliderY);
		barOutline.setSize(sf::Vector2f(barWidth, barHeight));
		barOutline.setFillColor(sf::Color::Transparent);
		barOutline.setOutlineThickness(2);
		barOutline.setOutlineColor(sf::Color(200, 200, 200));

		// Initialize the slider handle as a circle.
		float handleRadius = barHeight; // Proportional to bar height.
		handle.setRadius(handleRadius);
		handle.setOrigin(handleRadius, handleRadius);
		handle.setFillColor(sf::Color::White);
		handle.setOutlineThickness(2);
		handle.setOutlineColor(sf::Color(150, 150, 150));

		// Create a drop shadow for the handle.
		handleShadow = handle;
		handleShadow.setFillColor(sf::Color(0, 0, 0, 100)); // Semi-transparent shadow.

		// Set initial handle positions.
		targetHandleX = sliderX; // Corresponds to min value.
		currentHandleX = targetHandleX;
	}

	void handleClick(const sf::RenderWindow& window, sf::Vector2f mousePos) {
		// Start dragging if the click is within the slider bar area.
			isDragging = true;
			targetHandleX = mousePos.x;
			if (targetHandleX < sliderX) targetHandleX = sliderX;
			if (targetHandleX > sliderX + barWidth) targetHandleX = sliderX + barWidth;
			float percentage = (targetHandleX - sliderX) / barWidth;
			value = minValue + percentage * (maxValue - minValue);
	}

	bool containMouse(sf::Vector2f mousePos) {
		sf::FloatRect sliderBounds(sliderX, sliderY, barWidth, barHeight);
		return sliderBounds.contains(mousePos);
	}

	void handleMoving(const sf::RenderWindow& window, sf::Vector2f mousePos) {
		if (isDragging) {
			targetHandleX = mousePos.x;
			if (targetHandleX < sliderX) targetHandleX = sliderX;
			if (targetHandleX > sliderX + barWidth) targetHandleX = sliderX + barWidth;
			float percentage = (targetHandleX - sliderX) / barWidth;
			value = minValue + percentage * (maxValue - minValue);
		}
	}

	void handleRelase() {
		isDragging = false;
	}

	void update(float dt) {
		// Smoothly interpolate currentHandleX toward targetHandleX.
		currentHandleX += smoothingFactor * (targetHandleX - currentHandleX);

		// Ensure the value updates smoothly along with the handle.
		float percentage = (currentHandleX - sliderX) / barWidth;
		value = minValue + percentage * (maxValue - minValue);

		// Update the gradient fill bar's vertices.
		float fillWidth = currentHandleX - sliderX;
		if (fillWidth < 0) fillWidth = 0;
		gradientBar[0].position = sf::Vector2f(sliderX, sliderY);
		gradientBar[1].position = sf::Vector2f(sliderX + fillWidth, sliderY);
		gradientBar[2].position = sf::Vector2f(sliderX + fillWidth, sliderY + barHeight);
		gradientBar[3].position = sf::Vector2f(sliderX, sliderY + barHeight);

		// Update positions for the handle and its shadow.
		float handleY = sliderY + barHeight / 2;
		handle.setPosition(currentHandleX, handleY);
		handleShadow.setPosition(currentHandleX + 3, handleY + 3);
	}


	void draw(sf::RenderWindow& window) {
		// Draw the base (black) bar.
		window.draw(baseBar);
		// Draw the gradient-filled portion.
		window.draw(gradientBar);
		// Draw the border around the slider.
		window.draw(barOutline);
		// Draw the handle's shadow and then the handle.
		window.draw(handle);
	}

	float getValue() const {
		return value;
	}

	void setValue(float newValue) {
		// Clamp value within the min and max range
		if (newValue < minValue) newValue = minValue;
		if (newValue > maxValue) newValue = maxValue;

		// Set the value
		value = newValue;

		// Move the handle to match the new value
		float percentage = (value - minValue) / (maxValue - minValue);
		targetHandleX = sliderX + percentage * barWidth;
		currentHandleX = targetHandleX; // Instantly update the position
	}

	void updateHandleTarget() {
		float percentage = (value - minValue) / (maxValue - minValue);
		targetHandleX = sliderX + percentage * barWidth;
	}

private:
	// Slider bar representation.
	sf::VertexArray gradientBar;
	sf::RectangleShape baseBar;
	sf::RectangleShape barOutline;

	// Slider handle representation.
	sf::CircleShape handle;
	sf::CircleShape handleShadow;
	float currentHandleX; // Animated x-position.
	float targetHandleX;  // Target x-position based on input.

	float minValue, maxValue, value;
	float barWidth, barHeight;
	float sliderX, sliderY;
	bool isDragging;
	float smoothingFactor; // Controls easing speed.
};
