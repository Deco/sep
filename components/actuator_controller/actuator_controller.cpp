
#include <ActuatorController.h>

ActuatorController::ActuatorController(
    SerialPortInfo sportInfo
) : sport(sportInfo),
    servoComm(sport)
{
    
}
