
#include "events.h"
#include <opencv2/core/core.hpp>

#ifndef ACTUATOR_CONTROLLER_H
#define ACTUATOR_CONTROLLER_H

/* class ActuatorController
    Author: Declan White, Aaron Nguyen
    Description:
        A stateful class responsible for controlling the heading and attitude
        actuators.
    Changelog:
        [2014-09-02 DWW] Created.
*/
class ActuatorController {
public: // Public concept classes
    
    /* enum struct ActuatorController::OrderKind
        Author: Declan White
        Description: TODO
        Values:
            STOP: Cease all actuator movement as promptly as possible.
            MOVE: Move to a given orientation in a given time range.
            RESET: Move to an unspecified valid position.
        Changelog:
            [2014-09-02 DWW] Created.
    */
    enum struct OrderKind {
        NONE,
        STOP,
        MOVE,
        RESET
    }
    
    /* struct ActuatorController::ActuatorMoveOrder
        Author: Declan White
        Description:
            Contains information a desired move operation as ordered by external
            directing class.
        Changelog:
            [2014-09-02 DWW] Created.
    */
    struct Order {
        OrderKind kind;
        
        // for NONE, STOP, MOVE and RESET
        moment receivalTime;
        
        // for STOP, MOVE and RESET
        moment desiredStartTime;
        
        // for MOVE
        moment desiredCompletionTime;
        cv::Vec2d desiredOrientationDeg;
    }
    
    /* interface ActuatorController::ActuatorOrderProvider
        Author: Declan White, Aaron Nguyen
        Description:
            An interface for the concept of a class that provides orders to the
            actuator controller.
        Changelog:
            [2014-09-02 DWW] Created.
    */
    class OrderProvider {
        public:
            virtual OrderProvider() = default;
            virtual ~OrderProvider() = default;
        
        public:
            Event<void(ActuatorMoveOrder)> eventCurrentOrderChanged;
        
        public:
            ActuatorMoveOrder getCurrentOrder();
            void markOrderComplete(moment completionTime);
    }
    
    /* enum struct ActuatorController::ActuatorSystemState
        Author: Declan White
        Description:
            Used to convey the current state of the actuator system as a whole.
            If one actuator has an error, the entire assembly is considered in
            an "error" state; this holds for the other "negative" states.
        Values:
            DISCONNECTED: At least one actuator is disconnected (default state).
            CONNECTED: All actuators have responded by their state is not yet
                known.
            READY: All actuators are connected and their states are known.
            ERROR: At least one actuator is in an error state.
        Changelog:
            [2014-09-02 DWW] Created.
    */
    enum struct ActuatorSystemState {
        DISCONNECTED,
        CONNECTED,
        READY,
        ERROR,
    }
    

public: // Publicly subscribable events
    
    /* ActuatorController::eventControllerStatusChange
        Author: Declan White
        Description:
            This event is fired when the overall status of the actuator system
            has changed.
        Parameters:
            ActuatorSystemState state: The reported current state of the
                actuator system.
        Changelog:
            [2014-09-02 DWW] Created.
            [2014-09-02 DWW] Renamed from eventDeviceStatusChange to
                eventControllerStatusChange.
    */
    Event<void(ActuatorSystemState)> eventControllerStatusChange;
    
    /* ActuatorController::eventOrientationUpdate
        Description:
            This event is fired when the actuators report an updated orientation
            reading.
        Parameters:
            cv::Vec2d orientation: The reported current position of the actuators.
        Changelog:
            [2014-09-02 DWW] Created.
    */
    Event<void(cv::Vec2d)> eventOrientationUpdate;
    
public:
    /* ActuatorController::(primary constructor)
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    ActuatorController(ParamContext params);
    
    /* ActuatorController::~ActuatorController
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    ~ActuatorController();
    
    /* ActuatorController::connect
        Author: Declan White
        Description:
            This method commands the controller to initiate a connection with
            the actuators.
            If the actuators are already connecting or connected, this method
            will do nothing.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    void connect();
    
    /* ActuatorController::disconnect
        Author: Declan White
        Description:
            This method commands the controller to cleanly disconnect from the
            actuators.
            If the actuators are already disconnected, this method will do
            nothing.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    void disconnect();
    
    /* ActuatorController::halt
        Author: Declan White
        Description:
            This method commands the controller to halt all communcation with
            the actuators and disconnect all intermediate communication mediums
            without following the proper shutdown procedure.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    void halt();
    

private:
    
    

private:
    Hook hookOnSerialData;
    

};

#endif//ACTUATOR_CONTROLLER_H

