#ifndef _SENSOREALM_H
#define _SENSOREALM_H

#include "recttree.h"

class SensorReading {
public:
  SensorReading(
    double _time
  ) : time(_time)
  {};

public:
  double time;

};

template<class ReadingType=SensorReading>
class SensorPanoramicRealm {
public:
  SensorPanoramicRealm(
    double pitchSize
  );
  ~SensorPanoramicRealm();

  void addReading(ReadingType &reading)

private:

};

#endif