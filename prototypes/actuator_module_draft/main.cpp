#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <algorithm>
#include <assert.h>
#include <sstream>
#include "ActuatorController.h"
#include "Dynamixel/DynamixelComm.h"

int main()
{
	ActuatorController ac("/dev/tty.usbserial-A9S3VTXD");
	cv::Vec2d pos;
	ac.getCurrentPosition(pos);
	std::cout << pos[0] << ", " << pos[1] << std::endl;

	cv::Vec2d goalPos(300, 0);
	ac.move(goalPos, 10.0);
	sf::Clock clock;
	
	

	sf::RenderWindow window(sf::VideoMode(30,30), "test");
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::MouseButtonPressed)
			{
				ac.stop();
				std::cout << "Test" << std::endl;
				double time = clock.getElapsedTime().asSeconds();
				std::cout << time << std::endl;
				window.close();
			}
			else if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}
	}
	


}