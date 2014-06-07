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
	// Initialises Actuator Controller
	ActuatorController ac("/dev/tty.usbserial-A9S3VTXD");

	// Initialises Actuator Thread
	ac.init();

	cv::Vec2d pos;
	pos = ac.getCurrentPosition();
	std::cout << pos[0] << ", " << pos[1] << std::endl;

	sf::Clock clock;
	
	// Currently, not in adding mode.
	bool isAdding = false;
	// Empty list
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
				if(isAdding) { // In adding mode
		    		ActuatorMoveOrder order;
		    		//Calculates the mouse position on the window,
		    		order.posDeg = cv::Vec2d(-150+300.0*((double)event.mouseButton.x)/640.0, -90.0+180*((double)event.mouseButton.y)/480.0);
		    		order.duration = 2;
		    		// Push to ordersToAddList
		    		ordersToAddList.push_back(order);
				} else { // Send move order once at a time
					// Stops current move command
					ac.stop();
					ActuatorMoveOrder order;
				    order.posDeg = cv::Vec2d(-150+300.0*((double)event.mouseButton.x)/640.0, -90.0+180*((double)event.mouseButton.y)/480.0);
				    order.duration = 2;
				    // Sends move order to the ActuatorController
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
		    		// When A is pressed, window is in adding mode
		    		isAdding = true;
		    	}
		    	else if(event.key.code == sf::Keyboard::S) {
		    		// When S is pressed, send all orders in orderToAddList to the moveQueue 
		    		ac.queueMoves(ordersToAddList);
		    		// Clears all orders in the ordersToAddList
		    		ordersToAddList.clear();
		    		// Exit adding mode
		    		isAdding = false;
		    	}
		    	else if(event.key.code == sf::Keyboard::T) {
		    		// Stops current command and clears all orders in queue
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

		//Rectangle displayed is the current position of the servos
		auto rectPos = sf::Vector2f((pos[0]+150.0)/300.0*640.0, (pos[1]+90.0)/180.0*480.0);
		
		sf::RectangleShape rect;
		rect.setPosition(rectPos);
		rect.setSize(sf::Vector2f(20, 20));
		rect.setFillColor(sf::Color::White);
		window.draw(rect);

		sf::Vector2f prevPos = rectPos;


		// Draw lines on window to display movement path of the servos
		for(int i = 0; i < ordersToAddList.size(); i++) {
			ActuatorMoveOrder &order = ordersToAddList[i];
			//Adds new orders to the orderToAddList
			sf::Vector2f newPos = sf::Vector2f((order.posDeg[0]+150.0)/300.0*640.0, (order.posDeg[1]+90.0)/180.0*480.0);

			sf::Vertex linePoints[] = {prevPos, newPos};
			window.draw(linePoints, 2, sf::Lines);

			prevPos = newPos;
		}

		window.display();
	}


}