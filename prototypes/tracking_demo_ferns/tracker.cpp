
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
        
        double jitterPos = 25.0;
        double jitterSize = 25.0;
        double cx = x+w/2.0;
        double cy = y+h/2.0;
        w = clamp(w+drand(-jitterSize, jitterSize), (double)minBoxSize, (double)width );
        h = clamp(h+drand(-jitterSize, jitterSize), (double)minBoxSize, (double)height);
        
        cx = clamp(cx+drand(-jitterPos, jitterPos), w/2.0+1, ((double)width )-w/2.0-1);
        cy = clamp(cy+drand(-jitterPos, jitterPos), h/2.0+1, ((double)height)-h/2.0-1);
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
    frameCount = 0;
    
    srand(1337);
    for(int i = 0; i < fernCount; i++) {
        fernPosList.push_back(std::make_pair(
            cv::Vec2d(drand(0.0, 1.0), drand(0.0, 1.0)),
            cv::Vec2d(drand(0.0, 1.0), drand(0.0, 1.0))
        ));
    }
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
        world.createParticles(500);
    }
    
    BBox bestBox;
    double bestScore = 0.0;
    BBoxModel pMdl;
    cv::Mat pDebugImage;
    for(int i = 0; i < world.particleList.size(); i++) {
        frameCount++;
        //if((frameCount%2 == 0)^(i%2 == 0)) { continue; }
        Particle &p = world.particleList[i];
        fillBBoxModel(pMdl, frame, p.box, pDebugImage);
        //p.score = 1.0/(1.0+pow(getBBoxModelDistance(pMdl, referenceModel), 1));
        p.score = 1.0-getBBoxModelDistance(pMdl, referenceModel);
        if(p.score > bestScore) {
            bestScore = p.score;
            bestBox = p.box;
        }
    }
    
    world.breedParticles();
    world.updateParticles(0.0);
    
    return bestBox;
}

