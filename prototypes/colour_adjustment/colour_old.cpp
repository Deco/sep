//	This is a refined version of the original colour prototype
//	to handle different types of modelling, linear, flat and 
//	curves/beziers. The initial design could only use cubic
//	functions.

//	This application was developed to demonstrate a variable
//	colour model that can be used to change the way that pixels
//	are represented on the screen. This has applications to the
//	scanning software system that is being implemented, as it can
//	be used to manipulate thermal information to display readings
//	in a way that provides clarity to the user on what those readings
//	mean.
//	@author Sam Holmes 2014

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

//	ControlPoint represents positional values of the points
//	used to generate the graph that will be used to filter 
//	the temperature values into colour values. The type field
//	determines the type of behaviour the point will have;
//	flat, linear, curved/N-grade bezier. 
struct ControlPoint
{
    sf::Vector2f position;
    int type;
};

std::vector<ControlPoint> controls;
int curSelected = 0;

//	Standard recurvise factorial function used in the calculation of 
//	bezier curves.
double factorial(double k)
{
	return (k <= 1) ? 1 : k * factorial(k-1);
}

double bernstain(double i, double n, double t)
{
	return factorial(n) / 
			(factorial(i) * factorial(n-i)) * pow(t,i) * pow(1-t, n-i);
}

//	Bezier function takes in a vector of points (vec2f) as well as 
//	a 'timestep' value and returns a position of where the bezier
//	curve will lie for this timestep. This function will be called 
//	repeatedly to return values of the whole curve. The accuracy of 
//	the curve is determined by the increment of the 'timestep' t
//	iterations.
sf::Vector2f bezier(double t, std::vector<sf::Vector2f> points)
{
	sf::Vector2f r;
	int n = points.size();
	for (int i = 0; i < n; i++)
	{
//	Type-casting return into a float for vec2f
		r += points[i] * (float)bernstain(i,n-1,t);
	}
	return r;
}

//	Method for handling SFML keyboard input, used for controlling
//	the control points of the graph that alter the displayed 
//	colour values for temperature readings.
void handleInput(sf::Event e)
{
	ControlPoint c;


//	Switch statement handling all keyboard input. 
	switch (e.key.code)
	{
		case sf::Keyboard::F:
			c.position = sf::Vector2f(controls[curSelected].position.x + 30, 
				controls[curSelected].position.y - 30);
			c.type = 1;
			controls.insert(controls.begin() + curSelected + 1, c);
			curSelected++;
			break;
		case sf::Keyboard::L:
			c.position = sf::Vector2f(controls[curSelected].position.x + 30, 
				controls[curSelected].position.y - 30);
			c.type = 2;
			controls.insert(controls.begin() + curSelected + 1, c);
			curSelected++;
			break;
		case sf::Keyboard::C:
			c.position = sf::Vector2f(controls[curSelected].position.x + 30, 
				controls[curSelected].position.y - 30);
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
			controls[curSelected].position = sf::Vector2f(
				controls[curSelected].position.x - 1, 
				controls[curSelected].position.y);
			break;
		case sf::Keyboard::Right:
			controls[curSelected].position = sf::Vector2f(
				controls[curSelected].position.x + 1,
				controls[curSelected].position.y);
			break;
		case sf::Keyboard::Up:
			controls[curSelected].position = sf::Vector2f(
				controls[curSelected].position.x, 
				controls[curSelected].position.y - 1);
			break;
		case sf::Keyboard::Down:
			controls[curSelected].position = sf::Vector2f(
				controls[curSelected].position.x, 
				controls[curSelected].position.y + 1);
			break;
	}
}

//	Render function loops within itself to draw the SFML window
//	and all sub-views of that window.
void render()
{
	sf::RenderWindow window(sf::VideoMode(1280, 960), "Colour Diddler");
	window.setFramerateLimit(30);
	
//	Main render loop.
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

//	Clears the last window frame, so we can re-draw.
        window.clear();
//	Vertex array used to draw the line connecting all points.
//	Vertices added to it are dependent on the interactions of 
//	the control points along the graph.
        sf::VertexArray vertices(sf::LinesStrip, 0);
        vertices.append(sf::Vertex(controls[0].position));

//	Loop over all controls points
        for (int i = 0; i < controls.size(); i++)
        {
        	ControlPoint c = controls[i];
        	sf::RectangleShape rect(sf::Vector2f(5,5));
			rect.setPosition(c.position);
			rect.setFillColor(sf::Color(255,0,0));

//	Determine what type of control point
			switch (c.type)
			{
				case 1:
//	Draws a flat line to the next control point
					vertices.append(sf::Vertex(sf::Vector2f(
						controls[i+1].position.x, c.position.y)));
					vertices.append(sf::Vertex(controls[i+1].position));
					break;
				case 2:
//	Draws a linear graph to next control point
					vertices.append(sf::Vertex(controls[i+1].position));
					break;
				case 3:
//	Draws an N-Grade curve until the next control point that
//	isn't type=3.
					std::vector<sf::Vector2f> v;
					int j = 0;
					while (controls[i+j].type == 3)
					{
						v.emplace_back(controls[i+j].position);
						j++;
					}
					i += j;
//	v is a vector containing all the control points of the 
//	following curve.

//	lineToDraw affects the smoothness of the bezier drawn.
					int lineToDrawCount = 25;
					for (int lineToDrawI = 0; lineToDrawI < lineToDrawCount;
						lineToDrawI++)
					{
//	Calls the bezier function and appends the result to the vertex
//	array.
						double t = lineToDrawI/(lineToDrawCount-1);
						vertices.append(bezier(t, v));
					}
					break;
			}
//	Sets the currently selected control point to green.
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
//	The line must contain at least 2 points. 
//	These two control points make up the first and last
//	point of the line.
	ControlPoint c1;
	c1.position = sf::Vector2f(0,480);
	c1.type = 2;
	controls.emplace_back(c1);

	ControlPoint c2;
	c2.position = sf::Vector2f(640,0);
	c1.type = 0;
	controls.emplace_back(c2);

//	render contains the main loop of the application for displaying
//	content and handling inputs.
	render();

	return 1;
}