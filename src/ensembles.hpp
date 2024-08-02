/**
 * @file ensembles.hpp
 * @brief Header file to declare various ensembles
 */

#ifndef __ENSEMBLES_HPP__
#define __ENSEMBLES_HPP__

#if SCHEDULER_VERSION == CHARLIE_VERSION
#include "charlie_scheduler.hpp"
#else
#include "antara_scheduler.hpp"
#endif

#endif