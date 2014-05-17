#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>

#define PI 3.14159265

using namespace std;

sf::Vector2i Extrapolation(sf::Vector2f circlePoint, sf::Vector2i posArray[7])
{
	double diffX;
	double diffY;
	double sumX = 0;
	double sumY = 0;

	for (int i = 6; i > 0; i--)
	{
		diffX = posArray[i].x - circlePoint.x;
		diffY = posArray[i].y - circlePoint.y;
		sumX += diffX;
		sumY += diffY;
		cout << diffX << endl;
	}

	sf::Vector2i prediction;

	prediction.x = posArray[6].x + (sumX/10);
	prediction.y = posArray[6].y + (sumY/10);

	return prediction;
}

double Lerp(double y1, double y2, double mu)
{
	return (y1 * (1-mu) + y2*mu);
}

double Cerp(double y1, double y2, double mu)
{
	double mu2;
	mu2 = (1-cos(mu*PI))/2;
	return (y1 * (1-mu2) + y2*mu2);
}

void Render()
{
	sf::RenderWindow window(sf::VideoMode(1400, 800), "SmoothCam");
	window.setFramerateLimit(30);
	sf::CircleShape circle(20.0f);
	circle.setFillColor(sf::Color(250,250,50));
	circle.setPosition(400,400);

	sf::RectangleShape rectangle(sf::Vector2f(500, 500));
	rectangle.setFillColor(sf::Color(0,0,250,80));
	rectangle.setPosition(200,200);

	sf::Clock clock;
	sf::Time duration;

	sf::Clock clock2;
	sf::Time duration2;

	sf::Vector2i posArray[7];

	posArray[0] = sf::Mouse::getPosition(window);
	posArray[1] = posArray[0];
	posArray[2] = posArray[1];
	posArray[3] = posArray[2];
	posArray[4] = posArray[3];
	posArray[5] = posArray[4];
	posArray[6] = posArray[5];

	sf::Vector2i predPos;
	sf::Vector2i actualPos;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

		duration = clock.getElapsedTime();
		if (duration.asSeconds() > 0.05)
		{
			clock.restart();
			sf::Vector2i locPos = sf::Mouse::getPosition(window);
			if ((locPos.x > 0) && (locPos.x < 1400) && (locPos.y > 0) && (locPos.y < 800))
			{
				for (int i = 0; i < 7; i++)
				{
					if (i == 6)
					{
						posArray[6] = locPos;
					}
					else
					{
						posArray[i] = posArray[i+1];
					}
				}
			}
		}


		double counter = 0.25;
		duration2 = clock2.getElapsedTime();
		if (duration2.asSeconds() > 0.01)
		{
			predPos = Extrapolation(circle.getPosition(), posArray);
			if ((circle.getPosition().x != predPos.x) || 
				(circle.getPosition().y != predPos.y))
			{
				actualPos.x = Cerp(circle.getPosition().x, predPos.x, counter);
				actualPos.y = Cerp(circle.getPosition().y, predPos.y, counter);
				counter = counter + 0.25;
				if (counter >= 1)
				{
					counter = 0;
					clock2.restart();
				}
				circle.setPosition(actualPos.x, actualPos.y);
			}
		}

		rectangle.setPosition(circle.getPosition().x - 230, circle.getPosition().y - 230);

        window.clear();
        window.draw(circle);
        window.draw(rectangle);
        window.display();
        sf::sleep(sf::milliseconds(1));
    }
}

int main()
{
	Render();
	return 1;
}