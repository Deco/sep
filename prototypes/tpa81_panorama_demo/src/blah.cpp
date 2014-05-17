//#include "config.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <cmath>
#include <string>
#include <sstream>
#include <deque>

#include <serial_connection.h>

#include "httpserver.h"
#include "wsserver.h"
#include "recttree.h"

#define byte unsigned char
#define PI 3.141592653589793238463
#define EXP 2.7182818284590452
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define clamp(v, m0, m1) (min(max((v), (m0)), (m1)))

double TTT;
double weibull(double x, double l, double k);
sf::Color hsv(int hue, float sat, float val);

class SensorReading {
public:
  int temp; double time;
  inline SensorReading(int _temp, double _time)
    : temp(_temp), time(_time)
  {
    // 
  }
};

class ThermalScannerApp {
private:
  static const int windowWidth = 1440;
  static const int windowHeight = 600;//900;

  static const double thermalMaxReadings = 10000;
  static const double thermalPixelSizePitch = 5.12;
  static const double thermalPixelSizeYaw = 6.0;
  static const double thermalObservedPitchRange = 135.0;
  static const double thermalViewPitchRange = 60.0;
  static const double thermalViewMoveRate = 10.0;
  static const int thermalViewPixelSize = 1;

  static const int thermalSpectrumTempAbsMin = 0;
  static const int thermalSpectrumTempMin    = 10;
  static const int thermalSpectrumTempMax    = 80;
  static const double thermalSpectrumHueStart = 300.0;
  static const double thermalSpectrumHueEnd   =   0.0;
  static const double thermalSpectrumHuePower = EXP;

private:
  sf::Mutex isRunningMutex;
  bool isRunning;

  RectTree<SensorReading> readingsTree;

  std::deque< sf::Vector2<int> > updatedPixels;

  double viewPitch;
  double viewYaw;
  bool isViewLockedToDeviceOrientation;
  int cursorX, cursorY;

  double prevThermalUpdateTime;

  sf::RenderTexture viewImage;

private:
  sf::Thread scannerThread;

  sf::Mutex sensorControlMutex;
  byte sensorUpdateDelay;

  sf::Mutex sensorDataMutex;
  unsigned char sensorData[8];

private:
  sf::Thread httpdThread;
  sf::Thread wsdThread;

  sf::Mutex drMutex;
  bool drIsConnected;
  double drAlpha, drBeta, drGamma;

private:
  sf::Font font;

public:
  ThermalScannerApp()
    : readingsTree(
        0.0, 0.0, 360.0, thermalObservedPitchRange, 4
      )
    , scannerThread(&ThermalScannerApp::scannerThreadFunc, this)
    , httpdThread(&ThermalScannerApp::httpdThreadFunc, this)
    , wsdThread(&ThermalScannerApp::wsdThreadFunc, this)
  {
    isRunning = true;

    viewPitch = thermalViewPitchRange/2.0;
    viewYaw = 180.0;
    isViewLockedToDeviceOrientation = false;

    prevThermalUpdateTime = 0.0;

    viewImage.create(windowWidth, windowHeight);

    for(int i = 0; i < 0; i++) {
      RectTree<SensorReading>::Rect rect(
        (double)rand()/(double)RAND_MAX*360.0,
        (double)rand()/(double)RAND_MAX*thermalObservedPitchRange,
        (double)rand()/(double)RAND_MAX*50.0,
        (double)rand()/(double)RAND_MAX*50.0,
        SensorReading((double)rand()/(double)RAND_MAX*30.0, 5)
      );
      readingsTree.add(rect);
    }

    if(!font.loadFromFile("Arial Black.ttf")) {
      throw std::runtime_error("Unable to load font!");
    }

    sensorUpdateDelay = 35;
    scannerThread.launch();

    httpdThread.launch();

    //wsdThread.launch();
    wsdThread.launch();
  }
  ~ThermalScannerApp()
  {
    if(1) {
      sf::Lock isRunningMutexLock(isRunningMutex);
      isRunning = false;
    }
    scannerThread.wait();
    httpdThread.wait();
    wsdThread.terminate();
  }

  void initRender(sf::RenderWindow &window)
  {
    window.setFramerateLimit(30);

    //image.create(thermalImageWidth, thermalImageHeight);
    //image.clear(sf::Color::Black);
  }

