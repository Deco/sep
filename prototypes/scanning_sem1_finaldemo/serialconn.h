#include <boost/asio.hpp>





class SerialConn {
    public:
        SerialConn(const std::string deviceName, unsigned int baudRate=115200);
        void close();
        int read(char *buffer, int bufferSize);
        int write(char *buffer, int dataSize);
        void flush();
    private:
        #ifdef SERIALCONN_USE_BOOST
            boost::asio::io_service io;
            boost::asio::serial_port sport;
        #elif SERIALCONN_USE_IKARUS
            bool mac = true;
            // manual serial stuff here
        #endif
};

/*
SerialConn sp();

sp.open("/dev/ttyACM0", 115200);

byte buffer[100];
int readCount = sp.read(buffer, 100);

byte buffer[2];
buffer[0] = 123;
buffer[1] = 0xFF;
sp.write(buffer, 2);
*/
