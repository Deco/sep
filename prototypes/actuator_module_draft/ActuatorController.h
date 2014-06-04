#include <opencv2/opencv.hpp>
#include "Dynamixel/DynamixelComm.h"

class ActuatorController {
private:
	DynamixelComm dc;
	int servoXPos, servoYPos;
public:
	ActuatorController(const std::string deviceName);
	void update();
	void getCurrentPosition(cv::Vec2d &posRef);
	void move(cv::Vec2d &goalPos, double timeSeconds);
	void stop();
	void getPositionRange(cv::Vec2d &min, cv::Vec2d &max);

private:
	static const double minYawDeg = 0.0;
	static const double maxYawDeg = 300.0;
	static const double minPitchDeg = -90.0;
	static const double maxPitchDeg = 90.0;
};