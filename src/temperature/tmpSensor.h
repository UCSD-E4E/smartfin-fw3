#ifndef TMPSENSOR_H
#define TMPSENSOR_H
#include "max31725_cpp.h"
#include <stdint.h>

class tmpSensor {
public:
    tmpSensor(MAX31725 &sensor);
    bool init();
    bool stop();
    float getTemp();

private:
    MAX31725& m_sensor;
};

#endif // TMPSENSOR_H
