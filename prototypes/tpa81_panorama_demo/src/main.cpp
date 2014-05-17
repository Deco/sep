//#include "config.h"
#include <iostream>

#include <serial_connection.h>

int main(int argCount, char *argList[])
{
  std::cout << "[scanner] Connecting..." << std::endl;

  serial::SerialConnection sio(
    "/dev/tty.usbmodem1421", 9600,
    "/Users/Deco/workspace/sep/llllll.log"
  );
  std::cout << "[scanner] Connected!" << std::endl;

  serial::ByteArray requestData;
  requestData.push_back(255);
  requestData.push_back(delay);
  sio.SendData(requestData);

  serial::ByteArray res = sio.ReceiveData(1);
}