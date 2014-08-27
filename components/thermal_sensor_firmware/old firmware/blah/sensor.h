#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
public:
    void init();
    void shutdown();

    void loop(unsigned int time);

    int getNodeCount();
    bool obtainNodeData(
        int nodeId,
        unsigned short &sizeRef, byte *&dataPtrRef
    );
};

#endif
