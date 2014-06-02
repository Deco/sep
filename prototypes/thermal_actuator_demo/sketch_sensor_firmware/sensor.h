#include "Arduino.h"

#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
public:
    virtual void init() = 0;
    virtual void shutdown() = 0;

    virtual void loop(unsigned int time) = 0;

    virtual int getNodeCount() = 0;
    virtual bool obtainNodeData(
        int nodeId,
        unsigned short &sizeRef, byte *&dataPtrRef
    ) = 0;
};

#endif

