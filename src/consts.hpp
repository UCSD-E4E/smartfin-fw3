#ifndef CONSTS_HPP
#define CONSTS_HPP

#define __NL__ "\n"

/**
 * @brief Unix path separator
 *
 */
#define PATH_SEP "/"

#define CLI_SCREEN_HEIGHT 24
#define CLI_SCREEN_WIDTH 80

/**
 * @brief Q14 Scalar
 *
 */
#define Q14_SCALAR 16384

/**
 * @brief Q12 Scalar
 *
 * Used to decode the Quat9 heading accuracy field from the DMP.
 */
#define Q12_SCALAR 4096

/**
 * @brief Q10 Scalar
 *
 */
#define Q10_SCALAR 1024

/**
 * @brief Q9 Scalar
 *
 * Correct scalar for accelerometer values in m/s² at ±4g FSR.
 * Max representable: ±32767/512 = ±64.0 m/s²
 */
#define Q9_SCALAR 512

/**
 * @brief Q7 Scalar
 *
 */
#define Q7_SCALAR 128

/**
 * @brief Q3 Scalar
 *
 */
#define Q3_SCALAR 8
#endif // CONSTS_HPP
