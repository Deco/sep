/* FILE: rgb_controller.cpp
 * AUTHOR: Thomas Smallridge
 * CREATED: 29/10/14
 * PURPOSE: Controller for the rgb camera feed.
 *
 * CHANGELOG:
 * 29/10/14: File created, basic class structure/fields/constructor written
 * 2/11/14: Majorly updated. Added alternate constructor and implemented
 *			frame capturing functionality. Refactored into .cpp & .h file
 *
 */

#include "rgb_controller.h"
#include <opencv2/opencv.hpp>

using namespace std;

//Default constructor
//This contains the MINIMUM required to setup the video feed
RgbController::RgbController(
    const std::shared_ptr<ApplicationCore> &coreIn,
	int inDeviceNumber
)
	: core(coreIn)
	, deviceNumber(inDeviceNumber)
	, frameBuffer()
	, camera(deviceNumber)
	, timer(
		*core->getIOService(),
		boost::posix_time::seconds(5)
	)
{
	//
}

/* Initialises the camera feed, given the device number of the camera
 * as assigned by the operating system.
 */
void RgbController::init()
{
	//Initialises and opens the video feed with the device number

	//Checks to see if the camera opened succesfully
	if(!camera.isOpened()) {
		std::stringstream ss;
		ss << "Camera could not be opened. DN[" << deviceNumber << "]";
		throw new std::runtime_error(
        	ss.str()
        );
	}

	// timer.async_wait(
	// 	std::bind(&RgbController::captureFrame, this)
	// );
	// timer.expires_at(timer.expires_at() + boost::posix_time::seconds(1));
}

//Returns the next frame from the camera. 
void RgbController::captureFrame()
{
	//Declare the frame to be outputted
	std::shared_ptr<cv::Mat> framePtr = std::make_shared<cv::Mat>();
	//Extracts the next frame from the camera
	camera >> *framePtr;

	std::cout << "uwatttt: " << framePtr->rows << std::endl;

	frameBuffer.push_back(framePtr);
	if(frameBuffer.size() > MAX_BUFFER_SIZE) {
		frameBuffer.pop_front();
	}

	std::cout << "yay" << std::endl;

	// timer.async_wait(
	// 	std::bind(&RgbController::captureFrame, this)
	// );
	// timer.expires_at(timer.expires_at() + boost::posix_time::seconds(1));
}

std::shared_ptr<cv::Mat> RgbController::popFrame()
{
	std::shared_ptr<cv::Mat> framePtr = frameBuffer.front();
	frameBuffer.pop_front();
	return framePtr;
}

//Releases the video feed, to be done on system end
void RgbController::killCamera()
{
	camera.release();
}

//Returns the OS-assigned ID for the camera (default 0)
int RgbController::getDeviceNumber()
{
	return deviceNumber;
}


