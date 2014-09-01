
#include "events.h"
#include <opencv2/core/core.hpp>

struct ActuatorMoveOrder
{
    cv::Vec2d orientationDeg;
    double duration;
}

/* class ActuatorController
    A stateful class responsible for controlling the heading and attitude actuators
    Created by: Aaron Nguyen, Declan White
*/
class ActuatorController
{
public: // Public events
    // event OrientationUpdate
    // Fired when the position of the actuators has changed
    Event<void(cv::Vec2d)> eventOrientationUpdate;

public:
    ActuatorController();
    ~ActuatorController();
    
    void connect();
    void disconnect();
    
    void getOrientationLimit

private:
    Atom<std::queue<ActuatorMoveOrder>> orderQueue;
    
    Observed<bool> ready;
    Observed<cv::Vec2d> currentOrientationDeg;
    
private:
    Hook hookOnSerialData;

public: // Internal events
    // event AddOrder
    // In group 0, the attached order is appended to the order queue
    Event<void(ActuatorMoveOrder)> eventNewOrder;
    
    // event ClearOrderQueue
    // In group 0, the order queue is cleared
    Event<void()> eventClearOrderQueue;
    
    // event eventCompleteOrder
    // Fired when an order has been completed by the actuators
    Event<void()> eventCompleteOrder;

};

