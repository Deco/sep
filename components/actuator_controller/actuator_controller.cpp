
#include "actuator_controller.h"

ActuatorController::ActuatorController(
    const std::shared_ptr<ApplicationCore> &coreIn,
    const std::shared_ptr<Param> &paramsIn,
    const std::shared_ptr<ActuatorComm> &commIn
) : core(coreIn)
  , params(paramsIn)
  , comm(commIn)
{
    hookOnActuatorStateChange = comm->registerActuatorStateChangeCallback(
        std::bind(&ActuatorController::onActuatorStateChange, this)
    );
    hookOnActuatorMovementUpdate = comm->registerActuatorMovementUpdateCallback(
        std::bind(&ActuatorController::onActuatorMovementUpdate, this)
    );
}

void ActuatorController::onActuatorStateChange(
    const ActuatorInfo& info, ActuatorState state
)
{
    std::cout << "Actuator #" << info.id << " state: " << state << std::endl;
}

void ActuatorController::onActuatorMovementUpdate(
    const ActuatorInfo& info, double pos, double vel, bool isMoving
)
{
    std::cout << "Actuator #" << info.id << " movement:"
        << "\n    pos: " << pos << "    vel: " << vel
        << "    " << (isMoving ? "moving" : "still")
        << std::endl
    ;
    
}
