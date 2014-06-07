#include <opencv2/opencv.hpp>
#include "Dynamixel/DynamixelComm.h"
#include <queue>
#include <thread>
#include <mutex>

struct ActuatorMoveOrder {
	cv::Vec2d posDeg;
	double duration;
	bool isStopOrder;
};

class ActuatorController {
private:

	std::thread updateThread;
	DynamixelComm dc;
	cv::Vec2d servoPosDeg;

	std::mutex currentPosDegMutex;
	cv::Vec2d currentPosDeg;

	std::atomic_bool shouldStopAtom;
	
	std::mutex moveQueueMutex;
	std::queue<ActuatorMoveOrder> moveQueue;
	
public:
	ActuatorController(const std::string deviceName);

	void init();
	void shutdown();
	void update();

    cv::Vec2d getCurrentPosition();
	void getPositionRange(cv::Vec2d &min, cv::Vec2d &max);
    
    void queueMove(ActuatorMoveOrder order);
    void queueMoves(std::vector<ActuatorMoveOrder> orderList);
    void stop();
   

private:
	cv::Vec2i degreeToServoCoords(cv::Vec2d posDeg);
	cv::Vec2d servoCoordsToDegree(cv::Vec2i posCoord);
	
	void commMove(cv::Vec2d goalPos, double timeSeconds);
	cv::Vec2i commObtainCurrentCoords();
	bool commObtainIsMoving();
	
	void updateThreadFunc();
private:
	static const cv::Vec2d minDeg;
	static const cv::Vec2d maxDeg;
	static const cv::Vec2i minServoCoord;
	static const cv::Vec2i maxServoCoord;
};