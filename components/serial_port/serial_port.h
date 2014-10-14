
#include <cstddef>
#include <exception>

#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

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
    void open(
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
    void close();
    
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
    size_t write(const std::vector<byte> &&data);
    
    /* SerialPort::read
        Author: Declan White
        Description: TODO
        Parameters: TODO
        Returns: TODO
        Throws: TODO
        Changelog:
            [2014-09-04 DWW] Created.
    */
    size_t read(std::vector<byte> &data);
    size_t read(std::vector<byte> &data, size_t maxSize);
    
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
    
private
    std::shared_ptr<void> internalData;
};

#endif//SERIAL_PORT_H

