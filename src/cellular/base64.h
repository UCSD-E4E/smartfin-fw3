// From https://github.com/Akagi201/urlsafe-b64/blob/022c4cc6c114f9a4d37c1c21aeca0c03faad0f3f/src/b64.h
/**
* @file re_base64.h  Interface to Base64 encoding/decoding functions
*
* Copyright (C) 2010 Creytiv.com
*/

#ifndef B64_H_
#define B64_H_ (1)

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
int b64_encode(const uint8_t *in, size_t ilen, char *out, size_t *olen);

int b64_decode(const char *in, size_t ilen, uint8_t *out, size_t *olen);

int urlsafe_b64_encode(const uint8_t *in, size_t ilen, char *out, size_t *olen);

int urlsafe_b64_decode(const char *in, size_t ilen, uint8_t *out, size_t *olen);
#ifdef __cplusplus
}
#endif

#endif // B64_H_