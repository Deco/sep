#include <SFML/Graphics.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "tracker.h"

#ifndef APP_H
#define APP_H

class TrackingApp {
private:
    static const int windowWidth = 1440;
    static const int windowHeight = 600;//900;

private:
    int cursorX, cursorY;
    sf::Font font;

private:
    sf::Thread aviThread;
    
    bool isReady;
    bool isPlaying;
    int queueFrameCount;
    sf::Mutex frameMutex;
    cv::Mat frame;

private:
    sf::Texture frameTexture;
    bool isFrameTextureDirty;

private:
    bool isShowingRows;
    bool isDraggingBox;
    BBox testBox;
    bool isReferencePresent;
    BBox referenceBox;
    cv::Mat referenceImage;
    bool isTrackerSetup;
    Tracker tk;
    int trackQueueCount;
    double prevTrackTime;
    bool isTrackingPlaying;
    int selectedChannelI;

public:
    TrackingApp();
    ~TrackingApp();
    
    void aviThreadRun();
    
    void initRender(sf::RenderWindow &window);
    void onRender(sf::RenderWindow &window, double time, double deltaTime);
    void drawHistogram(sf::RenderWindow &window, const BBox &loc, const BBoxModel &mdl);
    
    bool onUpdate(double time, double deltaTime);
    
    void onKey(bool state, sf::Event &ev);
    void onMouseButton(bool state, sf::Event &ev);
    void onMouseMotion(sf::Event &ev);
    
    void getWindowMode(
        int &widthRef, int &heightRef, int &bppRef, int &flagsRef
    );
    
    std::string getWindowTitle();
};

#endif // APP_H
