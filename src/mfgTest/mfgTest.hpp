#ifndef __MFGTEST_H__
#define __MFGTEST_H__

#include "task.hpp"

#define MFG_MIN_VALID_TEMPERATURE   -10
#define MFG_MAX_VALID_TEMPERATURE   30

/**
 * @brief Handles manufacturing tests
 * @details Contains methods that test the functionality of hardware components on the manufactured board
 */
class MfgTest : public Task {
    public:
    /**
     * @brief Initialize manufacturing test task
     */
    void init(void);
    /**
     * @brief Run manufacturing tests and return to CLI
     * 
     * @return STATE_CLI
     */
    STATES_e run(void);
    void exit(void);

    private:
    /**
     * @brief Test result enumeration
     * 
     * Defines return values for all manufacturing tests
     */
    typedef enum {
        /**
         * @brief Return value if component passed
         */
        PASS = 0,
        /**
         * @brief Return value if component failed
         */
        FAIL = 1,
    } MFG_TEST_RESULT_t;
    /**
     * @brief Function signature for all manufacturing tests.
     * 
     * All manufacturing test functions MUST return one of either MfgTest::MFG_TEST_RESULT_t::PASS or MfgTest::MFG_TEST_RESULT_t::FAIL
     * 
     * @return MfgTest::MFG_TEST_RESULT_t::PASS if passed, otherwise MfgTest::MFG_TEST_RESULT_t::FAIL.
     * 
     */
    typedef MFG_TEST_RESULT_t (*mfg_test_fn)(void);
    /**
     * @brief Table of manufacturing tests
     * 
     * Initialized in mfgTest.cpp with references to each manufacturing test to run
     */
    static const mfg_test_fn MFG_TEST_TABLE[];

    /**
     * @brief Test the wet/dry sensor
     * 
     * @return MfgTest::MFG_TEST_RESULT_t::PASS if passed, otherwise MfgTest::MFG_TEST_RESULT_t::FAIL
     */
    static MFG_TEST_RESULT_t wet_dry_sensor_test(void);
    /**
     * @brief Test the temperature sensor
     * 
     * @return MfgTest::MFG_TEST_RESULT_t::PASS if passed, otherwise MfgTest::MFG_TEST_RESULT_t::FAIL
     */
    static MFG_TEST_RESULT_t temperature_sensor_test(void);
};

#endif
