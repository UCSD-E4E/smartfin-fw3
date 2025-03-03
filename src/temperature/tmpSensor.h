#ifndef TMPSENSOR_H
#define TMPSENSOR_H
#include "max31725_cpp.h"
#include <stdint.h>

/**
 * @brief The tmpSensor adapter class wraps a MAX31725 temperature sensor. 
 * It provides methods to confirm successful configuration register writing and getting sensor temperature data.
 */
class tmpSensor {
public:
    /**
     * @brief Constructs a tmpSensor adapter class object
     * @param sensor A reference to a MAX31725 sensor instance that the adapter will interface with
     */
    tmpSensor(MAX31725 &sensor);
    /**
     * @brief Initializes MAX31725 sensor and setup I2c connection
     * @return Boolean value of whether a MAX31725 sensor successfully wrote
     * to a configuration register
     * 
     * true on success, false on failure
     */
    bool init();
    /**
     * @brief Turns off MAX31725 sensor
     * @return Boolean value of whether a MAX31725 sensor successfully stopped writing
     * to a configuration register
     * 
     * true on successful stop and false otherwise
     */
    bool stop();
    /**
     * @brief Gets the current temperature reading from the MAX31725 sensor
     * @return temperature in degrees Celsius if successful in reading temperature, otherwise 0
     */
    float getTemp();
private:
    /**
     * @brief MAX31725 sensor reference
     */
    MAX31725& m_sensor;
};

#endif // TMPSENSOR_H
