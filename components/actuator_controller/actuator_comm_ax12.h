
#include "actuator_comm.h"

#include "threads.h"

#ifndef ACTUATOR_COMM_AX12_H
#define ACTUATOR_COMM_AX12_H


/* abstract class ActuatorCommAX12
    Author: Declan White
    Description: TODO
    Changelog:
        [2014-09-04 DWW] Created.
*/
class ActuatorCommAX12 : ActuatorComm {
public:
    
    /* ActuatorCommAX12::(primary constructor)
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    virtual ActuatorCommAX12(
        std::shared_ptr<ApplicationContext> app,
        ParamContext params,
        const std::shared_ptr<SerialPort> &serialPort
    );
    
    /* ActuatorCommAX12::~ActuatorCommAX12
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    virtual ~ActuatorCommAX12();
    

private:
    
    /* ActuatorCommAX12::serialThreadFunc
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    void serialThreadFunc();
    
    /* ActuatorCommAX12::sendPacket
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    void sendPacket(byte id, byte instruction, const std::vector<byte> &data);
    
    /* ActuatorCommAX12::receivePacket
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    void receivePacket(
        byte &idRef, byte &errorRef,
        std::vector<byte> &parameterListRef
    );
    
    /* ActuatorCommAX12::readByte
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    byte readByte(byte id, byte address);
    
    /* ActuatorCommAX12::readShort
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    short readShort(byte id, byte address);
    
    /* ActuatorCommAX12::writeByte
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    void writeByte(byte id, byte address, byte  value);
    
    /* ActuatorCommAX12::writeShort
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    void  writeShort(byte id, byte address, short value);
    

private:
    struct ActuatorData {
        int id;
        int dynamixelId;
        ActuatorInfo info;
        ActuatorState state;
        std::shared_ptr<ActuatorError> errorPtr;
        double goalPos; bool goalPosIsDirty;
        double goalVel; bool goalVelIsDirty;
    }
    
    enum struct ActuatorOrderKind {
        INITIATE_MOVEMENT,
    }
    

private:
    std::shared_ptr<ApplicationContext> app;
    const std::shared_ptr<const boost::asio::io_service> ios;
    std::shared_ptr<ParamContext> paramsPtr;
    
    std::shared_ptr<std::thread> serialThreadPtr;
    atom<bool> serialThreadShouldDisconnect;
    std::shared_ptr<SerialPort> serialPortPtr;
    
    
    atom<std::vector<ActuatorData>> actuatorDataList;
    
    atom<std::vector<const ActuatorInfo>> actuatorInfoList;
    
    
    hook hookParamSampleRateChanged;
};

#endif//ACTUATOR_COMM_AX12_H

