
#include "util.h"

#include "tracker.h"

#include <iostream>

Particle::Particle()
    : box()
{
    score = 0;
}
Particle::Particle(const Particle &source)
{
    box = source.box;
    score = source.score;
}

ParticleWorld::ParticleWorld(int _width, int _height, int _minBoxSize)
{
    width = _width; _height = height; minBoxSize = _minBoxSize;
}

void ParticleWorld::setRandParticle(Particle &p)
{
    int maxBoxWidth = 200;//width-1-minBoxSize;
    int maxBoxHeight = 200;//height-1-minBoxSize;
    double aspect = drand(1.0/1.6, 1.6);
    p.box.w = irand(minBoxSize, std::min(maxBoxWidth, width -p.box.x));
    p.box.h = clamp(((double)p.box.w)*aspect, ((double)minBoxSize), std::min((double)maxBoxHeight, ((double)height-p.box.y)));
    p.box.x = irand(0, width -p.box.w-1);
    p.box.y = irand(0, height-p.box.h-1);
}

void ParticleWorld::createParticles(int count)
{
    particleList.resize(count);
    for(int i = 0; i < count; i++) {
        setRandParticle(particleList[i]);
        // std::cout << "P: "
        //           << particleList[i].box.x << ","
        //           << particleList[i].box.y << ","
        //           << particleList[i].box.w << ","
        //           << particleList[i].box.h << std::endl
        // ;
    }
}

void ParticleWorld::breedParticles()
{
    std::vector<Particle> newParticleList;
    double scoreSum = 0.0;
    for(int i = 0; i < particleList.size(); i++) {
        scoreSum += particleList[i].score;
    }
    int desiredParticleCount = particleList.size();
    for(int newParticleI = 0; newParticleI < desiredParticleCount; newParticleI++) {
        Particle newParticle;
        
        //double createNewScore = 1-scoreSum;
        //std::cout << scoreSum << "; " << createNewScore << std::endl;
        //double selection = drand(0, scoreSum+createNewScore);
        //if(selection > scoreSum) {
        //    setRandParticle(newParticle);
        //} else {
        
        if(drand(0.0, 1.0) < 0.03) {
            setRandParticle(newParticle);
        } else {
            double selection = drand(0, scoreSum);
            double total = 0;
            for(int oldParticleI = 0; oldParticleI < particleList.size(); oldParticleI++){
                total += particleList[oldParticleI].score;
                if (total > selection) {
                    newParticle = particleList[oldParticleI];
                    break;
                }
            }
        }
        newParticleList.push_back(newParticle);
    }
    particleList = newParticleList;
}

void ParticleWorld::updateParticles(double deltaTime)
{
    for(int i = 0; i < particleList.size(); i++) {
        double x = particleList[i].box.x;
        double y = particleList[i].box.y;
        double w = particleList[i].box.w;
        double h = particleList[i].box.h;
        
        double jitter = 20.0;
        double cx = x+w/2.0;
        double cy = y+h/2.0;
        w = clamp(w+drand(-jitter, jitter), (double)minBoxSize, (double)width );
        h = clamp(h+drand(-jitter, jitter), (double)minBoxSize, (double)height);
        
        cx = clamp(cx+drand(-jitter, jitter), w/2.0+1, ((double)width )-w/2.0-1);
        cy = clamp(cy+drand(-jitter, jitter), h/2.0+1, ((double)height)-h/2.0-1);
        particleList[i].box.x = cx-w/2.0;
        particleList[i].box.y = cy-h/2.0;
        particleList[i].box.w = w;
        particleList[i].box.h = h;
        
        if(particleList[i].box.x < 0) { std::cout << "WTF1" << std::endl; }
        if(particleList[i].box.y < 0) { std::cout << "WTF2" << std::endl; }
        if(particleList[i].box.x+particleList[i].box.w >= width) { std::cout << "WTF3" << std::endl; }
        if(particleList[i].box.y+particleList[i].box.h >= height) { std::cout << "WTF4" << std::endl; }
    }
}


Tracker::Tracker()
    : world(100, 100, 10)
{
    isWorldSetup = false;
}
Tracker::~Tracker()
{
    // 
}

