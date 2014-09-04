
#include "common.h"
#include "serial_port.h"

#ifndef ACTUATOR_COMM_H
#define ACTUATOR_COMM_H

/* abstract class ActuatorComm
    Author: Declan White
    Description:
        A stateful class responsible for communicating with a set of actuators
        over a serial connection.
        Uses a seperate thread to allow for asynchronous communcation with and
        management of the actuators and various associated IO handles.
    Changelog:
        [2014-09-04 DWW] Created.
*/
class ActuatorComm {
public:
    
    /* ActuatorComm::ActuatorInfo
        Author: Declan White
        Description: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    struct ActuatorInfo {
        int id;
        std::string kind;
        std::string description;
        double angleMinDeg;
        double angleMaxDeg;
        double speedMinDegPerSec;
        double speedMaxDegPerSec;
        double torqueMinKgCm;
        double torqueMaxKgCm;
    };
    
    /* ActuatorComm::ActuatorState
        Author: Declan White
        Description: TODO
        Values:
            DISCONNECTED: The actuator is disconnected.
            CONNECTED: The actuator is connected; it could be ready or in a
                yet-to-be-detected error state.
            ERROR: The actuator is in an error state or a yet-to-be-detected
                recovered state.
        Changelog:
            [2014-09-04 DWW] Created.
    */
    enum struct ActuatorState {
        DISCONNECTED,
        CONNECTED,
        ERROR
    };
    

public:
    
    /* ActuatorComm::eventActuatorStateChange
        Author: Declan White
        Description:
            This event is fired when the overall state of an actuator changes.
        Parameters:
            const ActuatorInfo& info: The information of the relavent actuator
                (allows identification).
            ActuatorState state: The reported state of the actuator.
        Changelog:
            [2014-09-02 DWW] Created.
            [2014-09-02 DWW] Renamed from eventDeviceStatusChange to
                eventControllerStatusChange.
    */
    event<void(const ActuatorInfo&, ActuatorState)> eventActuatorStateChange;
    
    /* ActuatorComm::eventActuatorMovementUpdate
        Author: Declan White
        Description:
            This event is fired when an actuator reports an updated position or
            movement state.
        Parameters:
            const ActuatorInfo& info: The information of the relavent actuator
                (allows identification).
            double pos: The reported current angular position of the actuator in
                degrees.
            double vel: The reported current angular velocity of the actuator in
                degrees.
            bool isMoving: The reported current movement state of the actuator.
        Changelog:
            [2014-09-02 DWW] Created.
            [2014-09-02 DWW] Renamed from eventDeviceStatusChange to
                eventControllerStatusChange.
    */
    event<void(const ActuatorInfo&, double, double, bool)> eventActuatorMovementUpdate;
    

public:
    
    /* ActuatorComm::(primary constructor)
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Notes:
            This may need to be modified to accept multiple serial connections
            if not all the actuators are connected via one serial port.
        Changelog:
            [2014-09-04 DWW] Created.
    */
    virtual ActuatorComm(
        ParamContext params,
        const std::shared_ptr<SerialPort> &serialPort
    );
    
    /* ActuatorComm::~ActuatorComm
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    virtual ~ActuatorComm();
    

public:
    
    /* ActuatorComm::connect
        Author: Declan White
        Description:
            This method commands the communicator to initiate a connection with
            the actuators and bring them to state where they are ready to move.
            If the actuators are already connecting or connected, this method
            will do nothing.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    virtual void connect();
    
    /* ActuatorComm::disconnect
        Author: Declan White
        Description:
            This method commands the communicator to cleanly disconnect from the
            actuators.
            If the actuators are already disconnected, this method will do
            nothing.
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-02 DWW] Created.
    */
    virtual void disconnect();
    
    /* ActuatorComm::getActuatorInfoList
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    virtual void getActuatorInfoList(
        std::vector<ActuatorComm::ActuatorInfo> &infoList
    ) const;
    
    /* ActuatorComm::getActuatorState
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    virtual ActuatorState getActuatorState(int id) const;
    
    /* ActuatorComm::getActuatorState
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: A shared_ptr to an ActuatorError describing the error state of
            the actuator; this will be a nullptr if there is no error state
            recorded
        Changelog:
            [2014-09-04 DWW] Created.
    */
    virtual std::shared_ptr<ActuatorError> getActuatorError(int id) const;
    
    /* ActuatorComm::recoverActuator
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    virtual void recoverActuator(int id);
    
    /* ActuatorComm::getActuatorGoalPos
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Throws: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    virtual double getActuatorGoalPos(int id) const;
    virtual void   setActuatorGoalPos(int id, double posDeg);
    
    /* ActuatorComm::getActuatorGoalVel
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Throws: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    virtual double getActuatorGoalVel(int id) const;
    virtual void   setActuatorGoalVel(int id, double velDegPerSec);
    
    // [2014-09-04 DWW] I'm pretty sure this belongs as a parameter.
    ///* ActuatorComm::getActuatorGoalTorque
    //    Author: Declan White
    //    Description: TODO
    //    Parameters: TODO
    //    Returns: TODO
    //    Throws: TODO
    //    Changelog:
    //        [2014-09-04 DWW] Created.
    //*/
    //virtual double getActuatorGoalTorque(int id);
    //virtual void   setActuatorGoalTorque(int id, double torqueKgCm);
    
    // [2014-09-04 DWW] Another brand of actuator may not require polling (it may emit
    // events or something), so this belongs as a parameter.
    ///* ActuatorComm::getActuatorMovementSampleRate
    //    Author: Declan White
    //    Description: TODO
    //    Parameters: TODO
    //    Returns: TODO
    //    Throws: TODO
    //    Changelog:
    //        [2014-09-04 DWW] Created.
    //*/
    //virtual duration getActuatorMovementSampleRate(int id);
    //virtual void setActuatorMovementSampleRate(int id, duration sampleRate);
    

};

#endif//ACTUATOR_COMM_H