  bool onUpdate(double time, double deltaTime)
  {
    if(isViewLockedToDeviceOrientation) {
      if(1) {
        sf::Lock drMutexLock(drMutex);
        viewPitch = (drGamma)+90+thermalObservedPitchRange/2.0;
        viewYaw = fmod(drAlpha+180, 360);
      }
    } else {
      viewPitch = viewPitch+thermalViewMoveRate*deltaTime*(
          (sf::Keyboard::isKeyPressed(sf::Keyboard::Left ) ? -1.0 : 0.0)
        + (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ?  1.0 : 0.0)
      );
      viewYaw = viewYaw+thermalViewMoveRate*deltaTime*(
          (sf::Keyboard::isKeyPressed(sf::Keyboard::Left ) ? -1.0 : 0.0)
        + (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ?  1.0 : 0.0)
      );
    }
    viewPitch = clamp(viewPitch, 0.0, thermalViewPitchRange);
    viewYaw = fmod(viewYaw, 360.0);

    if(1) {
      if(time > prevThermalUpdateTime+sensorUpdateDelay/1000.0*1) {
        prevThermalUpdateTime = time;
        if(1) {
          sf::Lock sensorDataMutexLock(sensorDataMutex);
          sf::Lock drMutexLock(drMutex);
          if(!drIsConnected) {
            drAlpha += fmod(deltaTime*15, 360.0);
            drGamma = -90+20*sin(time*7);
          }
          for(int pixelI = 0; pixelI < 8; pixelI++) {
            double x = fmod(360-drAlpha+180, 360.0)-thermalPixelSizeYaw/2.0;
            double y = (drGamma)+90+thermalObservedPitchRange/2+(pixelI-4)*thermalPixelSizePitch;
            double w = thermalPixelSizeYaw, h = thermalPixelSizePitch;
            
            // RectTree<SensorReading>::Rect rect(
            //   (double)rand()/(double)RAND_MAX*(360.0-20.0),
            //   (double)rand()/(double)RAND_MAX*(thermalObservedPitchRange-20.0),
            //   (double)rand()/(double)RAND_MAX*20.0,
            //   (double)rand()/(double)RAND_MAX*20.0,
            //   SensorReading((double)rand()/(double)RAND_MAX*30.0, 5)
            // );
            RectTree<SensorReading>::Rect rect(
              x, y, w, h,
              SensorReading(sensorData[pixelI], time)
            );
            readingsTree.add(rect);

            double bpx = x/360.0*windowWidth;
            double bpy = y/thermalObservedPitchRange*windowHeight;
            double bpw = w/360.0*windowWidth;
            double bph = h/thermalObservedPitchRange*windowHeight;
            for(
              int px = floor(bpx/thermalViewPixelSize-2)*thermalViewPixelSize;
              px <= ceil((bpx+bpw)/thermalViewPixelSize+2)*thermalViewPixelSize;
              px += thermalViewPixelSize
            ) {
              for(
                int py = floor(bpy/thermalViewPixelSize-2)*thermalViewPixelSize;
                py <= ceil((bpy+bph)/thermalViewPixelSize+2)*thermalViewPixelSize;
                py += thermalViewPixelSize
              ) {
                updatedPixels.push_back(sf::Vector2<int>(px, py));
              }
            }
          }
        }
      }
    }
    return true;
  }

  void onKey(bool state, sf::Event &ev)
  {
    // 
  }
  void onMouseButton(bool state, sf::Event &ev)
  {
    // 
  }
  void onMouseMotion(sf::Event &ev)
  {
    fflush(stdout);
    cursorX = ev.mouseMove.x;
    cursorY = ev.mouseMove.y;
  }

  sf::Color getThermalPixelColor(double temp)
  {
    // TTT = fmod(TTT+0.1, 80.0);
    // temp = TTT;

    double scalar = (
        (clamp(temp, thermalSpectrumTempMin, thermalSpectrumTempMax)-thermalSpectrumTempMin)
      * (1.0/(double)(thermalSpectrumTempMax-thermalSpectrumTempMin))
    );
    scalar = weibull(scalar, 0.3, 2.6);
    //std::cout << temp << " = " << scalar << std::endl;
    double hue = thermalSpectrumHueStart+scalar*(thermalSpectrumHueEnd-thermalSpectrumHueStart);
    double sat = 1.0;
    double val = 1.0;
    if(temp < thermalSpectrumTempMin) {
      val = (
          clamp(temp, thermalSpectrumTempAbsMin, thermalSpectrumTempMin)
        * (1.0/(double)(thermalSpectrumTempMin-thermalSpectrumTempAbsMin))
      );
    }
    return hsv(hue, sat, val);
  }

