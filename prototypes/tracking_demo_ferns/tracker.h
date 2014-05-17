#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifndef TRACKER_H
#define TRACKER_H

struct BBox {
public:
    int x, y, w, h;
};

struct BBoxModel {
public:
    std::vector<double> colAvgList;
    std::vector<double> rowAvgList;
    std::vector<char> fernDiffList;
};


class Particle {
public:
    BBox box;
    double score;

public:
    Particle();
    Particle(const Particle &source);

};

class ParticleWorld {
public:
    int width, height;
    int minBoxSize;
    std::vector<Particle> particleList;

public:
    ParticleWorld(int _width, int _height, int _minBoxSize);
    
    void createParticles(int count);
    void breedParticles();
    void updateParticles(double deltaTime);
    void setRandParticle(Particle &p);
};

class Tracker {
public:
    typedef std::pair<cv::Vec2d, cv::Vec2d> FernPos;
    
public:
    static const int columnCount = 15;
    static const int rowCount = 15;
    static const int fernCount = 20;

public:
    bool isWorldSetup;
    ParticleWorld world;
    BBoxModel referenceModel;
    
    cv::Mat prevFrame;
    unsigned long frameCount;
    
    std::vector<FernPos> fernPosList;

public:
    Tracker();
    ~Tracker();
    
    BBox onFrame(double time, const cv::Mat &frame);
    
    void fillBBoxModel(BBoxModel &mdl, const cv::Mat &frame, const BBox &box, cv::Mat &img);
    double getBBoxModelDistance(const BBoxModel &modelA, const BBoxModel &modelB);
    
};

#endif // TRACKER_H


