#ifndef __STATES_HPP__
#define __STATES_HPP__

typedef enum STATES_
{
    STATE_NULL          = 0x0000,
    STATE_DEEP_SLEEP    = 0x0001,
    STATE_SESSION_INIT  = 0x0002,
    STATE_DEPLOYED      = 0x0003,
    STATE_UPLOAD        = 0x0004,
    STATE_CLI           = 0x0005,
    STATE_MFG_TEST      = 0x0006,
    STATE_TMP_CAL       = 0x0007,
    STATE_CHARGE        = 0x0008
}STATES_e;

#endif