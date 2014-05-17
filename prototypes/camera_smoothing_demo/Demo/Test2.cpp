#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

#define PI 3.14159265

using namespace std;

double Cerp(double y1, double y2, double mu)
{
	double mu2;
	mu2 = (1-cos(mu*PI))/2;
	return (y1 * (1-mu2) + y2*mu2);
}

int main()
{	
	sf::RenderWindow window(sf::VideoMode(1400, 800), "SmoothCam");
	window.setFramerateLimit(30);
	
	sf::CircleShape circ1(10.0f);
	sf::CircleShape circ2(10.0f);
	sf::CircleShape circ3(10.0f);
	sf::CircleShape circ4(10.0f);
	sf::CircleShape circ5(10.0f);
	sf::CircleShape circ6(10.0f);

	circ1.setFillColor(sf::Color(250,0,0));
	circ2.setFillColor(sf::Color(250,0,0));
	circ3.setFillColor(sf::Color(250,0,0));
	circ4.setFillColor(sf::Color(250,0,0));
	circ5.setFillColor(sf::Color(250,0,0));
	circ6.setFillColor(sf::Color(0,250,0));

	sf::RectangleShape rect(sf::Vector2f(50,50));
	rect.setFillColor(sf::Color(0,0,250));
	rect.setPosition(700,400);

	sf::Clock clock;
	sf::Time duration;

	bool isExtra = false;
	bool isDone = false;
	bool isInter = false;

	int counter = 0;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			clock.restart();
			isExtra = true;
		}
		if (isExtra)
		{
			duration = clock.getElapsedTime();
			if (duration.asSeconds() >= 0.1)
			{	
				clock.restart();
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				counter++;
				switch (counter)
				{
					case 1:
						circ1.setPosition(mousePos.x, mousePos.y);
						break;
					case 2:
						circ2.setPosition(mousePos.x, mousePos.y);
						break;
					case 3:
						circ3.setPosition(mousePos.x, mousePos.y);
						break;
					case 4:
						circ4.setPosition(mousePos.x, mousePos.y);
						break;
					case 5:
						circ5.setPosition(mousePos.x, mousePos.y);
						isExtra = false;
						isDone = true;
						counter = 0;
						clock.restart();
						break;
				}
			}
		}

		if (isDone)
		{
			duration = clock.getElapsedTime();
			if (duration.asSeconds() >= 0.7)
			{	
				sf::Vector2i dist1;
				sf::Vector2i dist2;
				sf::Vector2i dist3;
				sf::Vector2i dist4;
				sf::Vector2i dist5;

				dist1.x = circ2.getPosition().x - circ1.getPosition().x;
				dist1.y = circ2.getPosition().y - circ1.getPosition().y;

				dist2.x = circ3.getPosition().x - circ2.getPosition().x;
				dist2.y = circ3.getPosition().y - circ2.getPosition().y;

				dist3.x = circ4.getPosition().x - circ3.getPosition().x;
				dist3.y = circ4.getPosition().y - circ3.getPosition().y;

				dist4.x = circ5.getPosition().x - circ4.getPosition().x;
				dist4.y = circ5.getPosition().y - circ4.getPosition().y;

				dist5.x = (dist1.x*2 + dist2.x*4 + dist3.x*16 + dist4.x*64)/86;
				dist5.y = (dist1.y*2 + dist2.y*4 + dist3.y*16 + dist4.y*64)/86;

				circ6.setPosition(circ5.getPosition().x + dist5.x, 
								  circ5.getPosition().y + dist5.y);
				isInter = true;
				isDone = false;
				clock.restart();
			}
		}

		sf::Vector2i actualPos;

		if (isInter)
		{
			duration = clock.getElapsedTime();
			if (duration.asSeconds() >= 0.01)
			{
				actualPos.x = Cerp(rect.getPosition().x, circ6.getPosition().x,
				 					duration.asSeconds()/3);
				actualPos.y = Cerp(rect.getPosition().y, circ6.getPosition().y,
									duration.asSeconds()/3);

				if (duration.asSeconds() >= 3)
				{
					clock.restart();
					isInter = false;
				}
				rect.setPosition(actualPos.x, actualPos.y);
			}
		}

		window.clear();
		window.draw(circ1);
		window.draw(circ2);
		window.draw(circ3);
		window.draw(circ4);
		window.draw(circ5);
		window.draw(circ6);
		window.draw(rect);
		window.display();
        sf::sleep(sf::milliseconds(1));
	}
	return 1;
}