  void onRender(sf::RenderWindow &window, double time, double deltaTime)
  {
    double thermalViewYawRange = thermalViewPitchRange/windowHeight*windowWidth;
    double vxmin = 0.0;//viewYaw-thermalViewYawRange/2.0;
    double vxmax = 360.0;//viewYaw+thermalViewYawRange/2.0;
    double vymin = 0;//viewPitch-thermalViewPitchRange/2.0;
    double vymax = thermalObservedPitchRange;//viewPitch+thermalViewPitchRange/2.0;

    double cursorTemp = 0.0;
    double cursorOverlap = 0.0;

    window.clear(sf::Color::Black);

    sf::RectangleShape pixel;
    //pixel.setSize(sf::Vector2f(thermalViewPixelSize, thermalViewPixelSize));

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
      viewImage.clear();
      for(int px = 0; px < windowWidth; px += thermalViewPixelSize) {
        for(int py = 0; py < windowHeight; py += thermalViewPixelSize) {
          updatedPixels.push_back(sf::Vector2<int>(px, py));
        }
      }
    } else if(sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
      /*std::vector<RectTree<SensorReading>::RectResult> resList;
      readingsTree.retrieveAllRects(resList);
      readingsTree.removeResults(resList);*/
      viewImage.clear();
      readingsTree.reset(0.0, 0.0, 360.0, thermalObservedPitchRange, 4);
    }

    //std::deque< sf::Vector2<int> >::iterator iter = updatedPixels.begin();
    auto iter = updatedPixels.begin();
    for(; iter != updatedPixels.end(); iter++) {
      int px = iter->x, py = iter->y;

      double vx = vxmin+(vxmax-vxmin)*((double)px/(double)windowWidth );
      double vy = vymin+(vymax-vymin)*((double)py/(double)windowHeight);
      double vw = thermalPixelSizeYaw;
      double vh = thermalPixelSizePitch;

      std::vector<RectTree<SensorReading>::RectResult> resList;
      readingsTree.findRectsAtPoint(vx, vy, resList);

      std::vector<RectTree<SensorReading>::RectResult> delList;

      pixel.setPosition(sf::Vector2f(px, py));
      pixel.setSize(sf::Vector2f(
        thermalViewPixelSize,
        thermalViewPixelSize //(double)thermalViewPixelSize/(double)windowWidth*(double)windowHeight
      ));

      double temp = 0.0; int count = 0; double sum = 0;
      RectTree<SensorReading>::RectResult *oldestRes = NULL; double oldestTime = 0.0;
      std::vector<RectTree<SensorReading>::RectResult>::iterator iter = resList.begin();
      for(; iter != resList.end(); iter++) {
        RectTree<SensorReading>::RectResult &res = *iter;
        double age = time-res.rect->value.time;
        /*if(res.rect->value.time < oldestTime || oldestRes == NULL) {
          oldestTime = res.rect->value.time;
          oldestRes = &res;
        }*/
        double ageScalar = 1.0/(1.0+age);
        temp += res.rect->value.temp*ageScalar;
        sum += ageScalar;
        count++;
      }
      if(count > 40) {
        delList.insert(delList.end(), resList.begin(), resList.begin()+10);
      }
      temp /= sum;
      if(count > 0) {
        pixel.setFillColor(getThermalPixelColor(temp));
      } else {
        pixel.setFillColor(sf::Color::Black);
      }
      viewImage.draw(pixel);
      if(
            cursorX > px && cursorX < px+thermalViewPixelSize
        &&  cursorY > py && cursorY < py+thermalViewPixelSize
      ) {
        cursorTemp = temp;
      }

      readingsTree.removeResults(delList);
    }
    int updatedPixelCount = updatedPixels.size();
    updatedPixels.clear();

    viewImage.display();
    sf::Sprite sprite(viewImage.getTexture());
    window.draw(sprite);


