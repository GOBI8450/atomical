#ifndef OPTIONS_H
#define OPTIONS_H

#pragma once
#include <SFML/Graphics.hpp>

struct Options {
	sf::VideoMode desktopSize = sf::VideoMode::getDesktopMode();
	int window_height = desktopSize.height;
	int window_width = desktopSize.width;
	bool fullscreen = false;
	float gravity = 0;
	double massLock = 0;
};

extern Options options; 

#endif