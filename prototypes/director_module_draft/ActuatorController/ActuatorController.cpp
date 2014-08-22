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

/**
* Created by: Aaron Nguyen
* Template used to return the absolute value of a vector
*/
template<typename T>
cv::Vec<T, 2> vecabs(cv::Vec<T, 2> v) {
	return cv::Vec<T, 2>(std::abs(v[0]), std::abs(v[1]));
}

/** Constant values for the servos in the format of: (yaw, pitch) */
const cv::Vec2d ActuatorController::minDeg = cv::Vec2d(-150.0, -90.0);
const cv::Vec2d ActuatorController::maxDeg = cv::Vec2d( 150.0,  90.0);
const cv::Vec2i ActuatorController::minServoCoord = cv::Vec2i(   0, 203);
const cv::Vec2i ActuatorController::maxServoCoord = cv::Vec2i(1023, 819);

/** 
* Created by: Aaron Nguyen
* An alternate constructor importing device name.
*/
ActuatorController::ActuatorController(const std::string deviceName)
	: dc(deviceName.c_str(), 1000000)
{
	//Initialises shouldStop atomic boolean to false.
	shouldStopAtom.store(false);
	isAcMoving.store(false);
}

/**
* Created by: Aaron Nguyen 
* Initialises Actuator Thread 
*/
void ActuatorController::init()
{
	updateThread = std::thread(&ActuatorController::updateThreadFunc, this);
}

void ActuatorController::shutdown() 
{
	//
}

void ActuatorController::update() {
	// 
}


bool ActuatorController::getIsMoving() 
{
	if(1) {
		std::lock_guard<std::mutex> movingLock(movingMutex);
		return isAcMoving.load();
	}
}

/**
* Created by: Aaron Nguyen 
* Returns the servos yaw and pitch current position in degrees. 
*/
cv::Vec2d ActuatorController::getCurrentPosition()
{
	if(1) { // Ensures we have exclusive access to the current position of the servos
		std::lock_guard<std::mutex> currentPosDegLock(currentPosDegMutex);
		return currentPosDeg;
	}
}

/** 
* Created by: Aaron Nguyen
* Set the minimum and maximum degree values of yaw and pitch servos
*/
void ActuatorController::getPositionRange(cv::Vec2d &min, cv::Vec2d &max)
{
	min[0] = -150; //Min yaw
	min[1] = -90; //Min pitch

	max[0] = 150; //Max yaw
	max[1] = 90; //Max pitch
}

/**
* Created by: Aaron Nguyen 
* Receives an order to be pushed into the moveQueue 
*/
void ActuatorController::queueMove(ActuatorMoveOrder order)
{
	moveQueue.push(order);
}

/**
* Created by: Aaron Nguyen 
* Receives a list of move orders to be pushed into the moveQueue.
*/
void ActuatorController::queueMoves(std::vector<ActuatorMoveOrder> moveList) 
{
	// Pushes each order in moveList to moveQueue
	for(ActuatorMoveOrder order : moveList) {
		moveQueue.push(order);
	}
}

/**
* Created by: Aaron Nguyen 
* This method clears all queued orders and cancels the current order.
*/
void ActuatorController::stop() 
{
	if(1) { // Ensure we have exclusive access to moveQueue
		std::lock_guard<std::mutex> moveQueueLock(moveQueueMutex);
		// Swap the queue with an empty one, hence clearing all queued orders
		std::queue<ActuatorMoveOrder>().swap(moveQueue);
		// Set the flag that indicates we should stop all servo movement
		shouldStopAtom.store(true);
	}
}

/**** PRIVATE INTERNAL STUFF BELOW THIS LINE ****/

/** 
* Created by: Aaron Nguyen
* Converts yaw and pitch degrees into servo coordinates, based on the min and max
* angle and coord fields.
*/
cv::Vec2i ActuatorController::degreeToServoCoords(cv::Vec2d posDeg)
{
	// Calculate the difference and convert to a scalar
	double fracYaw   = (posDeg[0]-minDeg[0])/(maxDeg[0]-minDeg[0]);
	double fracPitch = (posDeg[1]-minDeg[1])/(maxDeg[1]-minDeg[1]);

	// Use this scalar to figure out the coords
	return cv::Vec2i(
		minServoCoord[0]+fracYaw  *(double)(maxServoCoord[0]-minServoCoord[0]),
		minServoCoord[1]+fracPitch*(double)(maxServoCoord[1]-minServoCoord[1])
	);
}

/**
* Created by: Aaron Nguyen 
* Converts yaw and pitch servo coordinates into degrees, based on the min and max
* angle and coord fields
*/
cv::Vec2d ActuatorController::servoCoordsToDegree(cv::Vec2i posCoord)
{
	// Calculate the difference and convert to a scalar
	double fracYaw   = ((double)posCoord[0]-(double)minServoCoord[0]);
	fracYaw /= ((double)maxServoCoord[0]-(double)minServoCoord[0]);

	double fracPitch = ((double)posCoord[1]-(double)minServoCoord[1]);
	fracPitch /= ((double)maxServoCoord[1]-(double)minServoCoord[1]);

	// Use this scalar to figure out the degrees
	return cv::Vec2d(
		(double)minDeg[0]+fracYaw  * (double)(maxDeg[0]-minDeg[0]),
		(double)minDeg[1]+fracPitch* (double)(maxDeg[1]-minDeg[1])
	);
}


