#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <math.h>

#include <SensorSamplerRealm.h>


sf::Color hsv(int hue, float sat, float val);
void copyMatToTexture(const cv::Mat &mat, sf::Texture &tex, bool toGray=false);


class ScanningApp {
public:
    bool running;
    
    double clickTemp;
    
    sf::Font debugOverlayFont;
    int cursorX, cursorY;

    std::string sensorDeviceName;
    std::string actuatorDeviceName;

public:
    ScanningApp(std::string sensorDeviceName_, std::string actuatorDeviceName_, bool noact=false)
	    : sensorDeviceName(sensorDeviceName_), actuatorDeviceName(actuatorDeviceName_)
	    , cursorX(0), cursorY(0)
    {
        running = true;
        
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
        
        sf::Text text((std::stringstream()
            << "blah\n"
            << "meow\n"
        ), debugOverlayFont, 32);
        text.setColor(sf::Color::White);
        text.setPosition(sf::Vector2f(5, 170));
        window.draw(text);
        
        window.display();
    }
    
    bool onUpdate(double time, double deltaTime)
    {
        // 
        return true;
    }
    void onKey(bool state, sf::Event &ev) {
        if(ev.key.code == sf::Keyboard::Space) {
            //
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
    void onMouseButton(bool state, sf::Event &ev)
    {
        
        cursorX = ev.mouseButton.x; cursorY = ev.mouseButton.y;
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
    
};

int main(int argCount, char *argList[])
{
    std::string sdn = "/dev/tty.usbmodem1411";
    std::string adn = "/dev/tty.usbserial-A9S3VTXD";
    std::cout << "wat: " << argCount << std::endl;
    if(argCount >= 3) {
       sdn = std::string(argList[1]);
       adn = std::string(argList[2]);
    }
    std::cout << sdn << std::endl;
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
        
        if(!app.onUpdate(time, deltaTime)) {
            app.onClose();
            window.close();
            return 0;
        }
        app.onRender(window, time, deltaTime);
        
        sf::Event event;
        while(window.pollEvent(event)) {
            switch(event.type) {
                case sf::Event::Closed: {)
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

// hue: 0-360°; sat: 0.
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