void Tracker::fillBBoxModel(BBoxModel &res, const cv::Mat &frame, const BBox &box, cv::Mat &img)
{
    cv::Rect rect;
    rect.x = box.x;
    rect.y = box.y;
    rect.width  = std::min(box.x+box.w, frame.cols-1)-box.x;
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
    //cv::resize(frame(rect), img, cv::Size(16, 16));
    img = frame(rect);
    
    res.fernDiffList.resize(fernCount*3);
    //res.fernDiffList.resize(fernCount);
    for(int fernI = 0; fernI < fernCount; fernI++) {
        cv::Vec2d &posAFrac = std::get<0>(fernPosList[fernI]);
        cv::Vec2d &posBFrac = std::get<1>(fernPosList[fernI]);
        
        //cv::Vec3b vA = img.at<cv::Vec3b>(img.rows*posAFrac[1]-1, img.cols*posAFrac[0]-1);
        //cv::Vec3b vB = img.at<cv::Vec3b>(img.rows*posBFrac[1]-1, img.cols*posBFrac[0]-1);
        cv::Vec2d posA(img.rows*posAFrac[1]-1, img.cols*posAFrac[0]-1);
        cv::Vec2d posB(img.rows*posBFrac[1]-1, img.cols*posBFrac[0]-1);
        double size = 5;
        cv::Mat roiA = img(
            cv::Range(std::max(0.0, posA[0]-size), std::min((double)img.rows-1, posA[0]+size)),
            cv::Range(std::max(0.0, posA[1]-size), std::min((double)img.cols-1, posA[1]+size))
        );
        cv::Mat roiB = img(
            cv::Range(std::max(0.0, posB[0]-size), std::min((double)img.rows-1, posB[0]+size)),
            cv::Range(std::max(0.0, posB[1]-size), std::min((double)img.cols-1, posB[1]+size))
        );
        cv::Mat1b maskA(img.rows, img.cols);
        cv::Scalar vA = cv::mean(roiA, cv::Mat1b(roiA.rows, roiA.cols));
        cv::Scalar vB = cv::mean(roiB, cv::Mat1b(roiB.rows, roiB.cols));
        // res.fernDiffList[fernI*3+0] = (vA[2] > vB[2]);
        // res.fernDiffList[fernI*3+1] = (vA[1] > vB[1]);
        // res.fernDiffList[fernI*3+2] = (vA[0] > vB[0]);
        for(int channelI = 0; channelI < 3; channelI++) {
            double sA = vA[2-channelI];//((double)vA[2])+((double)vA[1])+((double)vA[0]);
            double sB = vB[2-channelI];//((double)vB[2])+((double)vB[1])+((double)vB[0]);
            double similarRange = 10.0;
            res.fernDiffList[fernI*3+channelI] = (
                    std::abs(sA-sB) < similarRange ? 's'
                :   sA > sB                        ? 'a'
                :                                    'b'
            );
        }
    }
    
    res.colAvgList.resize(columnCount*3);
    res.rowAvgList.resize(rowCount*3);
    
    // int columnWidth = std::max(1, (int)img.cols/(int)columnCount);
    // int columnI = 0;
    // for(int bx = 0; bx < img.cols; bx += columnWidth) {
    //     columnI++;
    //     double rSum = 0.0, gSum = 0.0, bSum = 0.0;
    //     for(int px = bx; px < bx+columnWidth; px++) {
    //         for(int py = 0; py < img.rows; py++) {
    //             // if(px >= image.cols || py >= image.rows) { continue; }
    //             cv::Vec3b intensity = img.at<cv::Vec3b>(py, px);
    //             rSum += intensity.val[2];
    //             gSum += intensity.val[1];
    //             bSum += intensity.val[0];
    //         }
    //     }
    //     res.colAvgList[columnI*3+0] = (rSum/(columnWidth*img.rows));
    //     res.colAvgList[columnI*3+1] = (gSum/(columnWidth*img.rows));
    //     res.colAvgList[columnI*3+2] = (bSum/(columnWidth*img.rows));
    // }
    
    // int rowHeight = std::max(1, (int)img.rows/(int)rowCount);
    // int rowI = 0;
    // for(int by = 0; by < img.rows; by += rowHeight) {
    //     rowI++;
    //     double rSum = 0.0, gSum = 0.0, bSum = 0.0;
    //     for(int px = 0; px < img.cols; px++) {
    //         for(int py = by; py < by+rowHeight; py++) {
    //             // if(px >= image.cols || py >= image.rows) { continue; }
    //             cv::Vec3b intensity = img.at<cv::Vec3b>(py, px);
    //             rSum += intensity.val[2];
    //             gSum += intensity.val[1];
    //             bSum += intensity.val[0];
    //         }
    //     }
    //     res.rowAvgList[rowI*3+0] = (rSum/(rowHeight*img.cols));
    //     res.rowAvgList[rowI*3+1] = (gSum/(rowHeight*img.cols));
    //     res.rowAvgList[rowI*3+2] = (bSum/(rowHeight*img.cols));
    // }
}

double Tracker::getBBoxModelDistance(const BBoxModel &modelA, const BBoxModel &modelB)
{
    double dist;
    
    int diffCount = 0;
    for(int fernI = 0; fernI < modelA.fernDiffList.size(); fernI++) {
        if(modelA.fernDiffList[fernI] != modelB.fernDiffList[fernI]) {
            diffCount++;
        }
    }
    dist = pow((double)diffCount/3.0/(double)fernCount, 1/3.0);
    
    // double sum = 0.0;
    
    // const std::vector<double> &acList = modelA.colAvgList;
    // const std::vector<double> &bcList = modelB.colAvgList;
    // //assert(aList.size() == bList.size());
    // for(int i = 0; i < acList.size(); i++) {
    //     sum += abs(acList[i]-bcList[i])
    // }
    
    // const std::vector<double> &arList = modelA.rowAvgList;
    // const std::vector<double> &brList = modelB.rowAvgList;
    // //assert(aList.size() == bList.size());
    // for(int i = 0; i < arList.size(); i++) {
    //     sum += abs(arList[i]-brList[i])
    // }
    
    // //sum = sqrt(sum);
    
    return dist;
}



