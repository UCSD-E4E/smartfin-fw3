/**
 * @file Endianness utility functions unit tests
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief
 * @version 0.1
 * @date 2024-10-31
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "util.hpp"

#include <gtest/gtest.h>

TEST(EndiannessTests, EndiannessConsistency)
{
    uint16_t test_value2 = 0xf00d;
    ASSERT_EQ(test_value2, N_TO_B_ENDIAN_2(B_TO_N_ENDIAN_2(test_value2)));

    uint32_t test_value4 = 0xbaadf00d;
    ASSERT_EQ(test_value4, N_TO_B_ENDIAN_4(B_TO_N_ENDIAN_4(test_value4)));
}

TEST(EndiannessTests, OneWayTest)
{
    uint16_t test_value2 = 0xf00d;
    ASSERT_EQ(0x0df0, N_TO_B_ENDIAN_2(test_value2));

    uint32_t test_value4 = 0xbaadf00d;
    ASSERT_EQ(0x0df0adba, N_TO_B_ENDIAN_4(test_value4));
}