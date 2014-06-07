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

template<typename T>
cv::Vec<T, 2> vecabs(cv::Vec<T, 2> v) {
	return cv::Vec<T, 2>(std::abs(v[0]), std::abs(v[1]));
}

const cv::Vec2d ActuatorController::minDeg = cv::Vec2d(-150.0, -90.0);
const cv::Vec2d ActuatorController::maxDeg = cv::Vec2d( 150.0,  90.0);
const cv::Vec2i ActuatorController::minServoCoord = cv::Vec2i(   0, 203);
const cv::Vec2i ActuatorController::maxServoCoord = cv::Vec2i(1023, 819);

ActuatorController::ActuatorController(const std::string deviceName)
	: dc(deviceName.c_str(), 1000000)
{
	shouldStopAtom.store(false);
}

void ActuatorController::init()
{
	updateThread = std::thread(&ActuatorController::updateThreadFunc, this);
}

void ActuatorController::update() {
	// 
}

cv::Vec2i ActuatorController::degreeToServoCoords(cv::Vec2d posDeg)
{
	double fracYaw   = (posDeg[0]-minDeg[0])/(maxDeg[0]-minDeg[0]);
	double fracPitch = (posDeg[1]-minDeg[1])/(maxDeg[1]-minDeg[1]);

	return cv::Vec2i(
		minServoCoord[0]+fracYaw  *(maxServoCoord[0]-minServoCoord[0]),
		minServoCoord[1]+fracPitch*(maxServoCoord[1]-minServoCoord[1])
	);
}

cv::Vec2d ActuatorController::servoCoordsToDegree(cv::Vec2i posCoord)
{
	double fracYaw   = ((double)posCoord[0]-(double)minServoCoord[0]);
	fracYaw /= ((double)maxServoCoord[0]-(double)minServoCoord[0]);

	double fracPitch = ((double)posCoord[1]-(double)minServoCoord[1]);
	fracPitch /= ((double)maxServoCoord[1]-(double)minServoCoord[1]);

	return cv::Vec2d(
		(double)minDeg[0]+fracYaw  * (double)(maxDeg[0]-minDeg[0]),
		(double)minDeg[1]+fracPitch* (double)(maxDeg[1]-minDeg[1])
	);
}

void ActuatorController::stop() 
{
	if(1) {
		std::lock_guard<std::mutex> moveQueueLock(moveQueueMutex);
		std::queue<ActuatorMoveOrder>().swap(moveQueue);
		shouldStopAtom.store(true);
	}
}
void ActuatorController::getPositionRange(cv::Vec2d &min, cv::Vec2d &max)
{
	min[0] = -150; //Min x
	min[1] = -90; //Min y

	max[0] = 150; //Max x
	max[1] = 90; //Max y
}

void ActuatorController::queueMove(ActuatorMoveOrder order)
{
	moveQueue.push(order);
}

void ActuatorController::queueMoves(std::vector<ActuatorMoveOrder> moveList) 
{
	for(ActuatorMoveOrder order : moveList) {
		moveQueue.push(order);
	}
}

cv::Vec2d ActuatorController::getCurrentPosition()
{
	if(1) {
		std::lock_guard<std::mutex> currentPosDegLock(currentPosDegMutex);
		return currentPosDeg;
	}
}

// PRIVATE INTERNAL STUFF BELOW THIS LINE

void ActuatorController::updateThreadFunc()
{
	dc.SetTorqueEnabled(01, false);
	dc.SetTorqueEnabled(16, false);

	while(true) {
		bool isMoving = commObtainIsMoving();

		cv::Vec2i currentCoords = commObtainCurrentCoords();
        cv::Vec2d posDeg = servoCoordsToDegree(currentCoords);

        if(1) {
            std::lock_guard<std::mutex> currentPosDegLock(currentPosDegMutex);
		    currentPosDeg = posDeg;
        }

        bool shouldStop = shouldStopAtom.load();
        bool shouldSendNextOrder = true;
		if(isMoving && !shouldStop) {
			shouldSendNextOrder = false;
		}
		if(shouldSendNextOrder) {
            bool hasOrder = false;
            ActuatorMoveOrder order;
			if(1) {
				std::lock_guard<std::mutex> moveQueueLock(moveQueueMutex);
                if(!moveQueue.empty()) {
                    order = moveQueue.front();
                    moveQueue.pop();
                    hasOrder = true;
                }
			}
			if(hasOrder) {
        		std::cout << "ORDER ISSUED!" << std::endl;
                commMove(order.posDeg, order.duration);
			} else if(shouldStop) {
				dc.SetPosition(01, currentCoords[0]);
				dc.SetPosition(16, currentCoords[1]);
			}
		}
		if(shouldStop) {
			shouldStopAtom.store(false);
		}
	}
}

cv::Vec2i ActuatorController::commObtainCurrentCoords()
{
	return cv::Vec2i(dc.GetPosition(01), dc.GetPosition(16));
}

void ActuatorController::commMove(cv::Vec2d goalDegPos, double timeSeconds)
{
	if(goalDegPos[0] < minDeg[0]) {
		std::cout << goalDegPos[0] << std::endl;
		throw std::runtime_error("Goal yaw is too low!");
	}
	else if(goalDegPos[0] > maxDeg[0]) {
		throw std::runtime_error("Goal yaw is too high!");
	}
	else if(goalDegPos[1] < minDeg[1]) {
		throw std::runtime_error("Goal pitch is too low!");
	}
	else if(goalDegPos[1] > maxDeg[1]) {
		throw std::runtime_error("Goal pitch is too high!");
	}

	double maxSpeedRpm = 114.0;
	double maxSpeedCoord = 1023.0;

	cv::Vec2d currentDegServoPos = servoCoordsToDegree(commObtainCurrentCoords());

	cv::Vec2d goalDegDiff = currentDegServoPos-goalDegPos;

	cv::Vec2d velDeg = vecabs(goalDegDiff)/timeSeconds;

	cv::Vec2d rpmDeg = (velDeg/360.0)*60.0;

	cv::Vec2d servoSpeed = (rpmDeg/maxSpeedRpm) * maxSpeedCoord;

	dc.Move(01, degreeToServoCoords(goalDegPos)[0], servoSpeed[0]);
	dc.Move(16, degreeToServoCoords(goalDegPos)[1], servoSpeed[1]);
	
}

bool ActuatorController::commObtainIsMoving() 
{
	bool isMoving = false;
	bool isServoXMoving = dc.GetIsMoving(01);
	bool isServoYMoving = dc.GetIsMoving(16);
	if(isServoYMoving || isServoXMoving) {
		isMoving = true;
	}
	return isMoving;
}