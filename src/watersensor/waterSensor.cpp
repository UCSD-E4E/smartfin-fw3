#include "waterSensor.hpp"
#include "product.hpp" // Added by PJB. Is it conventional to do this? Not sure but we need USB_PWR_DETECT_PIN
#include "cli/conio.hpp"

WaterSensor::WaterSensor(uint8_t water_detect_en_pin_to_set, uint8_t water_detect_pin_to_set, uint8_t window_size, uint8_t array_size) : water_detect_en_pin(water_detect_en_pin_to_set), water_detect_pin(water_detect_pin_to_set), moving_window_size(window_size), water_detect_array_size(array_size)
{
    water_detect_array = (uint8_t *)calloc(water_detect_array_size, sizeof(uint8_t));
}

WaterSensor::~WaterSensor()
{
    free(water_detect_array);
}

// reset the array, load it all up w/ 0s and clear the sum & location
bool WaterSensor::resetArray()
{
    // clear the buffer out
    memset(water_detect_array, (uint8_t)0, (sizeof(uint8_t) * water_detect_array_size));
    array_sum = 0;
    array_location = 0;
    samples_taken_since_reset = 0;
    return 0;
}

void WaterSensor::setWindowSize(uint8_t window_size_to_set)
{
    // swtich the window parameter and clear the sum (for resumming)
    moving_window_size = window_size_to_set;
    array_sum = 0;

    // sum all of the array items from the current location backward for the entire window,
    //  or until you get the number of samples taken after reset
    for (uint8_t i = 0; (i < moving_window_size) && (i < samples_taken_since_reset); i++)
    {
        array_sum += water_detect_array[waterDetectArrayLocation(array_location, (-1 * i))];
    }
    // if changing from a larger to smaller sample window size, change samples_taken_since_reset accordingly
    if (samples_taken_since_reset > moving_window_size)
        samples_taken_since_reset = moving_window_size;
}

uint8_t WaterSensor::takeReading()
{
    // increment array location
    array_location = (array_location + 1) % water_detect_array_size;
    // subtract last value in the window from the rolling sum
    array_sum -= water_detect_array[waterDetectArrayLocation(array_location, (-1 * moving_window_size))];

    digitalWrite(water_detect_en_pin, LOW);
    delayMicroseconds(WATER_DETECT_EN_TIME_US);
    water_detect_array[array_location] = digitalRead(water_detect_pin); // comment out if using above logic block
    digitalWrite(water_detect_en_pin, HIGH);

    // add value to the current sum
    array_sum += water_detect_array[array_location];

    // increment sample counter if needed
    if (samples_taken_since_reset < moving_window_size)
        samples_taken_since_reset++;

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

    // out of the water!
    else if ((((uint16_t)array_sum * 100) / moving_window_size) < low_detect_percentage)
    {
        last_water_detect = 0;
        return last_water_detect;
    }

    // in the water!
    else if ((((uint16_t)array_sum * 100) / moving_window_size) > high_detect_percentage)
    {
        last_water_detect = 1;
        return last_water_detect;
    }

    // nothing has changed, return current status
    else
    {
        return last_water_detect;
    }
}

void WaterSensor::update()
{
    this->takeReading();
}

uint8_t WaterSensor::getLastReading()
{
    return this->water_detect_array[this->array_location];
}

uint8_t WaterSensor::getLastStatus()
{
    return last_water_detect;
}

// gets the current in/out of water status (return true = in water, false = out)
uint8_t WaterSensor::getCurrentStatus()
{
    return last_water_detect;
}

// forces the in/out of water status.  Usefule because of hystersis
void WaterSensor::forceState(uint8_t water_detect_state)
{
    last_water_detect = water_detect_state;
}

// PJB edited next few lines (to water_detect_array) to prevent fin from thinking it's wet when powered by USB
// This chunk can be altered by adding a diode to WATER input
// gets the current reading of the sensor (not the overall status w/ hystersis)
uint8_t WaterSensor::getCurrentReading()
{
    uint8_t temp_8;

    digitalWrite(water_detect_en_pin, LOW);
    delayMicroseconds(WATER_DETECT_EN_TIME_US);
    temp_8 = digitalRead(water_detect_pin); // comment out if using above logic block
    digitalWrite(water_detect_en_pin, HIGH);

    return temp_8; // comment out if using above logic block
}

// set the low detection percentage of the moving window for hystersis
bool WaterSensor::setLowDetectPercentage(uint8_t low_percentage)
{
    if ((low_percentage >= 0) && (low_percentage <= 100))
    {
        low_detect_percentage = low_percentage;
        return true;
    }
    else
        return false;
}
// set the high detection percentage of the moving window for hystersis
bool WaterSensor::setHighDetectPercentage(uint8_t high_percentage)
{
    if ((high_percentage >= 0) && (high_percentage <= 100))
    {
        high_detect_percentage = high_percentage;
        return true;
    }
    else
        return false;
}

uint8_t WaterSensor::waterDetectArrayLocation(int16_t location, int16_t offset)
// returns a valid location (between 0 and WATER_DETECT_ARRAY_SIZE) for a location + an offset
{
    if ((location + offset) < 0)
        return (water_detect_array_size + (location + offset));
    else if ((location + offset) >= water_detect_array_size)
        return ((location + offset) - water_detect_array_size);
    else
        return (location + offset);
}
