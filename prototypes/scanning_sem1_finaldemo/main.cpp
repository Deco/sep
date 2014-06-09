#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <math.h>

#include "SensorSamplerRealm.h"
#include "sensor_controller.h"
#include "ActuatorController.h"

cv::Vec4b hsv(int hue, float sat, float val);
void copyMatToTexture(const cv::Mat &mat, sf::Texture &tex, bool toGray=false);


class ScanningApp {
public:
    static const int windowWidth  = 1440;
    static const int windowHeight = 600;

    static constexpr double stillDelay = 0.5;
    static constexpr double defaultFOV = 45.0;

public:
    bool running;
    
    ActuatorController ac;
    ThermalSensorController sc;
    SensorSamplerRealm realm;

    SensorViewWindow realmView;

    sf::View thermalView;

    std::deque<cv::Vec2d> movementQueue;
    double timeStill;
    bool orderPending;
    bool isQuickMove;
    bool hasSampled;
    
    sf::Texture tex;

    double clickTemp;

    sf::Font debugOverlayFont;
    int cursorX, cursorY;

    std::string sensorDeviceName;
    std::string actuatorDeviceName;

public:
    ScanningApp(std::string sensorDeviceName_, std::string actuatorDeviceName_, bool noact=false)
	    : sensorDeviceName(sensorDeviceName_)
        , actuatorDeviceName(actuatorDeviceName_)
        , ac(actuatorDeviceName_)
        , sc(sensorDeviceName_)
        , realm(
            cv::Vec2d(-150.00, -90.00),
            cv::Vec2d( 150.00,  90.00),
            cv::Vec2d(   3.75,   3.75)
          )
        , realmView(
            cv::Mat(windowWidth, windowHeight, CV_8UC4),
            cv::Vec2d(-150.0, - 90.0),
            cv::Vec2d( 300.0,  180.0)
          )
        , thermalView(
            sf::Vector2f(150, 90),
            sf::Vector2f(defaultFOV*(double)windowWidth/(double)windowHeight, defaultFOV)
          )
        , movementQueue()
	    , cursorX(0), cursorY(0)
    {
        running = true;

        ac.init();
        sc.init();
        //realm.init();

        timeStill = 0.0;
        orderPending = false;
        isQuickMove = false;
        hasSampled = false;

        clickTemp = 20.0;
        
        if(!debugOverlayFont.loadFromFile("Arial Black.ttf")) {
            throw std::runtime_error("Unable to load font!");
        }
    }
    void onClose() { running = false; }
    
    void initRender(sf::RenderWindow &window) { window.setFramerateLimit(30); }
    
    void onRender(sf::RenderWindow &window, double time, double deltaTime)
    {
        window.clear();

        window.setView(thermalView);

        sf::Sprite sprite;
        sprite.setTexture(tex);
        sprite.setPosition(0, 0);
        sf::Vector2u texSize = tex.getSize();
        sprite.setScale(sf::Vector2f(
            300.0/(double)texSize.x,
            180.0/(double)texSize.y
        ));
        window.draw(sprite);

        cv::Vec2d acPos = ac.getCurrentPosition();

        if(1) {
            auto rectPos = sf::Vector2f(acPos[0]+150.0, acPos[1]+90.0);
            
            sf::RectangleShape rect;
            rect.setPosition(rectPos-sf::Vector2f(1.5, 1.5));
            rect.setSize(sf::Vector2f(3, 3));
            rect.setFillColor(sf::Color::White);
            window.draw(rect);
            rect.setPosition(rectPos-sf::Vector2f(0.5, 0.5));
            rect.setSize(sf::Vector2f(1, 1));
            rect.setFillColor(sf::Color(100, 100, 100, 255));
            window.draw(rect);
        }

        window.setView(window.getDefaultView());

        sf::Text text((std::stringstream()
            << "blah\n"
            << "meow\n"
        ).str(), debugOverlayFont, 32);
        text.setColor(sf::Color::White);
        text.setPosition(sf::Vector2f(5, 550));
        window.draw(text);
        
        window.display();
    }
    
