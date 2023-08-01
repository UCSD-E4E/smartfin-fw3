#ifndef TMPSENSOR_H
#define TMPSENSOR_H
#include "max31725_cpp.h"
#include <stdint.h>

class tmpSensor {
public:
    tmpSensor(MAX31725 &sensor);
    /**
     * @brief Initializes MAX31725 sensor and setup I2c connection
    */
    bool init();
    /**
     * @brief Turns of MAX31725 sensor
    */
    bool stop();
    /**
     * @brief Gets the current temperature reading from the MAX31725 sensor
    */
    float getTemp();

private:
    MAX31725& m_sensor;
};

#endif // TMPSENSOR_H
