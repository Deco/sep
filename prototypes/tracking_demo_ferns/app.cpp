#include <stdexcept>
#include <cstdio>

#include "app.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define clamp(v, m0, m1) (min(max((v), (m0)), (m1)))
sf::Color hsv(int hue, float sat, float val);

void copyMatToTexture(const cv::Mat &mat, sf::Texture &tex, bool toGray=false);

TrackingApp::TrackingApp()
    : aviThread(&TrackingApp::aviThreadRun, this)
{
    aviThread.launch();
    
    isTrackerSetup = false;
    trackQueueCount = 0;
    
    prevTrackTime = 0.0;
    isReady = false;
    isPlaying = true;
    queueFrameCount = 0;
    isReferencePresent = false;
    selectedChannelI = 0;
    
    isDraggingBox = false;
    isShowingRows = false;
    testBox.x = 50;  testBox.y = 50;
    testBox.w = 100; testBox.h = 100;
    
    if(!font.loadFromFile("Arial Black.ttf")) {
      throw std::runtime_error("Unable to load font!");
    }
}
TrackingApp::~TrackingApp()
{
    // 
}

void TrackingApp::initRender(sf::RenderWindow &window)
{
    window.setFramerateLimit(30);
}
void TrackingApp::onRender(sf::RenderWindow &window, double time, double deltaTime)
{
    if(!isReady) { return; }
    window.clear();
    double dist = 0.0;
    if(1) {
        if(isFrameTextureDirty) {
            sf::Lock frameMutexLock(frameMutex);
            copyMatToTexture(frame, frameTexture);
            isFrameTextureDirty = false;
            //std::cout << "redraw" << std::endl;
            
            if(isReferencePresent) {
                double delay = 0.1;
                if(trackQueueCount > 0 || (isTrackingPlaying && time > prevTrackTime+delay)) {
                    //std::cout << "!" << std::endl;
                    trackQueueCount--;
                    prevTrackTime = time;
                    testBox = tk.onFrame(time, frame);
                }
            }
        }
        
        sf::Sprite sprite;
        sprite.setTexture(frameTexture);
        sprite.setScale(1, 1);
        
        window.draw(sprite);
    }
    
    if(1) {
        for(int i = 0; i < tk.world.particleList.size(); i++) {
            BBox &loc = tk.world.particleList[i].box;
            sf::RectangleShape rectangle;
            rectangle.setPosition(
                (double)loc.x,
                (double)loc.y
            );
            rectangle.setSize(sf::Vector2f(
                (double)loc.w,
                (double)loc.h
            ));
            rectangle.setFillColor(sf::Color::Transparent);
            //rectangle.setOutlineColor(sf::Color::White);
            rectangle.setOutlineColor(
                sf::Color(255, 255, 255, tk.world.particleList[i].score*255)
            );
            rectangle.setOutlineThickness(1);
            window.draw(rectangle);
        }
    }
    
    
    if(1) {
        cv::Mat testBoxImage;
        BBoxModel mdl;
        if(1) {
            sf::Lock frameMutexLock(frameMutex);
            tk.fillBBoxModel(mdl, frame, testBox, testBoxImage);
        }
        
        sf::Texture boxImageTexture;
        copyMatToTexture(testBoxImage, boxImageTexture, true);
        sf::Sprite sprite;
        sprite.setTexture(boxImageTexture);
        sf::Vector2u textureSize = boxImageTexture.getSize();
        sprite.setPosition(
            testBox.x+20,
            testBox.y
        );
        window.draw(sprite);
        drawHistogram(window, testBox, mdl);
        
        if(isReferencePresent) {
            
            sf::Texture boxImageTexture;
            copyMatToTexture(referenceImage, boxImageTexture);
            sf::RectangleShape sprite;
            sprite.setTexture(&boxImageTexture);
            //sf::Vector2u textureSize = boxImageTexture.getSize();
            sf::Vector2f textureSize(100, 100);
            sprite.setSize(textureSize);
            sprite.setPosition(
                (double)windowWidth-textureSize.x-10,
                (double)windowHeight-textureSize.y-10
            );
            window.draw(sprite);
            BBox loc;
            loc.x = (double)windowWidth-textureSize.x-10;
            loc.y = (double)windowHeight-textureSize.y*2-20;
            loc.w = textureSize.x; loc.h = textureSize.y;
            drawHistogram(window, loc, tk.referenceModel);
            
            dist = tk.getBBoxModelDistance(mdl, tk.referenceModel);
            /*sf::Text text;
            text.setFont(font);
            std::stringstream ss;
            ss << "dist: " << tk.getBBoxModelDistance(mdl, tk.referenceModel);
            text.setString(ss.str());
            text.setCharacterSize(24);
            text.setColor(sf::Color::White);
            text.setPosition(
                (double)windowWidth-textureSize.x-10,
                (double)windowHeight-textureSize.y*2-24-30
            );
            window.draw(text);*/
        }
    }
    if(1) {
        sf::Text text;
        text.setFont(font);
        std::stringstream ss;
        ss << (
                selectedChannelI == 0 ? "Red"
            :   selectedChannelI == 1 ? "Green"
            :   selectedChannelI == 2 ? "Blue"
            :   "wat"
        );
        ss << " - ";
        ss << dist;
        text.setString(ss.str());
        text.setCharacterSize(24);
        text.setPosition(4, 4);
        text.setColor(sf::Color::Black);
        window.draw(text);
        text.setPosition(2, 2);
        text.setColor(sf::Color::White);
        window.draw(text);
    }
    
    window.display();
}

