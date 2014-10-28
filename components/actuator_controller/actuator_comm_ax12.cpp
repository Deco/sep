#include "actuator_comm_ax12.h"

/* Dynamixel AX-12 sentinel values
    Attribution:
        Dynamixel AX-12 manual.
        http://www.electronickits.com/robot/BioloidAX-12(english).pdf
    Description: TODO
    Changelog:
        [2014-09-04 DWW] Inserted.
*/

// Instruction packet codes
#define INST_PING               1
#define INST_READ               2
#define INST_WRITE              3
#define INST_REG_WRITE          4
#define INST_ACTION             5
#define INST_RESET              6
#define INST_SYNC_WRITE         131

// Memory addresses: EEPROM area
#define P_MODEL_NUMBER_L      0
#define P_MODOEL_NUMBER_H     1
#define P_VERSION             2
#define P_ID                  3
#define P_BAUD_RATE           4
#define P_RETURN_DELAY_TIME   5
#define P_CW_ANGLE_LIMIT_L    6
#define P_CW_ANGLE_LIMIT_H    7
#define P_CCW_ANGLE_LIMIT_L   8
#define P_CCW_ANGLE_LIMIT_H   9
#define P_SYSTEM_DATA2        10
#define P_LIMIT_TEMPERATURE   11
#define P_DOWN_LIMIT_VOLTAGE  12
#define P_UP_LIMIT_VOLTAGE    13
#define P_MAX_TORQUE_L        14
#define P_MAX_TORQUE_H        15
#define P_RETURN_LEVEL        16
#define P_ALARM_LED           17
#define P_ALARM_SHUTDOWN      18
#define P_OPERATING_MODE      19
#define P_KP                  20
#define P_KD                  21
#define P_KI                  22
#define P_IDAMP               23

// Memory addresses: RAM area
#define P_TORQUE_ENABLE         24
#define P_LED                   25
#define P_CW_COMPLIANCE_MARGIN  26
#define P_CCW_COMPLIANCE_MARGIN 27
#define P_CW_COMPLIANCE_SLOPE   28
#define P_CCW_COMPLIANCE_SLOPE  29
#define P_GOAL_POSITION_L       30
#define P_GOAL_POSITION_H       31
#define P_GOAL_SPEED_L          32
#define P_GOAL_SPEED_H          33
#define P_TORQUE_LIMIT_L        34
#define P_TORQUE_LIMIT_H        35
#define P_PRESENT_POSITION_L    36
#define P_PRESENT_POSITION_H    37
#define P_PRESENT_SPEED_L       38
#define P_PRESENT_SPEED_H       39
#define P_PRESENT_LOAD_L        40
#define P_PRESENT_LOAD_H        41
#define P_PRESENT_VOLTAGE       42
#define P_PRESENT_TEMPERATURE   43
#define P_REGISTERED_INSTRUCTION 44
#define P_PAUSE_TIME            45
#define P_MOVING                46
#define P_LOCK                  47
#define P_PUNCH_L               48
#define P_PUNCH_H               49

/* ActuatorCommAX12::(primary constructor)
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
ActuatorCommAX12::ActuatorCommAX12(
    std::shared_ptr<ApplicationCore> appIn,
    const std::shared_ptr<Param> &&paramsIn,
    const std::shared_ptr<SerialPort> &serialPortIn
) : app(appIn),
    ios(appIn->getIOService()),
    paramsPtr(paramsIn),
    serialThreadPtr(nullptr),
    serialThreadShouldDisconnect(false),
    serialPortPtr(serialPortIn)
{
    
    /*paramsPtr->declare<duration>("sample_rate_sec");
    
    hookParamSampleRateSec = paramsPtr->observe(
        "sample_rate_sec",
        &ActuatorCommAX12::onSampleRateChanged, this
    );*/
    
}

/* ActuatorCommAX12::~ActuatorCommAX12
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
/*ActuatorCommAX12::~ActuatorCommAX12()
{
    disconnect();
}*/

/* ActuatorCommAX12::connect
    Author: Declan White
    Changelog:
        [2014-09-02 DWW] Created.
        [2014-09-04 DWW] Implemented.
*/
void ActuatorCommAX12::connect()
{
    // If the thread already exists, we're already connected or connecting..
    if(serialThreadPtr) {
        // ..so do nothing.
        return;
    }
    // Otherwise, start the thread.
    serialThreadPtr = std::make_shared<std::thread>(
        &ActuatorCommAX12::serialThreadFunc, this
    );
    
}

/* ActuatorCommAX12::disconnect
    Author: Declan White
    Changelog:
        [2014-09-02 DWW] Created.
        [2014-09-04 DWW] Implemented.
*/
void ActuatorCommAX12::disconnect()
{
    // If the thred isn't running then we're not connected..
    if(!serialThreadPtr) {
        // ..so do nothing.
        return;
    }
    // Otherwise, signal the thread that it's time to disconnected..
    serialThreadShouldDisconnect.set(true);
    // ..and wait for it to finish.
    serialThreadPtr->join();
    // No more thread!
    serialThreadPtr.reset();
}

