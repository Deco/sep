#include <opencv2/opencv.hpp>

struct SensorViewWindow
{
public:
    SensorViewWindow(cv::Mat _img, cv::Vec2d _pos, cv::Vec2d _size);
    
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
	double time;
};

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
	std::vector<ReadingInfo> readingList;
	cv::Vec2d minPos;
	cv::Vec2d maxPos;
	cv::Vec2d readingSize;
};
