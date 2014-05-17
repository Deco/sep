#include "Arduino.h"

#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
public:
    virtual void init();
    virtual void shutdown();

    virtual void loop(unsigned int time);

    virtual int getNodeCount();
    virtual bool obtainNodeData(
        int nodeId,
        unsigned short &sizeRef, byte *&dataPtrRef
    );
};

#endif