    bool onUpdate(sf::RenderWindow &window, double time, double deltaTime)
    {
        float leftMove  = sf::Keyboard::isKeyPressed(sf::Keyboard::Left ) ? -1.0 : 0;
        float rightMove = sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ?  1.0 : 0;
        float upMove    = sf::Keyboard::isKeyPressed(sf::Keyboard::Up   ) ? -1.0 : 0;
        float downMove  = sf::Keyboard::isKeyPressed(sf::Keyboard::Down ) ?  1.0 : 0;
        thermalView.move(25.0f*sf::Vector2f(
            ( leftMove+rightMove)*deltaTime,
            (   upMove+ downMove)*deltaTime
        ));

        float inZoom = sf::Keyboard::isKeyPressed(sf::Keyboard::LBracket) ? -1.0 : 0;
        float outZoom = sf::Keyboard::isKeyPressed(sf::Keyboard::RBracket) ? 1.0 : 0;
        thermalView.zoom(1.0+0.1*(inZoom+outZoom));

        cv::Mat_<float> readingImg;
        double readingTime;

        if(ac.getIsMoving()) {
            orderPending = false;
            timeStill = 0.0;
        } else {
            timeStill += deltaTime;
        }

        bool hasChanged = false;
        while(true) {
            bool isNewReading = sc.popThermopileReading(readingImg, readingTime);
            if(!isNewReading) {
                break;
            }

            if(timeStill > stillDelay) {
                hasChanged = true;
            }
        }
        if(hasChanged and !hasSampled) {
            hasSampled = true;
            realm.addReadingWindow(
                readingImg,
                ac.getCurrentPosition(),
                readingTime
            );
        }
        if(hasChanged) {
            realm.updateViewWindow(
                realmView,
                [](cv::Mat &pixelRef, const cv::Vec2i &pixelPos, const float &temp) {
                    //std::cout << temp << std::endl;
                    cv::Vec4b col = hsv(
                        300.0*(std::max(temp, 14.0f)-14.0)/(40.0-14.0),
                        1, 1
                    );
                    if(temp <= 11.0) {
                        col = cv::Vec4b(30, 30, 50, 255);
                    } else if(temp > 40.0) {
                        col = cv::Vec4b(255, 255, 255, 255);
                    }
                    pixelRef.at<cv::Vec4b>(pixelPos[1], pixelPos[0]) = col;
                }
            );
            copyMatToTexture(realmView.getImage(), tex);
        }
        if(timeStill > stillDelay*1.1 and movementQueue.size() > 0) {
            std::cout << "still! " << movementQueue.size() << std::endl;
            ac.stop();
            ActuatorMoveOrder order;
            order.posDeg = movementQueue.front();
            movementQueue.pop_front();
            order.duration = 0.1;
            ac.queueMove(order);
            orderPending = true;
            timeStill = 0.0;
            hasSampled = false;
        }

        return true;
    }
    void onKey(sf::RenderWindow &window, bool state, sf::Event &ev) {
        if(ev.key.code == sf::Keyboard::Space) {
            realmView.forceInvalidation();
            realm.clearAllReadings();
        }
        if(ev.key.code == sf::Keyboard::LShift or ev.key.code == sf::Keyboard::RShift) {
            isQuickMove = state;
        }
        if(state && ev.key.code >= sf::Keyboard::Num0 && ev.key.code <= sf::Keyboard::Num9) {
            switch(ev.key.code) {
                case sf::Keyboard::Num1: clickTemp = 15.0; break;
                case sf::Keyboard::Num2: clickTemp = 17.0; break;
                case sf::Keyboard::Num3: clickTemp = 19.0; break;
                case sf::Keyboard::Num4: clickTemp = 21.0; break;
                case sf::Keyboard::Num5: clickTemp = 23.0; break;
                case sf::Keyboard::Num6: clickTemp = 25.0; break;
                case sf::Keyboard::Num7: clickTemp = 27.0; break;
                case sf::Keyboard::Num8: clickTemp = 29.0; break;
                case sf::Keyboard::Num9: clickTemp = 31.0; break;
                case sf::Keyboard::Num0: clickTemp = 33.0; break;
            }
        }
    }
    void onMouseButton(sf::RenderWindow &window, bool state, sf::Event &ev)
    {
        cursorX = ev.mouseButton.x; cursorY = ev.mouseButton.y;
        window.setView(thermalView);
        sf::Vector2f cursorPos = window.mapPixelToCoords(sf::Vector2i(cursorX, cursorY));
        std::cout << "click: " << cursorPos.x << ", " << cursorPos.y << std::endl;
        cv::Vec2d clickPos(cursorPos.x-150.0, cursorPos.y-90.0);

        if(ev.mouseButton.button == sf::Mouse::Left and state) {
            /*cv::Mat_<float> readingImg(4, 16);
            double readingTime;
            sc.popThermopileReading(readingImg, readingTime);
            // for(int viewColI = 0; viewColI < readingImg.cols; viewColI++) {
            //     for(int viewRowI = 0; viewRowI < readingImg.rows; viewRowI++) {
            //         readingImg(viewRowI, viewColI) = 20.0;
            //     }
            // }
            // readingImg(3, 3) = 33.0;

            realm.addReadingWindow(
                readingImg,
                posDeg,
                readingTime
            );*/
            std::cout << "maow: " << isQuickMove << std::endl;
            if(isQuickMove) {
                ac.stop();
                ActuatorMoveOrder order;
                order.posDeg = clickPos;
                order.duration = 2;
                ac.queueMove(order);
                movementQueue.clear();
            } else {
                cv::Vec2d currPos = (
                        movementQueue.empty() ? ac.getCurrentPosition()
                    :   movementQueue.back()
                );
                double distance = cv::norm(clickPos-currPos);
                int stepCount = floor(distance/4.0);
                for(int stepI = 0; stepI < stepCount; stepI++) {
                    if(stepI < stepCount-1) {
                        cv::Vec2d goalPos = currPos+(clickPos-currPos)*(double)(stepI+1)/(double)stepCount;
                        movementQueue.push_back(goalPos);
                        std::cout << "goal: " << goalPos[0] << ", " << goalPos[1] << std::endl;
                    } else {
                        movementQueue.push_back(clickPos);
                    }
                }
            }
        }
    }
    void onMouseMotion(sf::RenderWindow &window, sf::Event &ev)
    {
        cursorX = ev.mouseMove.x; cursorY = ev.mouseMove.y;
        cursorX = ev.mouseButton.x; cursorY = ev.mouseButton.y;
        window.setView(thermalView);
        sf::Vector2f cursorPos = window.mapPixelToCoords(sf::Vector2i(cursorX, cursorY));
    }
    void getWindowMode(int &widthRef, int &heightRef, int &bppRef, int &flagsRef)
    {
        widthRef = windowWidth; heightRef = windowHeight; bppRef = 32;
    }
    std::string getWindowTitle() { return "!"; }
    
};

