#include <opencv2/opencv.hpp>


struct SensorViewWindow;

/// Interface for the Scanning Data Realm which recieves readings
/// from the Scanning Data Reader, and stores the information in
/// a list/quad-tree. Its responisble for processing the raw 
/// readings into higher, refactored readings through supersampling
/// and time-weighting.
class SensorSamplerRealm
{
public:
	//SensorSamplerRealm(ParamSet &_pset);
	SensorSamplerRealm(cv::Vec2d _minPos, cv::Vec2d _maxPos);
    
	void addReadingWindow(
	    cv::Mat_<float> &matRef,
	    cv::Vec2d pos, cv::Vec2d readingSize,
	    double time
    );
    void clearAllReadings();
    
	void updateViewWindow(
	    SensorViewWindow &viewWindowRef,
	    std::function<void (cv::Mat &pixelRef, const float &temp)> setPixelColorFunc
    );

private:
    struct ReadingInfo
    {
        cv::Vec2d pos;
        double temp;
        double time;
        
        ReadingInfo(cv::Vec2d _pos, double _temp, double _time) : pos(_pos), temp(_temp), time(_time) { /* */ };
    };

private:
    /// List of readings. Will be changed to a quad-tree in later
    /// releases.
	std::vector<ReadingInfo> readingList;
	cv::Vec2d minPos; /// In Degrees
	cv::Vec2d maxPos;
	cv::Vec2d readingSize; /// Size of individual reader.
};

struct SensorViewWindow
{
public:
    SensorViewWindow(cv::Mat _img, cv::Vec2d _pos, cv::Vec2d _size);
    
    /// Setters and getters of the view window.
    void changePos(cv::Vec2d v);
    cv::Vec2d getPos() const;
    cv::Vec2d getCurrentPos() const;
    
    void changeSize(cv::Vec2d v);
    cv::Vec2d getSize() const;
    cv::Vec2d getCurrentSize() const;
    
    const cv::Mat &getImage() const;
    
    double getTime() const;
    
protected:
    friend class SensorSamplerRealm;
    
    cv::Mat img;
    cv::Vec2d newPos, currPos, newSize, currSize;
    double 
    time;
};