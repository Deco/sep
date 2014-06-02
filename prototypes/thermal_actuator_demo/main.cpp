#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <algorithm>
#include <assert.h>
#include <sstream>

#include "Dynamixel/DynamixelComm.h"

sf::Color hsv(int hue, float sat, float val);

class ActuatorApp {
public:
    bool running;
    sf::Thread sensorThread;
    sf::Thread actuatorThread;

    sf::Mutex sensorDataMutex;
    float sensorData[64];
    sf::Vector2f hotSpot;
    double ambientTemp;

    bool seekHeat;
    sf::Vector2f moveVec;
    int servoXPos, servoYPos;
    double servoSpeed;

    sf::Font font;
    int cursorX, cursorY;

public:
    ActuatorApp()
        : sensorThread(&ActuatorApp::sensorThreadRun, this)
        , actuatorThread(&ActuatorApp::actuatorThreadRun, this)
    {
        running = true;
        sensorThread.launch();
        actuatorThread.launch();

        hotSpot.x = 0.5;
        hotSpot.y = 0.5;

        seekHeat = false;
        moveVec.x = 0; moveVec.y = 0;

        servoSpeed = 32;

        if(!font.loadFromFile("Arial Black.ttf")) {
            throw std::runtime_error("Unable to load font!");
        }
    }
    void onClose()
    {
        running = false;
    }

    void initRender(sf::RenderWindow &window)
    {
        window.setFramerateLimit(30);
    }
    void onRender(sf::RenderWindow &window, double time, double deltaTime)
    {   
        window.clear();

        window.display();
    }

    bool onUpdate(double time, double deltaTime)
    {
        // 
        return true;
    }
    void onKey(bool state, sf::Event &ev) {
        if(ev.key.code == sf::Keyboard::Space) {
            if(state) {
                seekHeat = !seekHeat;
            }
        } else if(ev.key.code == sf::Keyboard::Left) {
            moveVec.x = (state ? -1 : 0);
        } else if(ev.key.code == sf::Keyboard::Right) {
            moveVec.x = (state ?  1 : 0);
        } else if(ev.key.code == sf::Keyboard::Up) {
            moveVec.y = (state ? -1 : 0);
        } else if(ev.key.code == sf::Keyboard::Down) {
            moveVec.y = (state ?  1 : 0);
        }
        if(state && ev.key.code >= sf::Keyboard::Num0 && ev.key.code <= sf::Keyboard::Num9) {
            switch(ev.key.code) {
                case sf::Keyboard::Num1: servoSpeed =   10.0; break;
                case sf::Keyboard::Num2: servoSpeed =   30.0; break;
                case sf::Keyboard::Num3: servoSpeed =   70.0; break;
                case sf::Keyboard::Num4: servoSpeed =  100.0; break;
                case sf::Keyboard::Num5: servoSpeed =  200.0; break;
                case sf::Keyboard::Num6: servoSpeed =  400.0; break;
                case sf::Keyboard::Num7: servoSpeed =  600.0; break;
                case sf::Keyboard::Num8: servoSpeed =  800.0; break;
                case sf::Keyboard::Num9: servoSpeed = 1024.0; break;
                case sf::Keyboard::Num0: servoSpeed =    0.0; break; // fast!
            }
        }
    }
    void onMouseButton(bool state, sf::Event &ev)
    {
        // 
    }
    void onMouseMotion(sf::Event &ev)
    {
        cursorX = ev.mouseMove.x; cursorY = ev.mouseMove.y;
    }
    void getWindowMode(int &widthRef, int &heightRef, int &bppRef, int &flagsRef)
    {
        widthRef = 800; heightRef = 600; bppRef = 32;
    }
    std::string getWindowTitle() { return "!"; }

