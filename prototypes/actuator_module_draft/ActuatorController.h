#include <opencv2/opencv.hpp>
#include "Dynamixel/DynamixelComm.h"
#include <queue>
#include <thread>
#include <mutex>

/** 
* A struct containing a vector of goal yaw and pitch position and
* a duration time
*/
struct ActuatorMoveOrder {
	cv::Vec2d posDeg;
	double duration;
};

class ActuatorController {
private:

	// Actuator Thread
	std::thread updateThread;

	// Gives access to DynamixelComm methods
	DynamixelComm dc;

	// Mutex to gve exclusive access to the currentPosDeg
	std::mutex currentPosDegMutex;

	// A vector containing current position of yaw and pitch servos
	cv::Vec2d currentPosDeg;

	// Atomic boolean used as a flag for the stop functionality
	std::atomic_bool shouldStopAtom;
	
	// Mutex to give exclusive access to the moveQueue
	std::mutex moveQueueMutex;

	// Queue containing ActuatorMoveOrders
	std::queue<ActuatorMoveOrder> moveQueue;
	
public:
	// Constructor for the Actuator Controller
	// Imports the actuator's device name as a string
	ActuatorController(const std::string deviceName);

	// Initialises the actuator thread
	void init();
	// Currently does nothing
	void shutdown();
	// Current does nothing
	void update();

	// Imports void.
	// Exports the current yaw and pitch position
	// in servo coordinates within a vector.
    cv::Vec2d getCurrentPosition();

    // Imports a reference min and max vectors to set the
    // maximum and minimum degrees limitations of the yaw and pitch
    // servos.
	void getPositionRange(cv::Vec2d &min, cv::Vec2d &max);
    
    // Imports an ActuatorMoveOrder to be pushed into the queue
    void queueMove(ActuatorMoveOrder order);
    // Imports a list of ActuatorMoveOrders to be pushed into the queue
    void queueMoves(std::vector<ActuatorMoveOrder> orderList);
    // Clears the moveQueue and changes shouldStop atomic boolean to true
    void stop();
   

private:
	// Imports yaw and pitch servos position in degrees
	// to be converted into servo coordinates
	// Exports the positions as servo coordinates, integer
	cv::Vec2i degreeToServoCoords(cv::Vec2d posDeg);

	// Imports vector of servos positions in servo coordinates
	// to be converted into degrees
	// Exports the positions as degrees, double
	cv::Vec2d servoCoordsToDegree(cv::Vec2i posCoord);
	
	// Imports yaw and pitch goal positions in degrees and the 
	// arrival time it takes to get to the goal position
	// Calculates servo speed to move the actuator
	void commMove(cv::Vec2d goalPos, double timeSeconds);

	// 
	cv::Vec2i commObtainCurrentCoords();

	// Checks whether actuator servos are currently moving
	// Returns true or false
	bool commObtainIsMoving();
	
	// Calls necessary functions when actuator servos needs
	// to be stopped or moved.
	void updateThreadFunc();

private:
	// Constant values of the maximum and minimum rotation 
	// in degrees and servo coordinates 
	static const cv::Vec2d minDeg;
	static const cv::Vec2d maxDeg;
	static const cv::Vec2i minServoCoord;
	static const cv::Vec2i maxServoCoord;
};