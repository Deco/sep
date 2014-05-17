
#include <iostream>

#include "Dynamixel/DynamixelComm.h"

int main()
{
    DynamixelComm dc("/dev/tty.usbserial-A9S3VTXD", 1000000);

    dc.Move(01, 512, 0x50);
    dc.Move(16, 512, 0x50);
}