void TrackingApp::drawHistogram(sf::RenderWindow &window, const BBox &loc, const BBoxModel &mdl)
{
    sf::RectangleShape rectangle;
    rectangle.setPosition((double)loc.x, (double)loc.y);
    rectangle.setSize(sf::Vector2f((double)loc.w, (double)loc.h));
    rectangle.setFillColor(sf::Color::Transparent);
    rectangle.setOutlineColor(sf::Color::Green);
    rectangle.setOutlineThickness(1);
    window.draw(rectangle);
    
    for(int fernI = 0; fernI < mdl.fernDiffList.size(); fernI++) {
        cv::Vec2d &posAFrac = std::get<0>(tk.fernPosList[fernI]);
        cv::Vec2d &posBFrac = std::get<1>(tk.fernPosList[fernI]);
        sf::CircleShape shape(4);
        sf::Color col = hsv(360.0*((double)fernI)/(double)tk.fernCount, 1.0, 1.0);
        shape.setOutlineColor(col);
        shape.setOutlineThickness(1);
        char c = mdl.fernDiffList[fernI*3+selectedChannelI];
        shape.setFillColor(
                (c == 'a' || c == 's') ? col
            : sf::Color::Transparent
        );
        shape.setPosition(loc.x+loc.w*posAFrac[0], loc.y+loc.h*posAFrac[1]);
        window.draw(shape);
        shape.setFillColor(
                (c == 'b' || c == 's') ? col
            : sf::Color::Transparent
        );
        shape.setPosition(loc.x+loc.w*posBFrac[0], loc.y+loc.h*posBFrac[1]);
        window.draw(shape);
    }
    return;
    if(isShowingRows) {
        for(int i = 0; i < tk.rowCount; i++) {
            sf::RectangleShape rectangle;
            rectangle.setPosition(
                (double)loc.x,
                (double)loc.y+(double)loc.h/(double)tk.rowCount*(double)i
            );
            rectangle.setSize(sf::Vector2f(
                (double)loc.w,
                (double)loc.h/(double)tk.rowCount
            ));
            rectangle.setFillColor(sf::Color::Transparent);
            rectangle.setOutlineColor(sf::Color::Green);
            rectangle.setOutlineThickness(1);
            
            window.draw(rectangle);
        }
        int count = tk.rowCount*3;
        for(int i = 0; i < count && i < mdl.rowAvgList.size(); i++) {
            sf::RectangleShape colShape;
            colShape.setPosition(
                (double)loc.x,
                (double)loc.y+(double)loc.h/(double)count*(double)i
            );
            colShape.setSize(sf::Vector2f(
                (double)loc.w*mdl.rowAvgList[i]/255.0,
                (double)loc.h/(double)count
            ));
            colShape.setFillColor(
                    i%3 == 0 ? sf::Color::Red
                :   i%3 == 1 ? sf::Color::Green
                :              sf::Color::Blue
            );
            
            window.draw(colShape);
        }
    } else {
        for(int i = 0; i < tk.columnCount; i++) {
            sf::RectangleShape rectangle;
            rectangle.setPosition(
                (double)loc.x+(double)loc.w/(double)tk.columnCount*(double)i,
                (double)loc.y
            );
            rectangle.setSize(sf::Vector2f(
                (double)loc.w/(double)tk.columnCount,
                (double)loc.h
            ));
            rectangle.setFillColor(sf::Color::Transparent);
            rectangle.setOutlineColor(sf::Color::Green);
            rectangle.setOutlineThickness(1);
            
            window.draw(rectangle);
        }
        int count = tk.columnCount*3;
        for(int i = 0; i < count && i < mdl.colAvgList.size(); i++) {
            sf::RectangleShape colShape;
            colShape.setPosition(
                (double)loc.x+(double)loc.w/(double)count*(double)i,
                (double)loc.y+(double)loc.h*(1.0-mdl.colAvgList[i]/255.0)
            );
            colShape.setSize(sf::Vector2f(
                (double)loc.w/(double)count,
                (double)loc.h*mdl.colAvgList[i]/255.0
            ));
            colShape.setFillColor(
                    i%3 == 0 ? sf::Color::Red
                :   i%3 == 1 ? sf::Color::Green
                :              sf::Color::Blue
            );
            
            window.draw(colShape);
        }
    }
}

