#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <algorithm>
#include <assert.h>
#include <sstream>
#include "Director.h"
#include <SFML/Window/Mouse.hpp>

int main()
{
	//ActuatorController ac("/dev/tty.usbserial-A9S3VTXD");
	//ac.init();

	sf::Clock clock;
	

	sf::RenderWindow window(sf::VideoMode(640,480), "test");
	bool isAdding = false;
	bool area_selected = false;

	Director director;
	std::vector<cv::Vec2d> pointsList;
	std::vector<cv::Vec2d> area_points;
	std::vector<cv::Vec2d> selected_points;
	std::vector<ActuatorMoveOrder> ordersList;
	while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
        	sf::Time time1 = clock.getElapsedTime();
            director.update();
            if (event.type == sf::Event::MouseButtonPressed) {
            	if(isAdding) {
            		cv::Vec2d point = cv::Vec2d(-150+300.0*((double)event.mouseButton.x)/640.0, -90.0+180*((double)event.mouseButton.y)/480.0);
            		std::cout << point[0] << ',' << point[1] << std::endl;
            		pointsList.push_back(point);

            		cv::Vec2d mouse_pos = {(double)event.mouseButton.x, (double)event.mouseButton.y};
            		selected_points.push_back(mouse_pos); 
            	}
            	else {
            		cv::Vec2d point = cv::Vec2d(-150+300.0*((double)event.mouseButton.x)/640.0, -90.0+180*((double)event.mouseButton.y)/480.0);
            		std::cout << point[0] << ',' << point[1] << std::endl;
            		ActuatorMoveOrder order;
            		order.posDeg = point;
            		order.duration = 2;
            		director.performManualPan(order);
            		std::cout << time1.asSeconds() << std::endl;

            	}
            }
            else if (event.type == sf::Event::KeyPressed) {
            	if (event.key.code == sf::Keyboard::Escape) {
			    	window.close();
		    	}
		    	else if(event.key.code == sf::Keyboard::A) {
		    		// When A is pressed, window is in adding mode
		    		isAdding = true;
		    	}
		    	else if(event.key.code == sf::Keyboard::S) {
		    		//Draw box, Scan region
		    		area_points = director.performRegionScan(pointsList);
		    		pointsList.clear();
		    		isAdding = false;
		    		area_selected = true;		   
		    	}
		    	else if(event.key.code == sf::Keyboard::D) {
		    		//FullScan
		    		ordersList = director.performFullScan();

		    	}
		    	else if(event.key.code == sf::Keyboard::C) {
		    		area_points.clear();
		    		selected_points.clear();
		    		ordersList.clear();
		    		isAdding = false;
		    		area_selected = false;
		    		director.haltMovement();
		    	}

            }
            else if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
        window.clear();

        //Acuator Position
        cv::Vec2d pos;
		pos = director.getCurrentPositionAC();
		//std::cout << pos << std::endl;
		auto actPos = sf::Vector2f((pos[0]+150.0)/300.0*640.0, (pos[1]+90.0)/180.0*480.0);
		sf::CircleShape actuator;
		actuator.setRadius(2);
		actuator.setFillColor(sf::Color::Green);		
		actuator.setPosition(actPos);
		window.draw(actuator);


		sf::Vector2f prevPos;
		//Actuator move path - create lines
		for(int j = 0; j < ordersList.size(); j++) {
			ActuatorMoveOrder &order = ordersList[j];
			//Adds new orders to the orderToAddList
			if(j != 0) {
				sf::Vector2f newPos = sf::Vector2f((order.posDeg[0]+150.0)/300.0*640.0, (order.posDeg[1]+90.0)/180.0*480.0);

				sf::Vertex linePoints[] = {prevPos, newPos};
				window.draw(linePoints, 2, sf::Lines);
				//std::cout<< prevPos.x << std::endl;
				prevPos = newPos;
			}
			else {
				prevPos = sf::Vector2f((order.posDeg[0]+150.0)/300.0*640.0, (order.posDeg[1]+90.0)/180.0*480.0);
			}
		}

        //Draws circles for the selected points
        for(int i = 0; i < selected_points.size(); i++) {
	        sf::CircleShape circle;
			circle.setRadius(1);
			circle.setFillColor(sf::Color::Red);
			circle.setOutlineColor(sf::Color::Red);
			circle.setOutlineThickness(1);
			circle.setPosition((selected_points[i])[0], (selected_points[i])[1]);
			window.draw(circle);
		}

		//Draw rectangle around selected points
		if(area_selected) {
			sf::RectangleShape rectangle;
			rectangle.setFillColor(sf::Color::Transparent);
			double width = ((area_points[1][0]+150.0)/300.0*640.0) - ((area_points[0][0]+150.0)/300.0*640.0);
			double height = ((area_points[1][1]+90.0)/180.0*480.0) - ((area_points[0][1]+90.0)/180.0*480.0);
			rectangle.setSize(sf::Vector2f(width, height));
			rectangle.setOutlineColor(sf::Color::Blue);
			rectangle.setOutlineThickness(1);
			rectangle.setPosition((area_points[0][0]+150.0)/300.0*640.0, (area_points[0][1]+90.0)/180.0*480.0);
			window.draw(rectangle);
		}

		//Draw x-y axis
		sf::Vertex x_axis[] = {sf::Vector2f(0, 240) , sf::Vector2f(640, 240)};
		sf::Vertex y_axis[] = {sf::Vector2f(320, 0) , sf::Vector2f(320, 480)}; 
		window.draw(x_axis, 2, sf::Lines);
		window.draw(y_axis, 2, sf::Lines);

		window.display();
    	
    }

}