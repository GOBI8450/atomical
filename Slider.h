#pragma once
#include <SFML/Graphics.hpp>

class Slider {
public:
	Slider(float x, float y, float width, float height,
		sf::Color startColor, sf::Color endColor,
		float minValue, float maxValue, bool hueSlider)
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
		if (hueSlider)
		{
			buildHueGradient();
		}
		else
		{
			gradientBar = sf::VertexArray(sf::Quads, 4);
		}
		gradientBar[0].position = sf::Vector2f(sliderX, sliderY);
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

	void handleClick(sf::Vector2f mousePos) {
		// Start dragging if the click is within the slider bar area.
			isDragging = true;
			targetHandleX = mousePos.x;
			if (targetHandleX < sliderX) targetHandleX = sliderX;
			if (targetHandleX > sliderX + barWidth) targetHandleX = sliderX + barWidth;
			float percentage = (targetHandleX - sliderX) / barWidth;
			value = minValue + percentage * (maxValue - minValue);
	}

	// Function to check if the mouse is within the slider handle area.
	bool containMouse(sf::Vector2f mousePos, float howMuchZoomed) {
		sf::FloatRect sliderBounds(sliderX / howMuchZoomed, sliderY / howMuchZoomed,  barWidth / howMuchZoomed, barHeight / howMuchZoomed);
		return sliderBounds.contains(mousePos);
	}

	// Function to handle mouse movement while dragging.
	void handleMoving(const sf::RenderWindow& window, sf::Vector2f mousePos) {
		if (isDragging) {
			targetHandleX = mousePos.x;
			if (targetHandleX < sliderX) targetHandleX = sliderX;
			if (targetHandleX > sliderX + barWidth) targetHandleX = sliderX + barWidth;
			float percentage = (targetHandleX - sliderX) / barWidth;
			value = minValue + percentage * (maxValue - minValue);
		}
	}

	// Function to handle mouse release event.
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

	// Function to draw the slider on the window.
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

	// Getters and setters for the slider value.
	float getValue() const {
		return value;
	}

	// Set the value of the slider, clamping it within the min and max range.
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

	sf::Color hueToRGB(float hue) {
		hue = fmod(hue, 360.f);
		if (hue < 0) hue += 360.f;

		float c = 1.0f;
		float x = 1.0f - fabs(fmod(hue / 60.f, 2.f) - 1.0f);

		float r = 0, g = 0, b = 0;
		if (hue < 60) { r = c; g = x; b = 0; }
		else if (hue < 120) { r = x; g = c; b = 0; }
		else if (hue < 180) { r = 0; g = c; b = x; }
		else if (hue < 240) { r = 0; g = x; b = c; }
		else if (hue < 300) { r = x; g = 0; b = c; }
		else { r = c; g = 0; b = x; }

		return sf::Color(
			static_cast<sf::Uint8>(r * 255),
			static_cast<sf::Uint8>(g * 255),
			static_cast<sf::Uint8>(b * 255)
		);
	}

	sf::Color getValueColor() {
		return hueToRGB(value); // value is in [0, 360]
	}

	void buildHueGradient() {
		const int steps = 36; // 10-degree steps over 360°
		gradientBar = sf::VertexArray(sf::Quads, steps * 4);

		for (int i = 0; i < steps; ++i) {
			float hue1 = (i * 360.0f) / steps;
			float hue2 = ((i + 1) * 360.0f) / steps;

			sf::Color color1 = hueToRGB(hue1);
			sf::Color color2 = hueToRGB(hue2);

			float x1 = sliderX + (i * barWidth) / steps;
			float x2 = sliderX + ((i + 1) * barWidth) / steps;

			// Top-left
			gradientBar[i * 4 + 0].position = sf::Vector2f(x1, sliderY);
			gradientBar[i * 4 + 0].color = color1;

			// Top-right
			gradientBar[i * 4 + 1].position = sf::Vector2f(x2, sliderY);
			gradientBar[i * 4 + 1].color = color2;

			// Bottom-right
			gradientBar[i * 4 + 2].position = sf::Vector2f(x2, sliderY + barHeight);
			gradientBar[i * 4 + 2].color = color2;

			// Bottom-left
			gradientBar[i * 4 + 3].position = sf::Vector2f(x1, sliderY + barHeight);
			gradientBar[i * 4 + 3].color = color1;
		}
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
