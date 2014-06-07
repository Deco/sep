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
	ac.init();

	cv::Vec2d pos;
	pos = ac.getCurrentPosition();
	std::cout << pos[0] << ", " << pos[1] << std::endl;

	

	sf::Clock clock;

	//double time = clock.getElapsedTime().asSeconds();
	//std::cout << "Time elapsed: " << time << std::endl;

	
	bool isAdding = false;
	std::vector<ActuatorMoveOrder> ordersToAddList;

	sf::RenderWindow window(sf::VideoMode(640,480), "test");
	while (window.isOpen())
	{

		ac.update();

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::MouseButtonPressed)
			{
				if(isAdding) {
		    		ActuatorMoveOrder order;
		    		order.posDeg = cv::Vec2d(-150+300.0*((double)event.mouseButton.x)/640.0, -90.0+180*((double)event.mouseButton.y)/480.0);
		    		order.duration = 2;
		    		ordersToAddList.push_back(order);
				} else {
					ac.stop();
					ActuatorMoveOrder order;
				    order.posDeg = cv::Vec2d(-150+300.0*((double)event.mouseButton.x)/640.0, -90.0+180*((double)event.mouseButton.y)/480.0);
				    order.duration = 2;
					ac.queueMove(order);
				}
			}
			else if (event.type == sf::Event::KeyPressed)
			{
			    if (event.key.code == sf::Keyboard::Escape)
			    {
			    	window.close();
		    	}
		    	else if(event.key.code == sf::Keyboard::A) {
		    		isAdding = true;
		    	}
		    	else if(event.key.code == sf::Keyboard::S) {
		    		ac.queueMoves(ordersToAddList);
		    		ordersToAddList.clear();
		    		isAdding = false;
		    	}
		    	else if(event.key.code == sf::Keyboard::T) {
		    		ac.stop();
		    	}
		    }
			else if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}
		window.clear();

		cv::Vec2d pos;
		pos = ac.getCurrentPosition();
		//std::cout << pos[0] << ", " << pos[1] << std::endl;

		auto rectPos = sf::Vector2f((pos[0]+150.0)/300.0*640.0, (pos[1]+90.0)/180.0*480.0);
		
		sf::RectangleShape rect;
		rect.setPosition(rectPos);
		rect.setSize(sf::Vector2f(20, 20));
		rect.setFillColor(sf::Color::White);
		window.draw(rect);

		sf::Vector2f prevPos = rectPos;
		for(int i = 0; i < ordersToAddList.size(); i++) {
			ActuatorMoveOrder &order = ordersToAddList[i];

			sf::Vector2f newPos = sf::Vector2f((order.posDeg[0]+150.0)/300.0*640.0, (order.posDeg[1]+90.0)/180.0*480.0);

			sf::Vertex linePoints[] = {prevPos, newPos};
			window.draw(linePoints, 2, sf::Lines);

			prevPos = newPos;
		}

		window.display();
	}


}