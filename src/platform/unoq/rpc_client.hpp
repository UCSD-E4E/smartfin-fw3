/**
 * @file rpc_client.hpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Shared msgpack-rpc client for talking to the STM32U585 peripheral
 *        server over the SPI link.
 * @date 2026-07-23
 *
 * Owns the one physical link every SF_HAL::unoq backend file needs to
 * share (gpio.cpp, i2c.cpp, system.cpp): a single transport connection, a
 * single mutex serializing all RPC traffic on the wire, and the
 * msgpack-rpc msgid counter. Callers pack their own method-specific
 * params with msgpack-c and hand this module the raw encoded bytes; this
 * module owns only the msgpack-rpc envelope
 * ([type, msgid, method, params] / [type, msgid, error, result]) and the
 * physical send/receive.
 */
#ifndef SF_UNOQ_RPC_CLIENT_HPP
#define SF_UNOQ_RPC_CLIENT_HPP

#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include <cstddef>
#include <cstdint>

namespace sf_unoq
{

/**
 * @brief Initialise the shared RPC transport.
 *
 * Must be called once before any @c rpc_call() or @c rpc_notify(). Opens
 * the physical link to the STM32U585.
 */
void rpc_client_init();

/**
 * @brief Issue a msgpack-rpc request and block for its response.
 *
 * Serialised against every other caller via the shared bus mutex: only
 * one request is ever in flight on the wire at a time.
 *
 * @param method          Null-terminated RPC method name (an
 *                         @c SF_RPC_METHOD_* constant).
 * @param params_msgpack  Pointer to a complete, already msgpack-encoded
 *                         params array (built by the caller).
 * @param params_len      Length of @p params_msgpack in bytes.
 * @param result_out      Destination buffer for the msgpack-encoded
 *                         @c result element of the response.
 * @param result_capacity Capacity of @p result_out in bytes.
 * @param result_len      Set to the number of bytes written to
 *                         @p result_out.
 * @return @c true if a well-formed response was received, its @c msgid
 *         matched the request, and its @c error slot was nil;
 *         @c false on transport failure, a malformed response, a msgid
 *         mismatch, a non-nil protocol-level @c error, or if the result
 *         does not fit in @p result_capacity.
 */
bool rpc_call(const char *method,
             const uint8_t *params_msgpack,
             std::size_t params_len,
             uint8_t *result_out,
             std::size_t result_capacity,
             std::size_t &result_len);

/**
 * @brief Issue a msgpack-rpc notification; does not wait for a response.
 *
 * @param method         Null-terminated RPC method name.
 * @param params_msgpack Pointer to a complete, already msgpack-encoded
 *                        params array.
 * @param params_len     Length of @p params_msgpack in bytes.
 */
void rpc_notify(const char *method, const uint8_t *params_msgpack, std::size_t params_len);

/**
 * @brief Acquire the shared bus mutex.
 *
 * Exposed so @c SF_HAL::i2c_lock() can let a caller hold the bus across
 * several @c i2c_read()/@c i2c_write() calls. Per @c i2c_lock()'s
 * documented contract, a caller holding this must not itself call
 * @c rpc_call() or @c rpc_notify() (both acquire this same mutex
 * internally, and it is not reentrant).
 */
void rpc_bus_lock();

/**
 * @brief Release the shared bus mutex.
 */
void rpc_bus_unlock();

} // namespace sf_unoq

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ

#endif // SF_UNOQ_RPC_CLIENT_HPP
