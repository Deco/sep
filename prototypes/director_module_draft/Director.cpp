#include <iostream> 
#include <assert.h>
#include <sstream>
#include <limits>
#include <opencv2/opencv.hpp>
#include "Director.h"

/**
 * Created by: Aaron Nguyen
 * Constructor
 */
Director::Director() 
{
	ac->init();
}

/**
 * Created by: Aaron Nguyen
 * Constantly update whether the actuator is idle or not.
 */
void Director::update()
{
	isIdle = !ac->getIsMoving();
}

/**
 * Created by: Aaron Nguyen
 * Stop when actuator is moving, else, display output.
 */
void Director::haltMovement()
{
	if(isIdle) { //If actuator is already idle, then output message.
		std::cout << "Already Idle" << std::endl;
	}
	else {
		isIdle = true;
		//Calls stop funciton in ActuatorController to stop movement.
		ac->stop();		
	}
}

/**
 * Created by: Aaron Nguyen
 * Takes in a vector of Vec2d's which are the points the user selected.
 */
void Director::performRegionScan(std::vector<cv::Vec2d> pointsList)
{
    //Select the region 
	selectArea(pointsList);
	//If actuator is idle then...
	if(isIdle) {
		scanArea(selectedRegion);
	}
	else { //Else then stop moving then scan the area.
		haltMovement();
		scanArea(selectedRegion);
	}
}

/**
 * Created by: Aaron Nguyen
 * Performs a full scan using the actuator's maximum and minumum yaw and
 * pitch positions.
 */
std::vector<ActuatorMoveOrder> Director::performFullScan()
{
	std::vector<ActuatorMoveOrder> ordersList;
	double inf = std::numeric_limits<double>::infinity();
	std::vector<cv::Vec2d> selectedRegion = {{-150, -90}, {150, 90}};
	//The region is the maximum and minimum rotation limits of the actuator.
	if(isIdle) { //If actuator is not moving then scan
		ordersList = scanAreaPrecision(selectedRegion);
	}
	else { //If actuator is moving, then stop moving then scan
		haltMovement();
		ordersList = scanAreaPrecision(selectedRegion);
	}
	return ordersList;
}

/**
 * Created by: Aaron Nguyen
 * Manual panning imports a single order to move actuator to a single position
 */
void Director::performManualPan(ActuatorMoveOrder order)
{
	//std::cout << "Idle: " << isIdle << std::endl;
	if(isIdle) { //If 
		ac->queueMove(order);	
	}
	else {
		haltMovement();
		ac->queueMove(order);
	}
	
}

/**
 * Created by: Aaron Nguyen
 * Clears the data realm data used later on.
 */
void Director::clearCollectedData(/*Add time to clear later */)
{
	//Data realm stuff
}

/**
 * Created by: Aaron Nguyen
 * Selects a region given the points the user selected
 * by finding the max and min values for both yaw and pitch.
 */
void Director::selectArea(std::vector<cv::Vec2d> pointsList)
{
    //Initialize the max and min values as infinite
	double inf = std::numeric_limits<double>::infinity();
	cv::Vec2d min = {inf, inf};
	cv::Vec2d max = {-inf, -inf};
	//Go through each points and find the max and min, yaw and pitch values.
	for(int i = 0; i < pointsList.size(); i++) {
		if((pointsList[i])[0] < min[0]) {
			min[0] = (pointsList[i])[0];
		}
		if((pointsList[i])[0] > max[0]) {
			max[0] = (pointsList[i])[0];
		}
		if((pointsList[i])[1] < min[1]) {
			min[1] = (pointsList[i])[1];
		}
		if((pointsList[i])[1] > max[1]) {
			max[1] = (pointsList[i])[1];
		}
	}

	std::cout << "min: " << min[0] << "," << min[1] << std::endl;
	std::cout << "max: " << max[0] << "," << max[1] << std::endl;
	//selectedRegion contains the 4 corners of the region
	selectedRegion = {min, max};
}


/**
 * Created by: Aaron Nguyen
 * Scans a region given the region points.
 * A prototype that does not factor in the regions that have already been scanned.
 * Will be changed later on.
 */
