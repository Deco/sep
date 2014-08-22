#include <opencv2/opencv.hpp>
#include "ActuatorController/ActuatorController.h"

class Director {
private:
	ActuatorController *ac = new ActuatorController("/dev/tty.usbserial-A9S3VTXD");
	bool isIdle = true;
public:
	Director();
	void update();
	void haltMovement();
	std::vector<cv::Vec2d> performRegionScan(std::vector<cv::Vec2d> pointsList);
	std::vector<ActuatorMoveOrder> performFullScan();
	void performManualPan(ActuatorMoveOrder order);
	void clearCollectedData();
	cv::Vec2d getCurrentPositionAC();

private:
	std::vector<cv::Vec2d> selectArea(std::vector<cv::Vec2d> pointsList);
	void scanArea(std::vector<cv::Vec2d> area_points);
	std::vector<ActuatorMoveOrder> scanAreaPrecision(std::vector<cv::Vec2d> area_points);
	void addOrdersToQueue(std::vector<ActuatorMoveOrder> ordersList);
};