    void sensorThreadRun()
    {
        Serial sport("/dev/tty.usbmodem1411", 115200);
        unsigned char data[255];
        
        unsigned char buff[255];
        int rc;

        data[0] = 255;
        int sent = sport.SendBytes((char*)&data, 1);
        assert(sent == 1);

        int count = 0;
        do {
            if(!running) { return; }
            rc = sport.ReceiveBytes((char*)&buff, 1);
            if(rc > 0) {
                if(buff[0] == 255) {
                    count++;
                } else {
                    count = 0;
                }
            }
        } while(count < 50);

        data[0] = 254;
        sent = sport.SendBytes((char*)&data, 1);
        assert(sent == 1);

        do {
            if(!running) { return; }
            rc = sport.ReceiveBytes((char*)&buff, 1);
            if(rc > 0 && buff[0] == 254) {
                break;
            }
        } while(true);
        printf("synced!\n");

        while(running) {
            
            sport.ReceiveBytes((char*)&buff, 1);
            assert(buff[0] == 255);

            sport.ReceiveBytes((char*)&buff, 1);
            unsigned char code = buff[0];

            sport.ReceiveBytes((char*)&buff, 2);
            unsigned short len = *((unsigned short*)buff);

            sport.ReceiveBytes((char*)&buff, len);

            switch(code) {
                case 0x11: {
                    assert(len == sizeof(float));
                    float ambient = *((float*)buff);
                    ambientTemp = ambient;
                    break;
                };
                case 0x12: {
                    assert(len == 64*sizeof(float));
                    float *newSensorData = ((float*)buff);
                    // printf("temp: %f\n", newSensorData[0]);
                    if(1) {
                        sf::Lock sensorDataMutexLock(sensorDataMutex);
                        memcpy(sensorData, newSensorData, sizeof(float)*64);
                    }
                    break;
                };
                default: {
                    assert(1 == 0);
                }
            }
        }
    }
    void actuatorThreadRun()
    {
        DynamixelComm dc("/dev/tty.usbserial-A9S3VTXD", 1000000);
        while(running) {
            servoXPos = dc.GetPosition(01);
            servoYPos = dc.GetPosition(16);
            if(seekHeat) {
                sf::Vector2f spot;
                if (1) {
                    sf::Lock sensorDataMutexLock(sensorDataMutex);
                    spot = hotSpot;
                }
                std::cout << "spot: " << spot.x << ", " << spot.y << std::endl;
                if(std::abs(spot.x-0.5) > 0.1) {
                    int goal = servoXPos+(spot.x < 0.5 ? 10 : -10);
                    //goal = std::min(std::max(goal, 530), 530);
                    //printf("goal X: %d\n", goal);
                    dc.Move(01, goal, (int)servoSpeed);
                }
                if(std::abs(spot.y-0.5) > 0.05) {
                    int goal = servoYPos+(spot.y < 0.5 ? -05 : 05);
                    goal = std::min(std::max(goal, 512-50), 512+50);
                    //printf("goal Y: %d\n", goal);
                    dc.Move(16, goal, (int)servoSpeed);
                }
            } else {
                if(1) {
                    int goal = servoXPos-2000*moveVec.x;
                    goal = std::min(std::max(goal, 0), 1023);
                    //printf("goal X: %d\n", goal);
                    dc.Move(01, goal, (int)servoSpeed);
                }
                if(1) {
                    int goal = servoYPos+2000*moveVec.y;
                    goal = std::min(std::max(goal, 512-330), 512+330);
                    //printf("goal Y: %d\n", goal);
                    dc.Move(16, goal, (int)servoSpeed);
                }
            }
        }
    }
};

int main(int argCount, char *argList[])
{
    ActuatorApp app;
    
    int width = 800, height = 600;
    int bpp = 32; int flags = sf::Style::Resize | sf::Style::Close;
    std::string windowTitle = app.getWindowTitle();
    app.getWindowMode(width, height, bpp, flags);
    sf::RenderWindow window(
        sf::VideoMode(width, height, bpp),
        windowTitle.c_str(), flags
    );
    app.initRender(window);
    
    sf::Clock clock;
    double time = clock.getElapsedTime().asSeconds();
    double prevTime = time;
    
    while(window.isOpen()) {
        
        time = clock.getElapsedTime().asSeconds();
        double deltaTime = time-prevTime;
        prevTime = time;
        
        if(!app.onUpdate(time, deltaTime)) {
            app.onClose();
            window.close();
            return 0;
        }
        app.onRender(window, time, deltaTime);
        
        sf::Event event;
        while(window.pollEvent(event)) {
            switch(event.type) {
                case sf::Event::Closed: {
                    app.onClose();
                    window.close();
                    return 0;
                    break;
                }
                case sf::Event::KeyPressed: {
                    if(event.key.code == sf::Keyboard::Escape) {
                        app.onClose();
                        window.close();
                        return 0;
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

// hue: 0-360°; sat: 0.f-1.f; val: 0.f-1.f
sf::Color hsv(int hue, float sat, float val)
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