void Director::scanArea(std::vector<cv::Vec2d> area_points)
{
	std::vector<ActuatorMoveOrder> ordersList;
	double width = area_points[1][0] - area_points[0][0];
	double height = area_points[1][1] - area_points[0][1];
	double movepath_x = area_points[0][0];
	double movepath_y = area_points[0][1];
	bool change = true;
	double inf = std::numeric_limits<double>::infinity();
	if( width != -inf && height != -inf) {
		for(int i = 0; i < 90; i += 10) {
			ActuatorMoveOrder order;
			if(change) { //Changes the direction and angle of the the next scan position 
				//Start 10% from top
				movepath_y += height*0.10;
				//Add 10% from left side
				order.posDeg = cv::Vec2d(movepath_x + width*0.10 , movepath_y);
			    order.duration = 2;
			    //Add the order to the list
			    ordersList.push_back(order);
			    change = false;

			}
			else {
				movepath_y += height*0.10;
				order.posDeg = cv::Vec2d(movepath_x + width*0.90 , movepath_y);
			    order.duration = 2;
			    //Add the order to the list
			    ordersList.push_back(order);
			    change = true;
			}

		}
	}

    //Add the orders to the queue for the actuator to start scanning.
	std::cout << ordersList.size() << std::endl;
	if(ordersList.size() != 0)
	{
		addOrdersToQueue(ordersList);
	}
}

/**
 * Created by: Aaron Nguyen
 * A full scan prototype that does not factor in areas that have been already scanned.
 * Will be changed later on.
 */
std::vector<ActuatorMoveOrder> Director::scanAreaPrecision(std::vector<cv::Vec2d> area_points)
{
	std::vector<ActuatorMoveOrder> ordersList;
	double width = area_points[1][0] - area_points[0][0];
	double height = area_points[1][1] - area_points[0][1];
	double movepath_x = area_points[0][0];
	double movepath_y = area_points[0][1] + height*0.10;
	bool change = true;
	double inf = std::numeric_limits<double>::infinity();
	if( width != -inf && height != -inf) {
		//Top Scan to the right
		for(int i = 0; i < 180; i += 10) {
			ActuatorMoveOrder order;
			if(change) {
				movepath_x += width*0.05;
				order.posDeg = cv::Vec2d(movepath_x, movepath_y);
			    order.duration = 2;
			    //Add the order to the list
			    ordersList.push_back(order);
			    change = false;

			}
			else {
				movepath_x += width*0.05;
				order.posDeg = cv::Vec2d(movepath_x, movepath_y + height*0.20);
			    order.duration = 2;
			    //Add the order to the list
			    ordersList.push_back(order);
			    change = true;
			}
		}
		change = false;
		movepath_y += height*0.30;
		ActuatorMoveOrder order; 
		order.posDeg = cv::Vec2d(movepath_x, movepath_y);
		order.duration = 2;
		//Add the order to the list
		ordersList.push_back(order);
		std::cout<< change << std::endl;

		//Middle Scan to the left
		for(int i = 0; i < 170; i += 10) {
			ActuatorMoveOrder order;
			if(change) {
				movepath_x -= width*0.05;
				order.posDeg = cv::Vec2d(movepath_x, movepath_y);
			    order.duration = 2;
			    //Add the order to the list
			    ordersList.push_back(order);
			    change = false;

			}
			else {
				movepath_x -= width*0.05;
				order.posDeg = cv::Vec2d(movepath_x, movepath_y + height*0.20);
			    order.duration = 2;
			    //Add the order to the list
			    ordersList.push_back(order);
			    change = true;
			}
		}
		change = false;
		movepath_y += height*0.30;
		
		order.posDeg = cv::Vec2d(movepath_x, movepath_y);
		order.duration = 2;
		//Add the order to the list
		ordersList.push_back(order);
		std::cout<< change << std::endl;

		//Bottom Scan to the right
		for(int i = 0; i < 170; i += 10) {
			ActuatorMoveOrder order;
			if(change) {
				movepath_x += width*0.05;
				order.posDeg = cv::Vec2d(movepath_x, movepath_y);
			    order.duration = 2;
			    //Add the order to the list
			    ordersList.push_back(order);
			    change = false;

			}
			else {
				movepath_x += width*0.05;
				order.posDeg = cv::Vec2d(movepath_x, movepath_y + height*0.20);
			    order.duration = 2;
			    //Add the order to the list
			    ordersList.push_back(order);
			    change = true;
			}
		}
	}
	
	//Add the orders to the queue for the actuator to start scanning.
	std::cout << ordersList.size() << std::endl;
	if(ordersList.size() != 0)
	{
		addOrdersToQueue(ordersList);
	}
	
	//Returns the orderList to display the path of the scan. Used for testing purposes.
	return ordersList;
}

/**
 * Created by: Aaron Nguyen
 * Calls queueMoves in the ActuatorController class to do multiple scans.
 **/
void Director::addOrdersToQueue(std::vector<ActuatorMoveOrder> ordersList)
{
	ac->queueMoves(ordersList);
}

/**
 * Created by: Aaron Nguyen
 * Returns the current position of the actuator
 **/
cv::Vec2d Director::getCurrentPositionAC()
{
	return ac->getCurrentPosition();
}

/**
 * Created by: Aaron Nguyen
 * Returns the region the user selected
 **/
std::vector<cv::Vec2d> Director::getSelectedRegion()
{
	return selectedRegion;
}


















