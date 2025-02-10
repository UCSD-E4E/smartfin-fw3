// From https://github.com/Akagi201/urlsafe-b64/blob/022c4cc6c114f9a4d37c1c21aeca0c03faad0f3f/src/b64.h
/**
* @file re_base64.h  Interface to Base64 encoding/decoding functions
*
* Copyright (C) 2010 Creytiv.com
*/

/**
 * @defgroup Base64Codec Base64 Codec
 * @brief Functions for Base64 and URL-safe Base64 encoding and decoding.
 *
 * @{
 */

#ifndef B64_H_
#define B64_H_ (1)

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
/**
* @brief A Base-64 encode a buffer
*
* @param in   Input buffer
* @param ilen Length of input buffer
* @param out  Output buffer
* @param olen Size of output buffer, actual written on return
*
* @return 0 if success, otherwise errorcode
*/
int b64_encode(const uint8_t *in, size_t ilen, char *out, size_t *olen);
/**
* @brief  a Base-64 encoded string
*
* @param in   Input buffer
* @param ilen Length of input buffer
* @param out  Output buffer
* @param olen Size of output buffer, actual written on return
*
* @return 0 if success, otherwise errorcode
*/
int b64_decode(const char *in, size_t ilen, uint8_t *out, size_t *olen);
/**
* @brief Encode data to url-safe Base-64
* @param in   Input buffer
* @param ilen Length of input buffer
* @param out  Output buffer
* @param olen Size of output buffer, actual written on return
*
 * @return sucsess
*/
int urlsafe_b64_encode(const uint8_t *in, size_t ilen, char *out, size_t *olen);
/**
* @brief Decode data from url-safe Base-64
* @param in   Input buffer
* @param ilen Length of input buffer
* @param out  Output buffer
* @param olen Size of output buffer, actual written on return
*
 * @return sucsess
*/
int urlsafe_b64_decode(const char *in, size_t ilen, uint8_t *out, size_t *olen);

#ifdef __cplusplus
}
#endif

#endif // B64_H_
/** @} */ // end of Base64Codec