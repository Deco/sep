#include "serial_port.h"

#include <exception>
#include <algorithm>
#include <limits>
#include <vector>
#include <cstring>
#include <assert.h>

 #include <sys/stat.h>
#include <termios.h>

// Serial requirements
#include <stdio.h>
#include <unistd.h> // UNIX standard function definitions
#include <fcntl.h> // File control definitions

#include <sys/ioctl.h>



typedef unsigned char byte;
//int fileDescriptor; // temp added to compile. where this is used outside of openDevice, it should be internalData typecast from void* and dereferenced to get fd

inline void throwSerialError(const char *message, const char *deviceName) {
    char *what;
    assert(0 < asprintf(&what,
        "%s (\"%s\"): %s",
        message,
        deviceName,
        strerror(errno)
    ));
    std::string str(what);
    free(what);
    //throw SerialError(str); //not sure what to include for this?
    throw std::exception();
}

struct SeralPortInternalData {
    int fileDescriptor;
};

/* SerialPort::(primary constructor)
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
SerialPort::SerialPort()
{
    // 
}

/* SerialPort::~SerialPort
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
*/
SerialPort::~SerialPort()
{
    closeDevice();
}

/* SerialPort::open
    Author: Declan White
    Attribution:
        Original code modified from IKAROS project (GNU GPL licence).
        https://github.com/ikaros-project/ikaros/blob/master/Source/Kernel/IKAROS_Serial_BSD.cc
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-10-21 CW] Corrected syntax to compile.
*/
void SerialPort::openDevice(const std::string &&deviceName, unsigned long baudRate, bool shouldBlock)
{
    int optionFlags = 0;
    // Open serial device in read & write mode
    optionFlags |= O_RDWR;
    // "Prevent the OS from assigning the opened file as the process's
    // controlling terminal when opening a TTY device file."
    optionFlags |= O_NOCTTY;
    if(!shouldBlock) {
        // Open the device in non-blocking mode to prevent reading and writing 
        // from locking the IO thread
        optionFlags |= O_NDELAY;
    }
   
    int fileDescriptor = open(deviceName.c_str(), optionFlags); // moved decleration to global for now so other functions can use. maybe put it in class as a field
    if(-1 == fileDescriptor) {
        throwSerialError("failed to open serial device", deviceName.c_str());  
        return;
    }
    
    //fcntl(fileDescriptor, F_SETFL, 0);
    
    struct termios fileOptions;
    if(-1 == tcgetattr(fileDescriptor, &fileOptions)) { // fileOptions used to be options, not sure if correct
        throwSerialError("failed get serial device options", deviceName.c_str());
        return;
    }
    
    #ifndef MAC_OS_X
        // Setting the baud rate for non-OSX POSIX-compliant platforms
        if(-1 == cfsetispeed(&fileOptions, baudRate)) {
            throwSerialError("failed to set baud rate for serial input device", deviceName.c_str());
            return;
        }
        if(-1 == cfsetospeed(&fileOptions, baudRate)) {
            throwSerialError("failed to set baud rate for serial output device", deviceName.c_str());
            return;
        }
    #endif
    
    // Control modes
    fileOptions.c_cflag |= CS8; // Set character size to 8bit
    fileOptions.c_cflag |= CLOCAL; // Ignore modem control lines
    fileOptions.c_cflag |= CREAD; // Enable receiver
    // Input modes
    fileOptions.c_iflag = IGNPAR; // Ignore graming errors and parity errors.
    // Output modes
    fileOptions.c_oflag = 0;
    // Local modes
    fileOptions.c_lflag = 0;
    // Special characters
    fileOptions.c_cc[VMIN]  = 0; // Require zero characters for a successful read
    fileOptions.c_cc[VTIME] = 1; // Wait up to 1/10 seconds for characters while reading
    
    // Flush the input and output buffers
    if(-1 == tcflush(fileDescriptor, TCIOFLUSH)) {
        throwSerialError("failed to flush serial device", deviceName.c_str());
            return;
    }
    
    // Apply the new options
    if(-1 == tcsetattr(fileDescriptor, TCSANOW, &fileOptions)) {
        throwSerialError("failed to set serial device attributes", deviceName.c_str());
            return;
    }
    
    #ifdef MAC_OS_X
        if(-1 == cfmakeraw(&options)) { // necessary for ioctl to function; must come after setattr
            throwSerialError("failed to make serial device raw", deviceName.c_str());
            return;
        }
        
        const speed_t TGTBAUD = baudRate;
        if(-1 == ioctl(fd, IOSSIOSPEED, &TGTBAUD)) { // sets also non-standard baud rates
            throwSerialError("failed to set baud rate for serial device \"%s\": %s", deviceName.c_str());
            return;
        }
    #endif
    
    auto bsdInternalData = std::make_shared<SeralPortInternalData>();
    bsdInternalData->fileDescriptor = fileDescriptor;
    internalData = bsdInternalData;
    
}

