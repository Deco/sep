#include "SensorSamplerRealm.h"

/// Author S.Holmes.
/// Constructor
SensorSamplerRealm::SensorSamplerRealm(
    cv::Vec2d _minPos, cv::Vec2d _maxPos, cv::Vec2d _readingSize
)   : minPos(_minPos) 
    , maxPos(_maxPos)
    , readingSize(_readingSize)
    , readingList()
{
    //
}

/// Author S. Holmes
/// Public function for adding a reading from the sensor to the
/// list. Reading has a matrix of single channel values representing
/// temperatures. Also takes a position and creation time of the data.
void SensorSamplerRealm::addReadingWindow(
    cv::Mat_<float> &matRef,
    cv::Vec2d pos,
    double time
) {
    cv::Vec2d windowSize = readingSize*cv::Vec2d((double)matRef.cols, (double)matRef.rows);
    cv::Vec2d windowPos = pos-windowSize/2.0;
    
    for(int colI = 0; colI < matRef.cols; colI++) {
        for(int rowI = 0; rowI < matRef.rows; rowI++) {
            
            readingList.emplace_back(
                windowPos + readingSize*cv::Vec2d((double)colI, (double)rowI),
                matRef(rowI, colI),
                time
            );
            
        }
    }
}

/// Author S.Holmes Modified by D.White
/// Called by the SFML app/UI to re-populate it's image
/// with updated values from the list/quad-tree. Passes a function
/// reference that sets the colour values of the pixels, depending
/// on user parameter.
void SensorSamplerRealm::updateViewWindow(
    SensorViewWindow &viewWindowRef,
    std::function<void (cv::Mat &pixelRef, const float &temp)> setPixelColorFunc
) {
    cv::Mat_<float> &viewImgRef = viewWindowRef.img;
    cv::Vec2d &viewPosRef = viewWindowRef.currPos;
    cv::Vec2d &viewSizeRef = viewWindowRef.currSize;
    
    /// Invalidated means the window has moved, and we need to clear
    /// all existing pixels and re-populate, or modify for re-use.
    bool isViewWindowInvalidated = false;
    if(viewWindowRef.newPos != viewPosRef) {
        viewPosRef = viewWindowRef.newPos;
        isViewWindowInvalidated = true;
    }
    if(viewWindowRef.newSize != viewSizeRef) {
        viewSizeRef = viewWindowRef.newSize;
        isViewWindowInvalidated = true;
    }
    if(isViewWindowInvalidated) {
        /// either clear all pixels, or shift existing pixels for reuse
    }
    
    cv::Vec2d viewCellSize = viewSizeRef / cv::Vec2d((double)viewSizeRef.cols, (double)viewSizeRef.rows);
    

    /// Currently just re-populates the all the pixels in the window.
    /// TODO: change it to handle re-using existing pixels to
    /// optimize the image construction. Also still only uses a list instead
    /// of quad-tree.
    for(int viewColI = 0; viewColI < viewImgRef.cols; viewColI++) {
        for(int viewRowI = 0; viewRowI < viewImgRef.rows; viewRowI++) {
            cv::Rect cellBounds(
                viewWindowRef.pos + viewCellSize*cv::Vec2d((double)viewColI, (double)viewRowI),
                viewCellSize
            )
            
            double readingTempSum = 0.0;
            int readingCount = 0;
            for(ReadingInfo &reading : readingList) {
                cv::Rect readingBounds(reading.pos, readingSize);
                if((cellBounds & readingBounds).area() != 0) {
                    readingTempSum += reading.temp;
                    readingCount++;
                }
            }
            
            double readingTempAverage = (readingCount == 0 ? 0 : readingTempSum/(double)readingCount);
            
            /// Calls the function reference.
            setPixelColorFunc(viewImgRef(viewRowI, viewColI), readingTempAverage);
        }
    }
}


