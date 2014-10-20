
#ifndef SERIALCONN_H
#define SERIALCONN_H

#include <boost/asio.hpp>




class SerialConn {
    public:
        SerialConn(const std::string deviceName, unsigned int baudRate=115200);
        void close();
        int read(char *buffer, int bufferSize);
        int write(char *buffer, int dataSize);
        void flush();
    private:
            boost::asio::io_service io;
            boost::asio::serial_port sport;
};


#endif