#include <iostream> 
#include <assert.h>
#include <sstream>
#include <limits>
#include <opencv2/opencv.hpp>
#include "Director.h"

Director::Director() 
{
	ac->init();
}

void Director::update()
{
	//ac->update();
	if(!ac->getIsMoving()) {
		isIdle = true;
	}
	else {
		isIdle = false;
	}
}

void Director::haltMovement()
{
	if(isIdle) {
		std::cout << "Already Idle" << std::endl;
	}
	else {
		isIdle = true;
		ac->stop();		
	}
}

void Director::performRegionScan(std::vector<cv::Vec2d> pointsList)
{
	selectArea(pointsList);
	if(isIdle) {
		scanArea(selectedRegion);
		//isIdle = false;
	}
	else {
		haltMovement();
		scanArea(selectedRegion);
		//isIdle = false;
	}
}

std::vector<ActuatorMoveOrder> Director::performFullScan()
{
	std::vector<ActuatorMoveOrder> ordersList;
	double inf = std::numeric_limits<double>::infinity();
	std::vector<cv::Vec2d> selectedRegion = {{-150, -90}, {150, 90}};
	if(isIdle) {
		ordersList = scanAreaPrecision(selectedRegion);
		//isIdle = false;
	}
	else {
		haltMovement();
		ordersList = scanAreaPrecision(selectedRegion);
		//isIdle = false;
	}
	return ordersList;
}

void Director::performManualPan(ActuatorMoveOrder order)
{
	std::cout << "Idle: " << isIdle << std::endl;
	if(isIdle) {
		ac->queueMove(order);	
		//isIdle = false;
	}
	else {
		haltMovement();
		ac->queueMove(order);
		//isIdle = false;
	}
	
}

void Director::clearCollectedData(/*Add time to clear later */)
{
	//Data realm stuff
}

void Director::selectArea(std::vector<cv::Vec2d> pointsList)
{
	double inf = std::numeric_limits<double>::infinity();
	cv::Vec2d min = {inf, inf};
	cv::Vec2d max = {-inf, -inf};
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
	selectedRegion = {min, max};
}



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
			if(change) {
				//Start 10% from top
				movepath_y += height*0.10;
				//Add 10% from left side
				order.posDeg = cv::Vec2d(movepath_x + width*0.10 , movepath_y);
			    order.duration = 2;
			    ordersList.push_back(order);
			    change = false;

			}
			else {
				movepath_y += height*0.10;
				order.posDeg = cv::Vec2d(movepath_x + width*0.90 , movepath_y);
			    order.duration = 2;
			    ordersList.push_back(order);
			    change = true;
			}

		}
	}

	std::cout << ordersList.size() << std::endl;
	if(ordersList.size() != 0)
	{
		addOrdersToQueue(ordersList);
	}
}

//This is just a prototype for full scan.
//Can be removed.
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
			    ordersList.push_back(order);
			    change = false;

			}
			else {
				movepath_x += width*0.05;
				order.posDeg = cv::Vec2d(movepath_x, movepath_y + height*0.20);
			    order.duration = 2;
			    ordersList.push_back(order);
			    change = true;
			}
		}
		change = false;
		movepath_y += height*0.30;
		ActuatorMoveOrder order; 
		order.posDeg = cv::Vec2d(movepath_x, movepath_y);
		order.duration = 2;
		ordersList.push_back(order);
		std::cout<< change << std::endl;

		//Middle Scan to the left
		for(int i = 0; i < 170; i += 10) {
			ActuatorMoveOrder order;
			if(change) {
				movepath_x -= width*0.05;
				
				order.posDeg = cv::Vec2d(movepath_x, movepath_y);

			    order.duration = 2;
			    ordersList.push_back(order);
			    change = false;

			}
			else {
				movepath_x -= width*0.05;
				order.posDeg = cv::Vec2d(movepath_x, movepath_y + height*0.20);
			    order.duration = 2;
			    ordersList.push_back(order);
			    change = true;
			}
		}
		change = false;
		movepath_y += height*0.30;
		
		order.posDeg = cv::Vec2d(movepath_x, movepath_y);
		order.duration = 2;
		ordersList.push_back(order);
		std::cout<< change << std::endl;

		//Bottom Scan to the right
		for(int i = 0; i < 170; i += 10) {
			ActuatorMoveOrder order;
			if(change) {
				movepath_x += width*0.05;
				order.posDeg = cv::Vec2d(movepath_x, movepath_y);
			    order.duration = 2;
			    ordersList.push_back(order);
			    change = false;

			}
			else {
				movepath_x += width*0.05;
				order.posDeg = cv::Vec2d(movepath_x, movepath_y + height*0.20);
			    order.duration = 2;
			    ordersList.push_back(order);
			    change = true;
			}
		}
	}
	std::cout << ordersList.size() << std::endl;
	if(ordersList.size() != 0)
	{
		addOrdersToQueue(ordersList);
	}
	return ordersList;
}

void Director::addOrdersToQueue(std::vector<ActuatorMoveOrder> ordersList)
{
	ac->queueMoves(ordersList);
}

cv::Vec2d Director::getCurrentPositionAC()
{
	return ac->getCurrentPosition();
}

std::vector<cv::Vec2d> Director::getSelectedRegion()
{
	return selectedRegion;
}


















