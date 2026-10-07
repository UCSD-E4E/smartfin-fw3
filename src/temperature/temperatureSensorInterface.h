#ifndef TEMPERATURE_SENSOR_INTERFACE_H
#define TEMPERATURE_SENSOR_INTERFACE_H

#include <stdint.h>

/**
 * @brief Abstract interface for temperature sensors
 * 
 * This interface defines the contract that all temperature sensor implementations
 * must follow, allowing different sensor types to be used interchangeably.
 */
class ITemperatureSensor {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~ITemperatureSensor() {}
    
    /**
     * @brief Initializes the temperature sensor
     * @return Boolean value of whether the sensor successfully initialized
     * 
     * true on success, false on failure
     */
    virtual bool init() = 0;
    
    /**
     * @brief Stops/disables the temperature sensor
     * @return Boolean value of whether the sensor successfully stopped
     * 
     * true on success, false on failure
     */
    virtual bool stop() = 0;
    
    /**
     * @brief Gets the current temperature reading from the sensor
     * @return temperature in degrees Celsius, or NAN if error
     */
    virtual float getTemp() = 0;
};

#endif // TEMPERATURE_SENSOR_INTERFACE_H
