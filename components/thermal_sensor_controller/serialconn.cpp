#include "serialconn.h"

SerialConn::SerialConn(const std::string deviceName, unsigned int baudRate)
    : io(), sport(io, deviceName)
{
    sport.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
}

void SerialConn::close(){
//    if (sport.isOpen()) {
        sport.close();
   // }

}


int SerialConn::read(char *bufferPtr, int bufferSize){
    size_t amountRead;
    amountRead = boost::asio::read(sport, boost::asio::buffer(bufferPtr, bufferSize));
    return (int)amountRead;
}


int SerialConn::write(char *bufferPtr, int dataSize){
    size_t amountWritten;
    amountWritten = boost::asio::write(sport, boost::asio::buffer(bufferPtr, dataSize));
    return (int)amountWritten;
}


void SerialConn::flush(){

}