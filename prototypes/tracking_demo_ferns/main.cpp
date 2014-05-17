#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <cmath>
#include <string>
#include <sstream>
#include <deque>

class TrackingApp;
#include "app.h"

int main(int argCount, char *argList[])
{
    TrackingApp app;
    
    int width = 800, height = 600;
    int bpp = 32; int flags = sf::Style::Resize | sf::Style::Close;
    std::string windowTitle = app.getWindowTitle();
    app.getWindowMode(width, height, bpp, flags);
    sf::RenderWindow window(
        sf::VideoMode(width, height, bpp),
        windowTitle.c_str(), flags
    );
    
    sf::Clock clock;
    double time = clock.getElapsedTime().asSeconds();
    double prevTime = time;
    
    while(window.isOpen()) {
        
        time = clock.getElapsedTime().asSeconds();
        double deltaTime = time-prevTime;
        prevTime = time;
        
        app.onUpdate(time, deltaTime);
        app.onRender(window, time, deltaTime);
        
        sf::Event event;
        while(window.pollEvent(event)) {
            switch(event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                }
                case sf::Event::KeyPressed: {
                    if(event.key.code == sf::Keyboard::Escape) {
                        window.close();
                    } else {
                        app.onKey(true, event);
                    }
                    break;
                }
                case sf::Event::KeyReleased: {
                    app.onKey(false, event);
                    break;
                }
                case sf::Event::MouseButtonPressed: {
                    app.onMouseButton(true, event);
                    break;
                }
                case sf::Event::MouseButtonReleased: {
                    app.onMouseButton(false, event);
                    break;
                }
                case sf::Event::MouseMoved: {
                    app.onMouseMotion(event);
                    break;
                }
            }
        }
    }
    
    return 0;
}