/* SerialPort::close
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-10-21 CW] Corrected syntax to compile.
*/
void SerialPort::closeDevice()
{
    if(!internalData) {
        throw std::logic_error("cannot close a closed serial device");
        return;
    }
    
    SeralPortInternalData *bsdInternalData = (SeralPortInternalData*)internalData.get();
    
    if(-1 == close(bsdInternalData->fileDescriptor)) {
        throwSerialError("failed to close serial device", "");
        return;
    }
    
    internalData.reset();
    
}

/* SerialPort::isOpen
    Author: Declan White
    Attribution:
        Code for checking if a file descriptor is still valid adapted from
        StackOverflow answer (CC-Wiki with attribution required).
        http://stackoverflow.com/a/12340725/837856
    Description: TODO
    Parameters: TODO
    Returns: TODO
    Throws: TODO
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-10-21 CW] Corrected syntax to compile.
*/
bool SerialPort::isOpen()
{
    if(!internalData) {
        return false;
    }
    

    SeralPortInternalData *bsdInternalData = (SeralPortInternalData*)internalData.get();
    
    if(-1 == fcntl(bsdInternalData->fileDescriptor, F_GETFD)) {
        return false;
    } else if(errno == EBADF) {
        return false;
    }
    return true;
}

/* SerialPort::write
    Author: Declan White
    Throws: TODO
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-10-21 CW] Corrected syntax to compile.
*/
size_t SerialPort::writeDevice(const std::vector<byte> &&data)
{
    if(!internalData) {
        throw std::logic_error("cannot write to a closed serial device");
        return 0;
    }
    
    SeralPortInternalData *bsdInternalData = (SeralPortInternalData*)internalData.get();
    
    int bytesWritten = write(bsdInternalData->fileDescriptor, &data[0], data.size());
    
    if(-1 == bytesWritten) {
        throwSerialError("failed to write to serial device", "");
        return 0;
    }
    return bytesWritten;
}

/* SerialPort::read
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-10-21 CW] Corrected syntax to compile.
*/
size_t SerialPort::readDevice(std::vector<byte> &data)
{
    readDevice(data, std::numeric_limits<size_t>::max());
}
size_t SerialPort::readDevice(std::vector<byte> &data, size_t maxSize)
{
    if(!internalData) {
        throw std::logic_error("cannot read from a closed serial device");
        return 0;
    }
    
    SeralPortInternalData *bsdInternalData = (SeralPortInternalData*)internalData.get();
    
    int bytesAvailable = 0;
    if(-1 == ioctl(bsdInternalData->fileDescriptor, FIONREAD, &bytesAvailable)) {
        throwSerialError("failed to read available byte count from serial device", "");
        return 0;
    }
    if(bytesAvailable > 0) {
        int bytesToRead = std::min(bytesAvailable, (int)maxSize);
        if(data.size() < bytesToRead) {
            data.resize(bytesToRead);
        }
        int bytesRead = read(bsdInternalData->fileDescriptor, &data[0], bytesToRead);
        
        if(-1 == bytesRead) {
            throwSerialError("failed to read from serial device", "");
            return 0;
        }
        return bytesRead;
    }
    return 0;
}


/* SerialPort::flushWrite
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-10-21 CW] Corrected syntax to compile.
*/
void SerialPort::flushWrite()
{
    SeralPortInternalData *bsdInternalData = (SeralPortInternalData*)internalData.get();
    if(-1 == tcflush(bsdInternalData->fileDescriptor, TCOFLUSH)) { 
        throwSerialError("failed to flush serial device output object", "");
    }
}

/* SerialPort::flushRead
    Author: Declan White
    Changelog:
        [2014-09-04 DWW] Created.
        [2014-10-21 CW] Corrected syntax to compile.
*/
void SerialPort::flushRead()
{
    SeralPortInternalData *bsdInternalData = (SeralPortInternalData*)internalData.get();
    if(-1 == tcflush(bsdInternalData->fileDescriptor, TCIFLUSH)) { 
        throwSerialError("failed to flush serial device output object", "");
    }
}