/* ActuatorCommAX12::getActuatorInfoList
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-09-04 DWW] Implemented.
*/
void ActuatorCommAX12::obtainActuatorInfoList(
    std::vector<ActuatorComm::ActuatorInfo> &infoList
)
{
    // Lock the actuator data list..
    actuatorDataList.access_read([&](const std::vector<ActuatorData> & list) {
        infoList.reserve(list.size());
        
        // ..and retrieve all the actuator info structs.
        for(auto actuatorData : list) {
            infoList.push_back(actuatorData.info);
        }
    });
}

/* ActuatorCommAX12::getActuatorState
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-09-05 DWW] Implemented.
*/
ActuatorComm::ActuatorState ActuatorCommAX12::getActuatorState(int id)
{
    ActuatorState state;
    // Lock the actuator data list..
    actuatorDataList.access_read([&](const std::vector<ActuatorData> & list) {
        // ..and retrieve the state of the specified actuator.
        state = list.at(id).state;
    });
    return state;
}

/* ActuatorCommAX12::getActuatorState
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-09-05 DWW] Implemented.
*/
std::shared_ptr<ActuatorError> ActuatorCommAX12::getActuatorError(int id)
{
    std::shared_ptr<ActuatorError> errorPtr;
    // Lock the actuator data list..
    actuatorDataList.access_read([&](const std::vector<ActuatorData> & list) {
        // ..and retrieve the error of the specified actuator.
        errorPtr = list.at(id).errorPtr;
    });
    return errorPtr;
}

/* ActuatorCommAX12::recoverActuator
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
void ActuatorCommAX12::recoverActuator(int id)
{
    throw std::runtime_error("NYI");
}

/* ActuatorCommAX12::getActuatorGoalPos
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-09-05 DWW] Implemented.
*/
double ActuatorCommAX12::getActuatorGoalPos(int id)
{
    double goalPos;
    // Lock the actuator data list..
    actuatorDataList.access_read([&](const std::vector<ActuatorData> & list) {
        // ..and retrieve the goalPos of the specified actuator.
        goalPos = list.at(id).goalPos;
    });
    return goalPos;
}

/* ActuatorCommAX12::setActuatorGoalPos
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-09-05 DWW] Implemented.
*/
void ActuatorCommAX12::setActuatorGoalPos(int id, double posDeg)
{
    // Lock the actuator data list..
    actuatorDataList.access([&](std::vector<ActuatorData> & list) {
        // ..and set the goalPos of the specified actuator.
        list.at(id).goalPos = posDeg;
        // Ensure the goal pos is marked as dirty so that the new value
        // is sent to the actuator when `initiateMovement` is called.
        list.at(id).goalPosIsDirty = true;
    });
}

/* ActuatorCommAX12::getActuatorGoalVel
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
double ActuatorCommAX12::getActuatorGoalVel(int id)
{
    double goalVel;
    // Lock the actuator data list..
    actuatorDataList.access_read([&](const std::vector<ActuatorData> & list) {
        // ..and retrieve the goalVel of the specified actuator.
        goalVel = list.at(id).goalVel;
    });
    return goalVel;
}

/* ActuatorCommAX12::setActuatorGoalVel
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-09-13 DWW] Implemented.
*/
void ActuatorCommAX12::setActuatorGoalVel(int id, double velDegPerSec)
{
    // Lock the actuator data list..
    actuatorDataList.access([&](std::vector<ActuatorData> & list) {
        // ..and set the goalVel of the specified actuator.
        list.at(id).goalVel = velDegPerSec;
        // Ensure the goal vel is marked as dirty so that the new value
        // is sent to the actuator when `initiateMovement` is called.
        list.at(id).goalVelIsDirty = true;
    });
}

/* ActuatorCommAX12::initiateMovement
    Author: Declan White
    Changelog:
        [2014-09-13 DWW] Created.
        [2014-09-13 DWW] Implemented.
*/
void ActuatorCommAX12::initiateMovement(int id)
{
    // Lock the actuator data list..
    actuatorDataList.access_read([&](const std::vector<ActuatorData> & list) {
        for(ActuatorData data : list) {
            if(data.goalVelIsDirty) {
                
            }
        }
    });
}

/* ActuatorCommAX12::serialThreadFunc
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
void ActuatorCommAX12::serialThreadFunc()
{
    serialPortPtr = std::make_shared<SerialPort>();
    
}

/* ActuatorCommAX12::sendPacket
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
void ActuatorCommAX12::sendPacket(
    byte id, byte instruction, const std::vector<byte> &data
) {
    //
}

/* ActuatorCommAX12::receivePacket
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
void ActuatorCommAX12::receivePacket(
    byte &idRef, byte &errorRef, std::vector<byte> &parameterListRef
) {
    //
}


/* ActuatorCommAX12::readByte
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
byte ActuatorCommAX12::readByte(byte id, byte address)
{
    //
}

/* ActuatorCommAX12::readShort
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
short ActuatorCommAX12::readShort(byte id, byte address)
{
    //
}

/* ActuatorCommAX12::writeByte
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
void ActuatorCommAX12::writeByte(byte id, byte address, byte value)
{
    //
}

/* ActuatorCommAX12::writeShort
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
void ActuatorCommAX12::writeShort(byte id, byte address, short value)
{
    //
}