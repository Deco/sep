#include <opencv2/opencv.hpp>
#include "Dynamixel/DynamixelComm.h"
#include <queue>

struct ActuatorMoveOrder {
	cv::Vec2d pos;
	double duration;
};

class ActuatorController {
private:
	DynamixelComm dc;
	int servoXPos, servoYPos;
	int servoX_GoalPos, servoY_GoalPos;
	int degX_GoalPos, degY_GoalPos;
	double desiredXSpeed, desiredYSpeed;
	double totalTime;

	std::queue<ActuatorMoveOrder> moveQueue;

public:
	ActuatorController(const std::string deviceName);
	void update();
	void getCurrentPosition(cv::Vec2d &posRef);
	void move(cv::Vec2d goalPos, double timeSeconds);
	void queueMoves(std::vector<ActuatorMoveOrder> moveList);
	void stop();
	void getPositionRange(cv::Vec2d &min, cv::Vec2d &max);

	bool getIsMoving();
	//bool getIsAtGoal();



private:
	static const double minYawDeg = 0.0;
	static const double maxYawDeg = 300.0;
	static const double minPitchDeg = -90.0;
	static const double maxPitchDeg = 90.0;
};