#ifndef __STATES_HPP__
#define __STATES_HPP__

/**
 * @brief State Enumeration
 * 
 * If adding to this, add before STATE_N_STATES and allow for automatic values.
 * 
 * Also add the appropriate state name to states.cpp::STATES_NAME_TAB.
 */
typedef enum STATES_
{
    STATE_NULL,
    STATE_DEEP_SLEEP,
    STATE_SESSION_INIT,
    STATE_DEPLOYED,
    STATE_UPLOAD,
    STATE_CLI,
    STATE_MFG_TEST,
    STATE_TMP_CAL,
    STATE_CHARGE,
    STATE_N_STATES
}STATES_e;

extern const char* const STATES_NAME_TAB[];
#endif
