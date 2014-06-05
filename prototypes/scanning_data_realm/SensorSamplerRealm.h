#include <opencv2/opencv.hpp>

struct SensorSampleWindow 
{
	cv::Mat_<double> img;
	cv::Vec2d newPos, oldPos, size;
	double time;
};

struct ReadingInfo
{
    cv::Vec2d position;
    double tempValue;
    long creationTime;
};

class SensorSamplerRealm
{
public:
	std::vector<ReadingInfo> list;
	double maxPan;
	double maxTilt;
	double readingSize;
	//SensorSamplerRealm(ParamSet &_pset);
	SensorSamplerRealm(double inMaxTilt, double inMaxPan, double inReadingSize);

	void addReadings(cv::Mat_<float> &matRef, cv::Vec2d pos, double time);

	void updateWindow(SensorSampleWindow &windowRef);
};

