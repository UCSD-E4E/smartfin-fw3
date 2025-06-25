#ifndef __MFGTEST_H__
#define __MFGTEST_H__

#include "task.hpp"

#define MFG_MIN_VALID_TEMPERATURE 15
#define MFG_MAX_VALID_TEMPERATURE   30

/**
 * @brief Handles manufacturing tests
 * @details Contains methods that test the functionality of hardware components on the manufactured board
 */
class MfgTest
{
public:
    /**
     * @brief Run manufacturing tests
     *
     */
    void run(void);

private:
    /**
     * @brief Test result enumeration
     *
     * Defines return values for all manufacturing tests
     */
    typedef enum
    {
        /**
         * @brief Return value if component passed
         */
        PASS = 0,
        /**
         * @brief Return value if component failed
         */
        FAIL = 1,
        /**
         * @brief Component not run
         *
         */
        PENDING = -1
    } MFG_TEST_RESULT_t;
    /**
     * @brief Function signature for all manufacturing tests.
     *
     * All manufacturing test functions MUST return one of either
     * MfgTest::MFG_TEST_RESULT_t::PASS or MfgTest::MFG_TEST_RESULT_t::FAIL
     *
     * @return MfgTest::MFG_TEST_RESULT_t::PASS if passed, otherwise
     * MfgTest::MFG_TEST_RESULT_t::FAIL.
     *
     */
    typedef MFG_TEST_RESULT_t (*mfg_test_fn)(void);

    typedef struct
    {
        MfgTest::mfg_test_fn fn;
        const char *name;
        MfgTest::MFG_TEST_RESULT_t pass;
    } mfg_test_entry;
    /**
     * @brief Table of manufacturing tests
     * 
     * Initialized in mfgTest.cpp with references to each manufacturing test to run
     */
    static mfg_test_entry MFG_TEST_TABLE[];

    /**
     * @brief Test the wet/dry sensor
     *
     * @return MfgTest::MFG_TEST_RESULT_t::PASS if passed, otherwise
     * MfgTest::MFG_TEST_RESULT_t::FAIL
     */
    static MFG_TEST_RESULT_t wet_dry_sensor_test(void);
    /**
     * @brief Test the temperature sensor
     *
     * @return MfgTest::MFG_TEST_RESULT_t::PASS if passed, otherwise
     * MfgTest::MFG_TEST_RESULT_t::FAIL
     */
    static MFG_TEST_RESULT_t temperature_sensor_test(void);

    /**
     * @brief Tests the IMU
     *
     * @return MfgTest::MFG_TEST_RESULT_t::PASS if passed, otherwise
     * MfgTest::MFG_TEST_RESULT_t::FAIL
     */
    static MFG_TEST_RESULT_t imu_test(void);

    /**
     * @brief Tests the cellular connection
     *
     * @return MfgTest::MFG_TEST_RESULT_t::PASS if passed, otherwise
     * MfgTest::MFG_TEST_RESULT_t::FAIL
     */
    static MFG_TEST_RESULT_t cellular_test(void);

    /**
     * @brief Tests the GPS unit
     *
     * @return MfgTest::MFG_TEST_RESULT_t::PASS if passed, otherwise
     * MfgTest::MFG_TEST_RESULT_t::FAIL
     */
    static MFG_TEST_RESULT_t gps_test(void);
};

#endif
