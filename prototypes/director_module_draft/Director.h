#include <opencv2/opencv.hpp>
#include "ActuatorController/ActuatorController.h"

class Director {
private:
    //Instiate ActuatorController
	ActuatorController *ac = new ActuatorController("/dev/tty.usbserial-A9S3VTXD");
	//Boolean for when actuator is idle.
	bool isIdle = true;

public:
    //Max and min values of the selected area
	std::vector<cv::Vec2d> selectedRegion;

public:
	Director();
	//Constantly update the actuator's state
	void update();
	//Stops the actuator's movement
	void haltMovement();
	//Scans a selected region
	void performRegionScan(std::vector<cv::Vec2d> pointsList);
	//Scans the full region
	std::vector<ActuatorMoveOrder> performFullScan();
	//Pans the actuator to the selected point
	void performManualPan(ActuatorMoveOrder order);
	//Clears all data realm data
	void clearCollectedData();
	//Returns the current position of the acuator
	cv::Vec2d getCurrentPositionAC();
	//Returns the region the user selected
	std::vector<cv::Vec2d> getSelectedRegion();

private:
    //Selects a region given the points the user selected
	void selectArea(std::vector<cv::Vec2d> pointsList);
	//Scans the region given the region points
	void scanArea(std::vector<cv::Vec2d> area_points);
	//Scans the whole region
	std::vector<ActuatorMoveOrder> scanAreaPrecision(std::vector<cv::Vec2d> area_points);
	//Calls ActuatorController movesQueue function to do multiple scans
	void addOrdersToQueue(std::vector<ActuatorMoveOrder> ordersList);
};