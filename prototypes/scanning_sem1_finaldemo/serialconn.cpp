#include "serialconn.h"

/* Created by: Chris Webb
 * Date: 7/6/14
 * Function: SerialConn Constructor
 * Import:
 *      deviceName: String representing the Serial Port that device is attached on.
 *      baudRate: uint representing the baud rate used by device.
 * Description:
 * Construct a SerialConn object representing a Serial Connection to imported device.  
 * Initialises the io service and serial port to be used for reading and writing and
 * sets the baud rate.
 */
SerialConn::SerialConn(const std::string deviceName, unsigned int baudRate)
    : io(), sport(io, deviceName)
{
    sport.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
}



/* Created by: Chris Webb
 * Date: 7/6/14
 * Function: close()
 * Description:
 * Closes the serial connection to the serial port.
 */
void SerialConn::close(){
    sport.close();
}



/* Created by: Chris Webb
 * Date: 7/6/14
 * Function: read()
 * Import:
 *      bufferPtr: A pointer to a location to store bytes read from device.
 *      bufferSize: int representing how many bytes to read.
 * Description:
 * Read bytes from device and return the number of bytes successfully read.
 */
int SerialConn::read(char *bufferPtr, int bufferSize){
    size_t amountRead;
    amountRead = boost::asio::read(sport, boost::asio::buffer(bufferPtr, bufferSize));
    return (int)amountRead;
}



/* Created by: Chris Webb
 * Date: 7/6/14
 * Function: write()
 * Import:
 *      bufferPtr: A pointer to a location of bytes waiting to be read to device.
 *      bufferSize: int representing how many bytes to write.
 * Description:
 * Write bytes to device and return the number of bytes successfully read.
 */
int SerialConn::write(char *bufferPtr, int dataSize){
    size_t amountWritten;
    amountWritten = boost::asio::write(sport, boost::asio::buffer(bufferPtr, dataSize));
    return (int)amountWritten;
}



// Not implemented
void SerialConn::flush(){

}