int main(int argCount, char *argList[])
{
    std::string sdn = "/dev/tty.usbmodem1421";
    std::string adn = "/dev/tty.usbserial-A9S3VTXD";
    if(argCount >= 2) {
       sdn = std::string(argList[1]);
    }
    if(argCount >= 3) {
       adn = std::string(argList[2]);
    }
    ScanningApp app(sdn, adn, (adn == "off"));
    
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
        
        if(!app.onUpdate(window, time, deltaTime)) {
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
                        app.onKey(window, true, event);
                    }
                    break;
                }
                case sf::Event::KeyReleased: {
                    app.onKey(window, false, event);
                    break;
                }
                case sf::Event::MouseButtonPressed: {
                    app.onMouseButton(window, true, event);
                    break;
                }
                case sf::Event::MouseButtonReleased: {
                    app.onMouseButton(window, false, event);
                    break;
                }
                case sf::Event::MouseMoved: {
                    app.onMouseMotion(window, event);
                    break;
                }
            }
        }
    }
    
    return 0;
}

// hue: 0-360°; sat: 0.
cv::Vec4b hsv(int hue, float sat, float val)
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
        case 6: return cv::Vec4b(val*255, t*255, p*255, 255);
        case 1: return cv::Vec4b(q*255, val*255, p*255, 255);
        case 2: return cv::Vec4b(p*255, val*255, t*255, 255);
        case 3: return cv::Vec4b(p*255, q*255, val*255, 255);
        case 4: return cv::Vec4b(t*255, p*255, val*255, 255);
        case 5: return cv::Vec4b(val*255, p*255, q*255, 255);
    }
}

void copyMatToTexture(
    const cv::Mat &mat, sf::Texture &tex, bool toGray
) {
    if(mat.total() == 0) { return; }
    
    sf::Image tempImage;
    std::vector<unsigned char> imgData;
    imgData.resize(mat.total()*4);
    
    cv::Mat continuousRBGA(mat.size(), CV_8UC4, &imgData[0]);

    cv::cvtColor(mat, continuousRBGA, (toGray ? CV_BGR2GRAY : CV_BGR2RGBA), 4); 
    
    tempImage.create(mat.cols, mat.rows, &imgData[0]);
    
    tex.loadFromImage(tempImage);
}