    if(0) {
      std::vector<RectTree<SensorReading>::RectResult> resList;
      readingsTree.retrieveAllRects(resList);

      std::vector<RectTree<SensorReading>::RectResult>::iterator iter = resList.begin();
      for(; iter != resList.end(); iter++) {
        RectTree<SensorReading>::RectResult &res = *iter;
        double px = res.rect->x/vxmax*windowWidth;
        double py = res.rect->y/vymax*windowHeight;
        double pw = res.rect->w/vxmax*windowWidth;
        double ph = res.rect->h/vymax*windowHeight;
        pixel.setFillColor(sf::Color(255*(double)res.rect->value.temp/30.0, 50, 50, 100));
        pixel.setPosition(sf::Vector2f(px, py));
        pixel.setSize(sf::Vector2f(pw, ph));
        sf::RenderStates rs;
        rs.blendMode = sf::BlendAlpha;
        window.draw(pixel);
      }
    }
    if(1) {
      pixel.setFillColor(sf::Color(255, 255, 255));
      std::vector<RectTree<SensorReading>::RectResult> resList;
      double cvx = (double)cursorX/(double)windowWidth*vxmax;
      double cvy = (double)cursorY/(double)windowHeight*vymax;
      readingsTree.findRectsAtPoint(cvx, cvy, resList);

      updatedPixels.push_back(sf::Vector2<int>(
        floor(cursorX/thermalViewPixelSize)*thermalViewPixelSize,
        floor(cursorY/thermalViewPixelSize)*thermalViewPixelSize
      ));
      
      std::vector<RectTree<SensorReading>::RectResult>::iterator iter = resList.begin();
      for(; iter != resList.end(); iter++) {
        RectTree<SensorReading>::RectResult &res = *iter;
        double px = res.rect->x/vxmax*windowWidth;
        double py = res.rect->y/vymax*windowHeight;
        double pw = res.rect->w/vxmax*windowWidth;
        double ph = res.rect->h/vymax*windowHeight;
        pixel.setPosition(sf::Vector2f(px, py));
        pixel.setSize(sf::Vector2f(pw, ph));
        window.draw(pixel);
        cursorOverlap++;
      }

      // RectTree<SensorReading>::Node &node = readingsTree.getNodeAtPoint(cvx, cvy);
      // pixel.setPosition(sf::Vector2f(node.x/vxmax*windowWidth, node.y/vymax*windowHeight));
      // pixel.setSize(sf::Vector2f(node.w/vxmax*windowWidth, node.h/vymax*windowHeight));
      // pixel.setFillColor(sf::Color::Green);
      // window.draw(pixel);
    }

    sf::Text text;
    text.setFont(font);
    std::stringstream ss;
    ss << viewYaw << ", " << viewPitch << "; "
       << drAlpha << ", " << drGamma
       << " -- " << cursorTemp << ", " << cursorOverlap
       << " -- " << 1/deltaTime << "FPS, " << updatedPixelCount
    ;
    text.setString(ss.str());
    text.setCharacterSize(24);
    text.setColor(sf::Color::White);

    window.draw(text);
    window.display();
  }

  void scannerThreadFunc()
  {
    std::cout << "[scanner] Connecting..." << std::endl;

    serial::SerialConnection sio(
      "/dev/tty.usbmodem1421", 9600,
      "/Users/Deco/workspace/sep/thermalprototype/test.log"
    );
    std::cout << "[scanner] Connected!" << std::endl;

    byte delay = 0;
    int pixelI = 0;
    unsigned char newSensorData[8];


    while(true) {
      if(1) {
        sf::Lock isRunningMutexLock(isRunningMutex);
        if(!isRunning) {
          break;
        }
      }
      if(1) {
        sf::Lock sensorControlMutexLock(sensorControlMutex);
        if(delay != sensorUpdateDelay) {
          delay = sensorUpdateDelay;
          serial::ByteArray requestData;
          requestData.push_back(255);
          requestData.push_back(delay);
          sio.SendData(requestData);
        }
      }

      serial::ByteArray res = sio.ReceiveData(1);
      if(res[0] == 255) {
        pixelI = 0;
      } else {
        newSensorData[pixelI] = res[0];
        pixelI++;
        if(pixelI > 7) {
          pixelI = 7;
          if(1) {
            sf::Lock sensorDataMutexLock(sensorDataMutex);
            //std::cout << "mmmm " << (int)newSensorData[3] << std::endl;
            memcpy(sensorData, newSensorData, 8);
          }
          sf::sleep(sf::milliseconds(1));
        }
      }
    }
  }

  void httpdThreadFunc()
  {
    HTTPServer httpd(8080, true);
    while(isRunning) {
      sf::sleep(sf::milliseconds(500));
    }
  }
  void wsdThreadFunc()
  {
    wsdFunc(drIsConnected, drAlpha, drBeta, drGamma, drMutex);
  }

  void getWindowMode(
    int &widthRef, int &heightRef, int &bppRef, int &flagsRef
  ) {
    widthRef = windowWidth; heightRef = windowHeight;
    bppRef = 32; //flagsRef = sf::Style::Fullscreen;
  }

  std::string getWindowTitle()
  {
    return "!";
  }
};

int main(int argCount, char *argList[])
{
  ThermalScannerApp app;

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

double weibull(double x, double l, double k) {
  return 1.0-exp(-pow(x/l, k));
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
