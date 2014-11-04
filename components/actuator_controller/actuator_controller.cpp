
#include "actuator_controller.h"

ActuatorController::ActuatorController(
    const std::shared_ptr<ApplicationCore> &coreIn,
    const std::shared_ptr<Param> &paramsIn,
    const std::shared_ptr<ActuatorComm> &commIn
) : core(coreIn),
    params(paramsIn),
    comm(commIn)
{
    hookOnActuatorStateChange = comm->registerActuatorStateChangeCallback(
        std::bind(
            &ActuatorController::onActuatorStateChange,
            this,
            std::placeholders::_1,
            std::placeholders::_2
        )
    );
    hookOnActuatorMovementUpdate = comm->registerActuatorMovementUpdateCallback(
        std::bind(
            &ActuatorController::onActuatorMovementUpdate, 
            this, 
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4
        )
    );
}

void ActuatorController::onActuatorStateChange(
    const ActuatorComm::ActuatorInfo& info, ActuatorComm::ActuatorState state
)
{
    //add in state to print.
    std::cout << "Actuator #" << info.id << " state: " << std::endl;
}

void ActuatorController::onActuatorMovementUpdate(
    const ActuatorComm::ActuatorInfo& info, double pos, double vel, bool isMoving
)
{
    std::cout << "Actuator #" << info.id << " movement:"
        << "\n    pos: " << pos << "    vel: " << vel
        << "    " << (isMoving ? "moving" : "still")
        << std::endl
    ;
    
}
