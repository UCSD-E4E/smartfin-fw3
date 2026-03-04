#ifndef TMPSENSOR_H
#define TMPSENSOR_H
#include "tmp117_cpp.h"
#include <stdint.h>

/**
 * @brief The tmpSensor adapter class wraps a TMP117 temperature sensor. 
 * It provides methods to confirm successful configuration register writing and getting sensor temperature data.
 */
class tmpSensor {
public:
    /**
     * @brief Constructs a tmpSensor adapter class object
     * @param sensor A reference to a TMP117 sensor instance that the adapter will interface with
     */
    tmpSensor(TMP117 &sensor);
    /**
     * @brief Initializes TMP117 sensor and setup I2c connection
     * @return Boolean value of whether a TMP117 sensor successfully wrote
     * to a configuration register
     * * true on success, false on failure
     */
    bool init();
    /**
     * @brief Determines whether the TMP117 sensor is functional
     * @return Boolean value of whether a TMP117 sensor successfully stopped writing
     * to a configuration register, determining whether it is functional
     *
     * true on sensor being functional and false otherwise
     */
    bool stop();
    /**
     * @brief Gets the current temperature reading from the TMP117 sensor
     * @return temperature in degrees Celsius, or NAN if error
     */
    float getTemp();
private:
    /**
     * @brief TMP117 sensor reference
     */
    TMP117& m_sensor;
};

#endif // TMPSENSOR_H