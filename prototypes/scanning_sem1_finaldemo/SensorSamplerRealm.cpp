#include "SensorSamplerRealm.h"

/// Author S.Holmes.
/// Constructor
SensorSamplerRealm::SensorSamplerRealm(
    cv::Vec2d _minPos, cv::Vec2d _maxPos, cv::Vec2d _readingSize
)   : minPos(_minPos) 
    , maxPos(_maxPos)
    , readingSize(_readingSize)
    , readingList()
    , newReadingList()
    , readingTree(
        -150.0, -90.0,
        300.0, 180.0,
        6
      )
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
    cv::Vec2d windowSize = readingSize.mul(cv::Vec2d((double)matRef.cols, (double)matRef.rows));
    cv::Vec2d windowPos = pos-windowSize/2.0;
    
    for(int colI = 0; colI < matRef.cols; colI++) {
        for(int rowI = 0; rowI < matRef.rows; rowI++) {
            cv::Vec2d readingPos = windowPos + readingSize.mul(cv::Vec2d((double)colI, (double)rowI));
            float temp = matRef(rowI, colI);
            
            if(temp > -999.0) {
                ReadingInfo reading(readingPos, temp, time);

                //readingList.push_back(reading);

                // DW: hack hack hack hack
                newReadingList.push_back(reading);
                
                RectTree<ReadingInfo>::Rect rect(
                    readingPos[0], readingPos[1],
                    readingSize[0], readingSize[1],
                    reading
                );
                readingTree.add(rect);
            }
        }
    }
}

// DW: hack hack hack
struct compareVec2i {
    bool operator()(const cv::Vec2i &a, const cv::Vec2i &b) {
        return a[0] < b[0] or a[1] < b[1];
    }
};

/// Author S.Holmes Modified by D.White
/// Called by the SFML app/UI to re-populate it's image
/// with updated values from the list/quad-tree. Passes a function
/// reference that sets the colour values of the pixels, depending
/// on user parameter.
void SensorSamplerRealm::updateViewWindow(
    SensorViewWindow &viewWindowRef,
    PixelColorFunc setPixelColorFunc
) {
    cv::Mat &viewImgRef = viewWindowRef.img;
    cv::Vec2d &viewPosRef = viewWindowRef.currPos;
    cv::Vec2d &viewSizeRef = viewWindowRef.currSize;
    
    /// Invalidated means the window has moved, and we need to clear
    /// all existing pixels and re-populate, or modify for re-use.
    bool isViewWindowInvalidated = false;
    if(viewWindowRef.forcedInvalid) {
        viewWindowRef.forcedInvalid = false;
        isViewWindowInvalidated = true;
    }
    if(viewWindowRef.newPos != viewPosRef) {
        viewPosRef = viewWindowRef.newPos;
        isViewWindowInvalidated = true;
    }
    if(viewWindowRef.newSize != viewSizeRef) {
        viewSizeRef = viewWindowRef.newSize;
        isViewWindowInvalidated = true;
    }
    if(isViewWindowInvalidated) {
        viewImgRef = cv::Scalar(0);
    }
    
    cv::Vec2d imgResolution = cv::Vec2d((double)viewImgRef.cols, (double)viewImgRef.rows);
    cv::Vec2d viewCellSize = cv::Vec2d(viewSizeRef[0]/imgResolution[0], viewSizeRef[1]/imgResolution[1]);

    /* DW: HACK HACK HACK */
    std::set<cv::Vec2i, compareVec2i> pixelsToUpdate;
    for(ReadingInfo &reading : newReadingList) {
        cv::Vec2d tl = reading.pos-viewPosRef;
        cv::Vec2d br = tl+readingSize;
        int colMin = (int)floor(tl[0]/viewCellSize[0]), colMax = (int)floor(br[0]/viewCellSize[0]);
        int rowMin = (int)floor(tl[1]/viewCellSize[1]), rowMax = (int)floor(br[1]/viewCellSize[1]);

        for(int viewColI = colMin; viewColI < colMax; viewColI++) {
            for(int viewRowI = rowMin; viewRowI < rowMax; viewRowI++) {
                pixelsToUpdate.emplace(viewColI, viewRowI);
            }
        }
    }
    newReadingList.clear();
    /* cbf fixing it :P */

    /// Currently just repopulates the all the pixels in the window.
    /// TODO: change it to handle re-using existing pixels to
    /// optimize the image construction. Also still only uses a list instead
    /// of quad-tree.
    for(const cv::Vec2i &pixelCoord : pixelsToUpdate) {
        const int &viewColI = pixelCoord[0];
        const int &viewRowI = pixelCoord[1];

        cv::Vec2d pixelPos = viewPosRef + viewCellSize.mul(cv::Vec2d((double)viewColI, (double)viewRowI));

        /*
        cv::Rect_<double> cellBounds(
            pixelPos, cv::Size_<double>(viewCellSize)
        );
        
        double readingTempSum = 0.0;
        int readingCount = 0;
        for(ReadingInfo &reading : readingList) {
            cv::Rect_<double> readingBounds(reading.pos, cv::Size_<double>(readingSize));

            if((cellBounds & readingBounds).area() >= 0.001) {

                readingTempSum += reading.temp;
                readingCount++;
            }
        }
        */

        cv::Vec2d pixelCentrePos = pixelPos+viewCellSize/2.0;

        std::vector<RectTree<ReadingInfo>::RectResult> resList;
        readingTree.findRectsAtPoint(pixelCentrePos[0], pixelCentrePos[1], resList);

        double readingTempSum = 0.0;
        int readingCount = resList.size();

        for(auto &rectRes : resList) {
            ReadingInfo &reading = rectRes.rect->value;

            readingTempSum += reading.temp;
        }

        double readingTempAverage = (readingCount == 0 ? 0 : readingTempSum/(double)readingCount);

        if(readingCount > 0) {
            /// Calls the function reference.
            setPixelColorFunc(
                viewImgRef,
                cv::Vec2i(viewColI, viewRowI),
                readingTempAverage
            );
        }
    }
}

void SensorSamplerRealm::clearAllReadings()
{
    readingList.clear();
    newReadingList.clear();
    readingTree.reset(-150.0, -90.0, 300.0, 180.0, 5);
}


SensorViewWindow::SensorViewWindow(
    cv::Mat _img, cv::Vec2d _pos, cv::Vec2d _size
) : img(_img)
  , newPos(_pos), currPos(_pos)
  , newSize(_size), currSize(_size)
  , forcedInvalid(false)
{
    // 
}

const cv::Mat &SensorViewWindow::getImage() const
{
    return img;
}

void SensorViewWindow::forceInvalidation()
{
    forcedInvalid = true;
}