/** 
* Created by: Aaron Nguyen
* Calculates the yaw and pitch speed required to get to the goal position
* in the time given. It uses the servo speed calculated to send move commands
* to the actuator to be moved.
*/
void ActuatorController::commMove(cv::Vec2d goalDegPos, double timeSeconds)
{
	// Throws exception when:
	// Yaw goal position is below the min yaw constraints
	if(goalDegPos[0] < minDeg[0]) { 
		std::cout << goalDegPos[0] << std::endl;
		throw std::runtime_error("Goal yaw is too low!");
	}
	// Yaw goal position is above the max yaw constraints
	else if(goalDegPos[0] > maxDeg[0]) {
		throw std::runtime_error("Goal yaw is too high!");
	}
	//Pitch goal position is below the min yaw constraints
	else if(goalDegPos[1] < minDeg[1]) { 
		throw std::runtime_error("Goal pitch is too low!");
	}
	// Pitch goal position is above the max yaw constraints
	else if(goalDegPos[1] > maxDeg[1]) { 
		throw std::runtime_error("Goal pitch is too high!");
	}


	double maxSpeedRpm = 114.0;
	double maxSpeedCoord = 1023.0;

	// Obtain yaw and pitch current servo positions in degrees
	cv::Vec2d currentDegServoPos = servoCoordsToDegree(commObtainCurrentCoords());
	// Calculate the yaw and pitch distances from current to goal positions in degrees
	cv::Vec2d goalDegDiff = currentDegServoPos-goalDegPos;
	// Calculate velocity of the servos, velocity = distance/time
	// vecabs used to find absolute value of the difference
	cv::Vec2d velDeg = vecabs(goalDegDiff)/timeSeconds;
	// Calculate revolution per minute for yaw and pitch
	cv::Vec2d rpmDeg = (velDeg/360.0)*60.0;
	// Calculate servo speed for both yaw and pitch sservos
	cv::Vec2d servoSpeed = (rpmDeg/maxSpeedRpm) * maxSpeedCoord;

	// Moves the yaw and pitch servos given goal position and the servo speeds
	dc.Move(01, degreeToServoCoords(goalDegPos)[0], servoSpeed[0]);
	dc.Move(16, degreeToServoCoords(goalDegPos)[1], servoSpeed[1]);
	
}

/** 
* Created by: Aaron Nguyen
* Obtains the servos current position in servo coordinates.(yaw, pitch)
*/
cv::Vec2i ActuatorController::commObtainCurrentCoords()
{
	return cv::Vec2i(dc.GetPosition(01), dc.GetPosition(16));
}

/** 
* Created by: Aaron Nguyen
* Receives information from servos to check whether they are
* moving or not.
*/
bool ActuatorController::commObtainIsMoving() 
{
	bool isMoving = false;
	bool isServoXMoving = dc.GetIsMoving(01);
	bool isServoYMoving = dc.GetIsMoving(16);
	// Checks whether either servos are currently moving...
	if(isServoYMoving || isServoXMoving) {
		isMoving = true;
	}
	return isMoving;
}

/**
* Created by: Aaron Nguyen
* A thread function that continuously runs, and pops any order within the queue
* to be executed. Calls necesssary functions to stop or move the actuator servos
*/
void ActuatorController::updateThreadFunc()
{

	//dc.SetTorqueEnabled(01, false);
	//dc.SetTorqueEnabled(16, false);

	while(true) {
		//bool isMoving = commObtainIsMoving();
		if(1) {
			std::lock_guard<std::mutex> movingLock(movingMutex);
			isAcMoving.store(commObtainIsMoving());
			
		}
		bool isMoving = isAcMoving.load();

		cv::Vec2i currentCoords = commObtainCurrentCoords();
        cv::Vec2d posDeg = servoCoordsToDegree(currentCoords);

        if(1) {
            std::lock_guard<std::mutex> currentPosDegLock(currentPosDegMutex);
		    currentPosDeg = posDeg;
        }

        bool shouldStop = shouldStopAtom.load();
        bool shouldSendNextOrder = true;
        // If the servo is moving, and we aren't told to stop moving...
		if(isMoving && !shouldStop) {
			// don't send a new order, let the current one complete.
			shouldSendNextOrder = false;
		}
		if(shouldSendNextOrder) {
			// Otherwise, the servos are ready for new orders...
            bool hasOrder = false;
            ActuatorMoveOrder order;
			if(1) { // so ensure we have exclusive acccess to the queue...
				std::lock_guard<std::mutex> moveQueueLock(moveQueueMutex);
				// and if there's an order in the queue...
                if(!moveQueue.empty()) {
                	// pop it and store it...
                    order = moveQueue.front();
                    moveQueue.pop();
                    hasOrder = true;
                }
			}
			if(hasOrder) {
        		std::cout << "ORDER ISSUED!" << std::endl;
        		// and forward it to the servos.
                commMove(order.posDeg, order.duration);
			} else if(shouldStop) {
				// If there was no queued order, then the servos should stop moving.
				// To do this, we issue an order with no change in position.
				dc.SetPosition(01, currentCoords[0]);
				dc.SetPosition(16, currentCoords[1]);
			}
		}
		if(shouldStop) { // If servos currently should be stopped...
			// Then set shouldStopAtom flag to false, allowing the servos to 
			// continue moving on next command.
			shouldStopAtom.store(false);
		}
	}
}





