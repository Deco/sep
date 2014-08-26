#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

struct ControlPoint
{
    sf::Vector2f position;
    int type;
};

std::vector<ControlPoint> controls;
int curSelected = 0;

int factorial(int k)
{
	if ((k == 0) || (k == 1))
	{
		return 1;
	}
	else
	{
		return k * factorial(k-1);
	}
}

int bernstain(int i, int n, float t)
{
	return factorial(n) / (factorial(i) * factorial(n-i)) * pow(t,i) * pow(1-t, n-i);
}

std::vector<sf::Vector2f> bezier(float t, std::vector<sf::Vector2f> points)
{
	std::vector<sf::Vector2f> r;
	int n = points.size() - 1;
	for (int i = 0; i <= n; i++)
	{
		sf::Vector2f temp;
		temp.x = points[i].x * bernstain(i,n,t);
		temp.y = points[i].y * bernstain(i,n,t);
		r.emplace_back(temp);
	}
	return r;
}


void handleInput(sf::Event e)
{
	ControlPoint c;

	switch (e.key.code)
	{
		case sf::Keyboard::F:
			c.position = sf::Vector2f(controls[curSelected].position.x + 30, controls[curSelected].position.y - 30);
			c.type = 1;
			controls.insert(controls.begin() + curSelected + 1, c);
			curSelected++;
			break;
		case sf::Keyboard::L:
			c.position = sf::Vector2f(controls[curSelected].position.x + 30, controls[curSelected].position.y - 30);
			c.type = 2;
			controls.insert(controls.begin() + curSelected + 1, c);
			curSelected++;
			break;
		case sf::Keyboard::C:
			c.position = sf::Vector2f(controls[curSelected].position.x + 30, controls[curSelected].position.y - 30);
			c.type = 3;
			controls.insert(controls.begin() + curSelected + 1, c);
			curSelected++;
			break;

		case sf::Keyboard::N:
			curSelected++;
			if (curSelected >= controls.size())
			{
				curSelected = 0;
			}
			break;
		case sf::Keyboard::P:
			curSelected--;
			if (curSelected < 0)
			{
				curSelected = controls.size() - 1;
			}			
			break;

		case sf::Keyboard::Left:
			controls[curSelected].position = sf::Vector2f(controls[curSelected].position.x - 1, controls[curSelected].position.y);
			break;
		case sf::Keyboard::Right:
			controls[curSelected].position = sf::Vector2f(controls[curSelected].position.x + 1, controls[curSelected].position.y);
			break;
		case sf::Keyboard::Up:
			controls[curSelected].position = sf::Vector2f(controls[curSelected].position.x, controls[curSelected].position.y - 1);
			break;
		case sf::Keyboard::Down:
			controls[curSelected].position = sf::Vector2f(controls[curSelected].position.x, controls[curSelected].position.y + 1);
			break;
	}
}

void render()
{
	sf::RenderWindow window(sf::VideoMode(1280, 960), "Colour Diddler");
	window.setFramerateLimit(30);

	sf::Font font;
	font.loadFromFile("arial.ttf");

	sf::Text hue;
	hue.setColor(sf::Color(0,255,0));
	hue.setPosition(0,240);
	hue.setFont(font);
	hue.setString("HUE");
	hue.setCharacterSize(24);

	std::string str;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

        	if (event.type == sf::Event::KeyPressed)
        		handleInput(event);
        }

        window.clear();

        sf::VertexArray vertices(sf::LinesStrip, 0);
        vertices.append(sf::Vertex(controls[0].position));

        for (int i = 0; i < controls.size(); i++)
        {
        	ControlPoint c = controls[i];
        	sf::RectangleShape rect(sf::Vector2f(5,5));
			rect.setPosition(c.position);
			rect.setFillColor(sf::Color(255,0,0));

			switch (c.type)
			{
				case 1:
					vertices.append(sf::Vertex(sf::Vector2f(controls[i+1].position.x, c.position.y)));
					vertices.append(sf::Vertex(controls[i+1].position));
					break;
				case 2:
					vertices.append(sf::Vertex(controls[i+1].position));
					break;
				case 3:
					std::vector<sf::Vector2f> v;
					int j = 0;
					while (controls[i+j].type == 3)
					{
						v.emplace_back(controls[i+j].position);
						j++;
					}
					i += j;
					std::vector<sf::Vector2f> bezierPoints = bezier(0.1, v);

					for (sf::Vector2f vec : bezierPoints)
					{
						vertices.append(sf::Vertex(vec));
					}
					break;
			}
			if (i == curSelected)
			{
				rect.setFillColor(sf::Color(0,255,0));
			}
			window.draw(rect);

        }
        window.draw(vertices);

        window.display();
        sf::sleep(sf::milliseconds(1));
    }
}

int main()
{	
	ControlPoint c1;
	c1.position = sf::Vector2f(0,480);
	c1.type = 2;
	controls.emplace_back(c1);

	ControlPoint c2;
	c2.position = sf::Vector2f(640,0);
	c1.type = 0;
	controls.emplace_back(c2);

	render();
	return 1;
}