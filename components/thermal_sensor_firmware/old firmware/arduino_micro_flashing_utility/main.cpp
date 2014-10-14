#include <assert.h>

int main(int argc, char *argv[])
{
    assert(argc == 2);

    const char * device_name = argv[1];
    unsigned long baud_rate = 1200;

    struct termios options;

    fd = open(device_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd == -1)
        throw SerialException("Could not open serial device.\n", errno);
    
    fcntl(fd, F_SETFL, 0); // blocking
    tcgetattr(fd, &options); // get the current options // TODO: restore on destruction of the object

#ifndef MAC_OS_X
    if(cfsetispeed(&options, baud_rate))
        throw SerialException("Could not set baud rate for input", errno);
    if(cfsetospeed(&options, baud_rate))
        throw SerialException("Could not set baud rate for output", errno);
#endif

    options.c_cflag |= (CS8 | CLOCAL | CREAD);
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 1;    // tenth of seconds allowed between bytes of serial data
                                // but since VMIN = 0 we will wait at most 1/10 s for data then return
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd, TCSANOW, &options);   // set the options

#ifdef MAC_OS_X
    cfmakeraw(&options); // necessary for ioctl to function; must come after setattr
    const speed_t TGTBAUD = baud_rate;
    int ret = ioctl(fd, IOSSIOSPEED, &TGTBAUD); // sets also non-standard baud rates
    if (ret)
        throw SerialException("Could not set baud rate", errno);
#endif

    tcflush(fd, TCIOFLUSH);
    if(fd != -1)
        close(fd);

    return 0;
}

