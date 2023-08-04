#ifndef __STATES_HPP__
#define __STATES_HPP__

typedef enum STATES_
{
    STATE_NULL          = 0x0000,
    STATE_CLI           = 0x0001,
    STATE_DEEP_SLEEP    = 0x0002,
    STATE_CHARGE        = 0x0003,
    STATE_UPLOAD        = 0x0004
}STATES_e;

#endif
