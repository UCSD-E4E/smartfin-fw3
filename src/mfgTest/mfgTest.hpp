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
    static int wet_dry_sensor_test(void);
    static int temperature_sensor_test(void);
    typedef int (*mfg_test_fn)(void);
    static const mfg_test_fn MFG_TEST_TABLE[];
    
};

#endif