bool TrackingApp::onUpdate(double time, double deltaTime)
{
    // 
    return true;
}

void TrackingApp::onKey(bool state, sf::Event &ev)
{
    if(ev.key.code == sf::Keyboard::Space) {
        if(state) {
            isPlaying = !isPlaying;
            queueFrameCount = 0;
        }
    }
    if(ev.key.code == sf::Keyboard::Right) {
        if(state) {
            queueFrameCount++;
        }
    }
    if(ev.key.code == sf::Keyboard::R) {
        if(state) {
            isShowingRows = !isShowingRows;
        }
    }
    if(ev.key.code == sf::Keyboard::E) {
        if(state) {
            referenceBox = testBox;
            cv::Mat boxImage;
            if(1) {
                sf::Lock frameMutexLock(frameMutex);
                tk.fillBBoxModel(
                    tk.referenceModel,
                    frame, referenceBox, boxImage
                );
            }
            referenceImage = boxImage.clone();
            isReferencePresent = true;
        }
    }
    if(ev.key.code == sf::Keyboard::T) {
        if(state) {
            trackQueueCount++;
        }
    }
    if(ev.key.code == sf::Keyboard::Y) {
        if(state) {
            isTrackingPlaying = !isTrackingPlaying;
        }
    }
    if(ev.key.code == sf::Keyboard::Num1 && state) { selectedChannelI = 0; }
    if(ev.key.code == sf::Keyboard::Num2 && state) { selectedChannelI = 1; }
    if(ev.key.code == sf::Keyboard::Num3 && state) { selectedChannelI = 2; }
}
void TrackingApp::onMouseButton(bool state, sf::Event &ev)
{
    if(ev.mouseButton.button == sf::Mouse::Left) {
        if(state) {
            isDraggingBox = true;
            testBox.x = ev.mouseButton.x;
            testBox.y = ev.mouseButton.y;
        } else {
            isDraggingBox = false;
        }
    }
}
void TrackingApp::onMouseMotion(sf::Event &ev)
{
    cursorX = ev.mouseMove.x;
    cursorY = ev.mouseMove.y;
    if(isDraggingBox) {
        testBox.w = max(50, cursorX-testBox.x);
        testBox.h = max(50, cursorY-testBox.y);
    }
}

void TrackingApp::getWindowMode(
    int &widthRef, int &heightRef, int &bppRef, int &flagsRef
) {
    widthRef = windowWidth; heightRef = windowHeight;
    bppRef = 32; //flagsRef = sf::Style::Fullscreen;
}

std::string TrackingApp::getWindowTitle()
{
    return "!";
}

void TrackingApp::aviThreadRun()
{
    sf::Clock clock;
    sf::Time currTime = clock.getElapsedTime();
    sf::Time nextFrameTime = currTime;
    sf::Time frameDelay = sf::seconds(1.0/60.0);
    
    cv::VideoCapture inVideo(0);
    //inVideo.open("/Users/Deco/workspace/tom/capture.avi");
    
    if(!inVideo.isOpened()) {
        throw std::runtime_error(
            "[avi][ERROR]Video could not be opened\n"
        );
    }
    
    while(true) {
        //std::cout << (clock.getElapsedTime()-currTime).asSeconds() << std::endl;
        currTime = clock.getElapsedTime();
        bool shouldRead = (isPlaying || queueFrameCount > 0);
        if(currTime >= nextFrameTime && shouldRead) {
            nextFrameTime = (currTime + frameDelay);
            if(1) {
                sf::Lock frameMutexLock(frameMutex);
                if(!inVideo.read(frame)) {
                    break;
                }
                //std::cout << "frame" << std::endl;
                
                isFrameTextureDirty = true;
                isReady = true;
            }
            queueFrameCount--;
        }
        sf::sleep(frameDelay);
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
    cv::cvtColor(
        mat, continuousRBGA,
        (toGray ? CV_BGR2GRAY : CV_BGR2RGBA), 4
    ); 
    
    tempImage.create(mat.cols, mat.rows, &imgData[0]);
    
    tex.loadFromImage(tempImage);
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
