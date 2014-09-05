//	This application is a basic proof of concept for creating
//	a variable colour model that will allow temperature values
//	to be represented by a varying RGB based on the user input.
//	Initially the application will only deal with a 4-point
//	cubic function for graphing, but will later be extended to deal
//	with linear/flat and N-Grade curves (curves with more 3 or more
//	control points).
//	@author Sam Holmes 2014 


#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

std::vector<sf::RectangleShape> points;
int curSelected = 0;

//	Function for converting hue values (0->360), saturation and 
//	value (0->1) and converting them to sf::Color objects which
//	can be used for drawing within the app.
sf::Color hsvToRgb(int hue, float sat, float val)
{
    hue %= 360;
    while(hue<0) hue += 360;

    if(sat<0.f) sat = 0.f;
    if(sat>1.f) sat = 1.f;

    if(val<0.f) val = 0.f;
    if(val>1.f) val = 1.f;

    int h = hue/60;
    float f = float(hue)/60-h;
    float p = val*(1.f-sat);
    float q = val*(1.f-sat*f);
    float t = val*(1.f-sat*(1-f));

    switch(h)
    {
        default:
        case 0:
        case 6: return sf::Color(val*255, t*255, p*255);
        case 1: return sf::Color(q*255, val*255, p*255);
        case 2: return sf::Color(p*255, val*255, t*255);
        case 3: return sf::Color(p*255, q*255, val*255);
        case 4: return sf::Color(t*255, p*255, val*255);
        case 5: return sf::Color(val*255, p*255, q*255);
    }
}
//	Cubic Bezier calculator from:
//	https://github.com/LaurentGomila/SFML/wiki/Source:-cubic-bezier-curve
//	Credit goes to LaurentGomila
//	Takes a start & end point and two control points and calculates 
//	the points of a cubic line from these points, based on the number of 
//	segments specified.
std::vector<sf::Vector2f> calcCubicBezier(
        const sf::Vector2f &start,
        const sf::Vector2f &end,
        const sf::Vector2f &startControl,
        const sf::Vector2f &endControl,
        const size_t numSegments)
{
    std::vector<sf::Vector2f> ret;
    if (!numSegments)
        return ret;

    ret.push_back(start);
    float p = 1.f / numSegments;
    float q = p;
    for (size_t i = 1; i < numSegments; i++, p += q)
        ret.push_back(p * p * p * (end + 3.f * (startControl - endControl) - start) +
                      3.f * p * p * (start - 2.f * startControl + endControl) +
                      3.f * p * (startControl - start) + start);
    ret.push_back(end);
    return ret;
}

//	Handles input to move the control points of the graph. Points
//	can only be moved up or down, affecting the hue displayed, and
//	not affecting the temperature range.
void movePoint(sf::Event e)
{
	switch (e.key.code)
	{
		case sf::Keyboard::Num1:
			curSelected = 0;
			break;
		case sf::Keyboard::Num2:
			curSelected = 1;
			break;
		case sf::Keyboard::Num3:
			curSelected = 2;
			break;
		case sf::Keyboard::Num4:
			curSelected = 3;
			break;
		case sf::Keyboard::Up:
			points[curSelected].setPosition(points[curSelected].getPosition().x, points[curSelected].getPosition().y - 1);
			break;
		case sf::Keyboard::Down:
			points[curSelected].setPosition(points[curSelected].getPosition().x, points[curSelected].getPosition().y + 1);
			break;
	}
}

//	Main render function for displaying content and receiving inputs
void render()
{
	sf::RenderWindow window(sf::VideoMode(1280, 960), "Colour Diddler");
	window.setFramerateLimit(30);

//	Drawing of the grid/axis and labels for the graph.
	sf::RectangleShape grid1(sf::Vector2f(2,380));
	grid1.setFillColor(sf::Color(0,255,0));
	grid1.setPosition(60,40);

	sf::RectangleShape grid2(sf::Vector2f(500,2));
	grid2.setFillColor(sf::Color(0,255,0));
	grid2.setPosition(60,420);

	sf::Font font;
	font.loadFromFile("arial.ttf");

	sf::Text hue;
	hue.setColor(sf::Color(0,255,0));
	hue.setPosition(0,240);
	hue.setFont(font);
	hue.setString("HUE");
	hue.setCharacterSize(24);

	sf::Text temp;
	temp.setColor(sf::Color(0,255,0));
	temp.setPosition(270,420);
	temp.setFont(font);
	temp.setString("TEMP");
	temp.setCharacterSize(24);

	sf::Text origin;
	origin.setColor(sf::Color(0,255,0));
	origin.setPosition(50,420);
	origin.setFont(font);
	origin.setString("0");
	origin.setCharacterSize(24);

	sf::Text tempMax;
	tempMax.setColor(sf::Color(0,255,0));
	tempMax.setPosition(550,420);
	tempMax.setFont(font);
	tempMax.setString("64");
	tempMax.setCharacterSize(24);

	sf::Text hueMax;
	hueMax.setColor(sf::Color(0,255,0));
	hueMax.setPosition(20,40);
	hueMax.setFont(font);
	hueMax.setString("360");
	hueMax.setCharacterSize(24);

//	Generates the 4 control points
 	for (int i = 0; i <= 3; i++)
 	{
 		sf::RectangleShape rect(sf::Vector2f(5,5));
		rect.setFillColor(sf::Color(255,0,0));
		rect.setPosition(65 + i*140, 410 - i*120);
		points.emplace_back(rect);
 	}

//	Main render loop
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            	movePoint(event);
        }

        window.clear();

//	Create a vertex array for drawing
		sf::VertexArray vertices(sf::LinesStrip, 0);
//	Calculate the points on the curve (10 segments)
		std::vector<sf::Vector2f> linePoints =
		    CalcCubicBezier(
		        points[0].getPosition(),
		        points[3].getPosition(),
		       	points[1].getPosition(),
		        points[2].getPosition(),
		        360);

//	Append the points as vertices to the vertex array
		for (std::vector<sf::Vector2f>::const_iterator a = linePoints.begin(); a != linePoints.end(); ++a)
		    vertices.append(sf::Vertex(*a, sf::Color::White));
//	Draw stuff
		window.draw(vertices);
		window.draw(grid1);
		window.draw(grid2);
		window.draw(hue);
		window.draw(temp);
		window.draw(origin);
		window.draw(tempMax);
		window.draw(hueMax);
//	Spectrum drawing
        for (int i = 0; i < 360; i++)
		{
			sf::RectangleShape rect(sf::Vector2f(10,480));
			
			std::cout << 480 - linePoints[i].y - 70 << std::endl;
		
			rect.setFillColor(hsvToRgb(480 - linePoints[i].y - 70, 1, 1));
			rect.setPosition(i*2 + 640, 0);
			window.draw(rect);
		}

//std::cout << "Max Hue: " << (480 - linePoints[359].y  - 70) << std::endl;
//std::cout << "Min Hue: " << (480 - linePoints[0].y - 70) << std::endl;

//	Dragable points drawing.
		for (sf::RectangleShape r : points)
		{
			window.draw(r);
		}

        window.display();
        sf::sleep(sf::milliseconds(1));
    }
}

int main()
{	
	render();
	return 1;
}