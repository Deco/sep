#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <algorithm>
#include <assert.h>
#include <sstream>
#include <opencv2/opencv.hpp>
#include "Dynamixel/DynamixelComm.h"
#include "ActuatorController.h"

ActuatorController::ActuatorController(const std::string deviceName)
	: dc(deviceName.c_str(), 1000000)
{

}

int degreeToServoCoords(
	double deg, double minDeg, double maxDeg,
	int minServo, int maxServo)
{
	double frac = (deg-minDeg)/(maxDeg-minDeg);
	double servoCoord = ((double)minServo)+frac*((double)(maxServo-minServo));

	return (int)servoCoord;
}

int servoToDegrees(
	double servo, double minDeg, double maxDeg,
	int minServo, int maxServo)
{
	double frac = (servo - minServo) / (maxServo - minServo);
	double degrees = ((double)minDeg) + frac*((double)(maxDeg-minDeg));
	return (int)degrees;
}

void ActuatorController::update() { }

void ActuatorController::getCurrentPosition(cv::Vec2d &posRef)
{
	servoXPos = dc.GetPosition(01);
    servoYPos = dc.GetPosition(16);
    posRef[0] = servoXPos;
    posRef[1] = servoYPos;
}
void ActuatorController::move(cv::Vec2d &goalDegPos, double timeSeconds)
{
	if(goalDegPos[0] < minYawDeg) {
		throw std::runtime_error("Goal yaw is too low!");
	}
	else if(goalDegPos[0] > maxYawDeg) {
		throw std::runtime_error("Goal yaw is too high!");
	}
	else if(goalDegPos[1] < minPitchDeg) {
		throw std::runtime_error("Goal pitch is too low!");
	}
	else if(goalDegPos[1] > maxPitchDeg) {
		throw std::runtime_error("Goal pitch is too high!");
	}

	double maxSpeed = 114.0;
	double maxSpeedCode = 1023.0;

	//Servo 01, X
	double servoX_GoalPos = degreeToServoCoords(goalDegPos[0], 0, 300, 0, 1023);
	double degX_currentPos = servoToDegrees(servoXPos, 0, 300, 0, 1023);
	double degX_PosDiff = std::abs(goalDegPos[0] - degX_currentPos);
	double rpmX = (degX_PosDiff/360.0)/(timeSeconds/60.0); 
	double servoSpeedX = (rpmX/maxSpeed) * maxSpeedCode;


	//Servo 16, Y
	double servoY_GoalPos = degreeToServoCoords(goalDegPos[1], -90, 90, 203, 819);
	double degY_currentPos = servoToDegrees(servoYPos, -90, 90, 203, 819);
	double degY_PosDiff = std::abs(goalDegPos[1] - degY_currentPos);
	double rpmY = (degY_PosDiff/360.0)/(timeSeconds/60.0);
	double servoSpeedY = (rpmY/maxSpeed) * maxSpeedCode;


	dc.Move(01, servoX_GoalPos, (int)servoSpeedX);
	dc.Move(16, servoY_GoalPos, (int)servoSpeedY);
	
}
void ActuatorController::stop() 
{
	servoXPos = dc.GetPosition(01);
	servoYPos = dc.GetPosition(16);
	dc.SetPosition(01, servoXPos);
	dc.SetPosition(16, servoYPos);
}
void ActuatorController::getPositionRange(cv::Vec2d &min, cv::Vec2d &max)
{
	min[0] = 0; //Min x
	min[1] = -90; //Min y

	max[0] = 300; //Max x
	max[1] = 90; //Max y
}
