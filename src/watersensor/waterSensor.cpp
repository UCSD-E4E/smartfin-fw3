#include "waterSensor.hpp"

#include "cli/conio.hpp"
#include "consts.hpp"
#include "product.hpp" // Added by PJB. Is it conventional to do this? Not sure but we need USB_PWR_DETECT_PIN
#include "system.hpp"

uint8_t water_detect_array[WATER_DETECT_ARRAY_SIZE];

WaterSensor::WaterSensor(uint8_t water_detect_en_pin_to_set, uint8_t water_detect_pin_to_set)
    : water_detect_en_pin(water_detect_en_pin_to_set), water_detect_pin(water_detect_pin_to_set)

{
    if (!pSystemDesc->pNvram->get(NVRAM::WATER_DETECT_WINDOW_LEN, moving_window_size))
    {
        moving_window_size = WATER_DETECT_SURF_SESSION_INIT_WINDOW;
    }
    memset(water_detect_array, 0, WATER_DETECT_ARRAY_SIZE);
}

WaterSensor::~WaterSensor()
{
}

/**
 * @brief reset the array, load it all up w/ 0s and clear the sum & location
 * 
 *
 * @return sucsess
 */
bool WaterSensor::resetArray()
{
    // clear the buffer out
    memset(water_detect_array, (uint8_t)0, (sizeof(uint8_t) * WATER_DETECT_ARRAY_SIZE));
    array_sum = 0;
    array_location = 0;
    samples_taken_since_reset = 0;
    return 0;
}

/**
 * @brief Set window size to value
 *
 * @param window_size_to_set
 */
void WaterSensor::setWindowSize(uint8_t window_size_to_set)
{
    if (window_size_to_set > WATER_DETECT_ARRAY_SIZE)
    {
        window_size_to_set = WATER_DETECT_ARRAY_SIZE;
    }
    // swtich the window parameter and clear the sum (for resumming)
    moving_window_size = WATER_DETECT_ARRAY_SIZE;
    array_sum = 0;

    // sum all of the array items from the current location backward for the entire window,
    //  or until you get the number of samples taken after reset
    for (uint8_t i = 0; (i < moving_window_size) && (i < samples_taken_since_reset); i++)
    {
        array_sum += water_detect_array[waterDetectArrayLocation(array_location, (-1 * i))];
    }

    // if changing from a larger to smaller sample window size, change samples_taken_since_reset accordingly
    if (samples_taken_since_reset > moving_window_size)
    {
        samples_taken_since_reset = moving_window_size;
    }
}

/**
 * @brief Take a reading from the sensor
 * 
 * @return uint8_t 
 */
uint8_t WaterSensor::takeReading()
{
    // increment array location
    array_location = (array_location + 1) % WATER_DETECT_ARRAY_SIZE;
    // subtract last value in the window from the rolling sum
    array_sum -= water_detect_array[waterDetectArrayLocation(array_location, (-1 * moving_window_size))];

    water_detect_array[array_location] = getCurrentReading();

    // add value to the current sum
    array_sum += water_detect_array[array_location];

    // increment sample counter if needed
    if (samples_taken_since_reset < moving_window_size)
    {
        samples_taken_since_reset++;
    }

    /*
     * if we haven't gotten enough readings, return the hystersis enforced last value
     * 
     * we compare to high_detect_percentage of the moving window size because if
     * all of the samples measured so far are 1, this is sufficient to say we
     * are in the water.
     */
    if (samples_taken_since_reset < moving_window_size / 100. * high_detect_percentage)
    {
        return last_water_detect;
    }

    else if ((((uint16_t)array_sum * 100) / moving_window_size) < low_detect_percentage)
    {
        // out of the water!
        last_water_detect = 0;
        return last_water_detect;
    }

    else if ((((uint16_t)array_sum * 100) / moving_window_size) > high_detect_percentage)
    {
        // in the water!
        last_water_detect = 1;
        return last_water_detect;
    }

    else
    {
        // nothing has changed, return current status
        return last_water_detect;
    }
}

/**
 * @brief Takes a reading from the sensor as part of an update pulse
 * 
 */
void WaterSensor::update()
{
    this->takeReading();
}

/**
 * @brief Get last reading
 * 
 *
 * @return last reading from water array
 */
uint8_t WaterSensor::getLastReading()
{
    return water_detect_array[this->array_location];
}

/**
 * @brief Get's last water detect status
 * 
 *
 * @return last water detect 
 */
uint8_t WaterSensor::getLastStatus()
{
    return last_water_detect;
}

/**
 * @brief gets the current in/out of water status

 * 
 * @return uint8_t return true = in water, false = out
 */
uint8_t WaterSensor::getCurrentStatus()
{
    return last_water_detect;
}

/**
 * @brief forces the in/out of water status
 * Useful because of hystersis
 * @param water_detect_state 
 */
void WaterSensor::forceState(uint8_t water_detect_state)
{
    last_water_detect = water_detect_state;
}

/**
 * @brief gets the current reading of the sensor (not the overall status w/ hystersis)
 *
 * @return uint8_t current reading of the sensor
 */
uint8_t WaterSensor::getCurrentReading()
{
    uint8_t temp_8;

    digitalWrite(water_detect_en_pin, LOW);
    delayMicroseconds(WATER_DETECT_EN_TIME_US);
    temp_8 = digitalRead(water_detect_pin); // comment out if using above logic block
    digitalWrite(water_detect_en_pin, HIGH);

    return temp_8; // comment out if using above logic block
}

/**
 * @brief set the low detection percentage of the moving window for hystersis
 * 
 * @param low_percentage 
 * @return bool sucsess
 */
bool WaterSensor::setLowDetectPercentage(uint8_t low_percentage)
{
    if ((low_percentage >= 0) && (low_percentage <= 100))
    {
        low_detect_percentage = low_percentage;
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief set the high detection percentage of the moving window for hystersis
 * 
 * @param high_percentage 
 * @return bool sucsess
 */
bool WaterSensor::setHighDetectPercentage(uint8_t high_percentage)
{
    if ((high_percentage >= 0) && (high_percentage <= 100))
    {
        high_detect_percentage = high_percentage;
        return true;
    }
    else
    {
        return false;
    }
}


/**
 * @brief returns a valid location  for a location + an offset
 * value is between 0 and WATER_DETECT_ARRAY_SIZE
 *
 * @param location 
 * @param offset 
 * @return valid location
 */
uint8_t WaterSensor::waterDetectArrayLocation(int16_t location, int16_t offset)
{
    if ((location + offset) < 0)
    {
        return (WATER_DETECT_ARRAY_SIZE + (location + offset));
    }
    else if ((location + offset) >= WATER_DETECT_ARRAY_SIZE)
    { 
        return ((location + offset) - WATER_DETECT_ARRAY_SIZE);   
    } 
    else
    {
        return (location + offset);
    }
}

uint8_t WaterSensor::getWindowSize(void)
{
    return this->moving_window_size;
}