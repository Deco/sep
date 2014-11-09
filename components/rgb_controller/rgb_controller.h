/* FILE: rgb_controller.h
 * AUTHOR: Thomas Smallridge
 * CREATED: 2/11/14
 */

#include "application_core.h"
#include <opencv2/opencv.hpp>

#define MAX_BUFFER_SIZE 10

 class RgbController {

//Public class field definitions
public:
	//The OS-assigned device number of the camera
	const int deviceNumber;

//Private class field definitions
private:
	//The video feed itself
	cv::VideoCapture camera;

	//The buffer of frames
	std::deque<std::shared_ptr<cv::Mat>> frameBuffer;

    std::shared_ptr<ApplicationCore> core;

    boost::asio::deadline_timer timer;

public:
	RgbController(
        const std::shared_ptr<ApplicationCore> &core,
		int inDeviceNumber
	);

	//Public function forward declarations
	//Details outlined in .cpp file
	void initCamera();

	void captureFrame();

	std::shared_ptr<cv::Mat> popFrame();

	void killCamera();

	void setDeviceName(std::string inDeviceName);

	std::string getDeviceName();

	int getDeviceNumber(); 

 };