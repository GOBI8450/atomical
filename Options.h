#ifndef OPTIONS_H
#define OPTIONS_H

#pragma once
#include <SFML/Graphics.hpp>

struct Options {
	sf::VideoMode desktopSize = sf::VideoMode::getDesktopMode();
	int window_height = 1080;
	int window_width = 1920;
	bool fullscreen = false;
	float gravity = 9.8;
	double massLock = 0;
};

extern Options options; 

#endif