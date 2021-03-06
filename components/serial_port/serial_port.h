#include <cstddef>
#include <exception>
#include <memory>
#include <vector>

#include "common.h"
#include "events.h"


#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

typedef unsigned char byte;


/* class SerialPort
    Author: Declan White
    Description: TODO
    Changelog:
        [2014-09-04 DWW] Created.
*/
class SerialPort {
public:
    
    /* SerialPort::SerialFailure
        Author: Declan White
        Description: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    class SerialFailure : std::runtime_error {
        // 
    };

public:
    /* SerialPort::(primary constructor)
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    SerialPort();
    
    /* SerialPort::~SerialPort
        Author: Declan White
        Description: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    ~SerialPort();
    

public:
    
    /* SerialPort::open
        Author: Declan White

        Description: TODO
        Parameters: TODO
        Returns: TODO
        Throws: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    void openDevice(
        const std::string &&deviceName,
        unsigned long baudRate,
        bool shouldBlock = false
    );
    
    /* SerialPort::close
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Throws: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    void closeDevice();
    
    /* SerialPort::isOpen
        Author: Declan White
        Description: TODO

        Parameters: TODO
        Returns: TODO
        Throws: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    bool isOpen();
    
    /* SerialPort::write
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO

        Throws: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    size_t writeDevice(const std::vector<byte> &data);
    size_t writeDevice(char* data, int size);
    
    /* SerialPort::read
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Throws: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    size_t readDevice(std::vector<byte> &data);
    size_t readDevice(std::vector<byte> &data, size_t maxSize);
    
    size_t readDevice(char* data);
    size_t readDevice(char* data, size_t maxSize);

    /* SerialPort::flushWrite
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO

        Throws: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    void flushWrite();
    
    /* SerialPort::flushRead

        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Throws: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    void flushRead();

    int getAvailable();
    
private:
    std::shared_ptr<void> internalData;


public:
    hook registerOnSerialDataReadyCallback(
        std::function<void (SerialPort &)> callbackPtr
    );
};

#endif//SERIAL_PORT_H

