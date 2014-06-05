#include <opencv2/opencv.hpp>
#include "Dynamixel/DynamixelComm.h"
#include <queue>
#include <thread>
#include <atomic>

struct ActuatorMoveOrder {
	cv::Vec2d posDeg;
	double duration;
};

class ActuatorController {
private:

	std::thread updateThread;
	DynamixelComm dc;

	std::mutex currentPosDegMutex;
	cv::Vec2d currentPosDeg;
	
	std::mutex orderQueueMutex;
	std::queue<ActuatorMoveOrder> orderQueue;

	cv::Vec2d servoPosDeg;
	std::queue<ActuatorMoveOrder> moveQueue;
	
public:
	ActuatorController(const std::string deviceName);

	void init();
	void shutdown();

	void update();
	void getCurrentPosition(cv::Vec2d &posRef);
	cv::Vec2i getCurrentCoords();
	void move(cv::Vec2d goalPos, double timeSeconds);
	void queueMoves(std::vector<ActuatorMoveOrder> moveList);
	void stop();
	void getPositionRange(cv::Vec2d &min, cv::Vec2d &max);

	bool getIsMoving();


private:
	cv::Vec2i degreeToServoCoords(cv::Vec2d posDeg);
	cv::Vec2d servoCoordsToDegree(cv::Vec2i posCoord);

private:
	static const cv::Vec2d minDeg;
	static const cv::Vec2d maxDeg;
	static const cv::Vec2i minServoCoord;
	static const cv::Vec2i maxServoCoord;
};