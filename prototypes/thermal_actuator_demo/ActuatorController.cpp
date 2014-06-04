#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <algorithm>
#include <assert.h>
#include <sstream>

#include "Dynamixel/DynamixelComm.h"

class ActuatorController {
private:
	DynamixelComm dc;
public:
	ActuatorController(const std::string deviceName)
	{
		dc
	}

	void getCurrentPosition(cv::Vec2d &pos)
	{

	}
	void setGoalPosition(cv::Vec2d &pos, double time)
	{

	}
	void getPositionRange(cv::Vec2d &min, cv::Vec2d &max)
	{

	}

}
