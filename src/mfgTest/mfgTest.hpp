#ifndef __MFGTEST_H__
#define __MFGTEST_H__

#include "task.hpp"

#define MFG_MIN_VALID_TEMPERATURE   -10
#define MFG_MAX_VALID_TEMPERATURE   30

class MfgTest : public Task {
    public:
    /**
     * @brief 
     * 
     */
    void init(void);
    STATES_e run(void);
    void exit(void);

    private:
    typedef enum {
        PASS = 0,
        FAIL = 1,
    } MFG_TEST_RESULT_t;
    /**
     * @brief Function signature for all manufacturing tests.
     * 
     * All manufacturing test functions MUST return one of either MfgTest::PASS
     * 
     * @return MfgTest::MFG_TEST_RESULT_t::PASS if passed, otherwise MfgTest::MFG_TEST_RESULT_t::FAIL.
     * 
     */
    typedef MFG_TEST_RESULT_t (*mfg_test_fn)(void);
    static const mfg_test_fn MFG_TEST_TABLE[];
    
    static MFG_TEST_RESULT_t wet_dry_sensor_test(void);
    static MFG_TEST_RESULT_t temperature_sensor_test(void);
};

#endif