BBox Tracker::onFrame(double time, const cv::Mat &frame)
{
    if(!isWorldSetup) {
        isWorldSetup = true;
        world.width = frame.cols;
        world.height = frame.rows;
        world.minBoxSize = 20;
        world.createParticles(120);
    }
    
    BBox bestBox;
    double bestScore = 0.0;
    for(int i = 0; i < world.particleList.size(); i++) {
        Particle &p = world.particleList[i];
        cv::Mat pDebugImage;
        BBoxModel pMdl = getBBoxModel(frame, p.box, pDebugImage);
        p.score = 1.0/(1.0+pow(getBBoxModelDistance(pMdl, referenceModel), 1));
        if(p.score > bestScore) {
            bestScore = p.score;
            bestBox = p.box;
        }
    }
    
    world.breedParticles();
    world.updateParticles(0.0);
    
    return bestBox;
}

BBoxModel Tracker::getBBoxModel(const cv::Mat &frame, const BBox &box, cv::Mat &img)
{
    cv::Rect rect;
    rect.x = box.x;
    rect.y = box.y;
    rect.width = std::min(box.x+box.w, frame.cols-1)-box.x;
    rect.height = std::min(box.y+box.h, frame.rows-1)-box.y;
    //cv::Mat img = frame(rect);
    // std::cout << "B: "
    //           << box.x << ","
    //           << box.y << ","
    //           << box.w << ","
    //           << box.h << std::endl
    // ;
    // std::cout << "R: "
    //           << rect.x      << ","
    //           << rect.y      << ","
    //           << rect.width  << ","
    //           << rect.height << std::endl
    // ;
    img = frame(rect);
    
    BBoxModel res;
    int columnWidth = std::max(1, (int)img.cols/(int)columnCount);
    for(int bx = 0; bx < img.cols; bx += columnWidth) {
        double rSum = 0.0, gSum = 0.0, bSum = 0.0;
        for(int px = bx; px < bx+columnWidth; px++) {
            for(int py = 0; py < img.rows; py++) {
                // if(px >= image.cols || py >= image.rows) { continue; }
                cv::Vec3b intensity = img.at<cv::Vec3b>(py, px);
                rSum += intensity.val[2];
                gSum += intensity.val[1];
                bSum += intensity.val[0];
            }
        }
        res.colAvgList.push_back(rSum/(columnWidth*img.rows));
        res.colAvgList.push_back(gSum/(columnWidth*img.rows));
        res.colAvgList.push_back(bSum/(columnWidth*img.rows));
    }
    
    int rowHeight = std::max(1, (int)img.rows/(int)rowCount);
    for(int by = 0; by < img.rows; by += rowHeight) {
        double rSum = 0.0, gSum = 0.0, bSum = 0.0;
        for(int px = 0; px < img.cols; px++) {
            for(int py = by; py < by+rowHeight; py++) {
                // if(px >= image.cols || py >= image.rows) { continue; }
                cv::Vec3b intensity = img.at<cv::Vec3b>(py, px);
                rSum += intensity.val[2];
                gSum += intensity.val[1];
                bSum += intensity.val[0];
            }
        }
        res.rowAvgList.push_back(rSum/(rowHeight*img.cols));
        res.rowAvgList.push_back(gSum/(rowHeight*img.cols));
        res.rowAvgList.push_back(bSum/(rowHeight*img.cols));
    }
    
    return res;
}

double Tracker::getBBoxModelDistance(const BBoxModel &modelA, const BBoxModel &modelB)
{
    double sum = 0.0;
    
    const std::vector<double> &acList = modelA.colAvgList;
    const std::vector<double> &bcList = modelB.colAvgList;
    //assert(aList.size() == bList.size());
    for(int i = 0; i < acList.size(); i++) {
        sum += pow(abs(acList[i]-bcList[i]), 1.0);
    }
    
    const std::vector<double> &arList = modelA.rowAvgList;
    const std::vector<double> &brList = modelB.rowAvgList;
    //assert(aList.size() == bList.size());
    for(int i = 0; i < arList.size(); i++) {
        sum += pow(abs(arList[i]-brList[i]), 1.0);
    }
    
    //sum = sqrt(sum);
    
    return sum;
}



