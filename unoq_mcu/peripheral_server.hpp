/**
 * @file peripheral_server.hpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Dispatches msgpack-rpc calls received over the SPI transport to
 *        real STM32U585 GPIO and I2C peripherals, and to the on-device
 *        sensor sampling task.
 * @date 2026-07-23
 *
 * Sits above sf_mcu::transport: decodes msgpack-rpc messages per
 * ipc/hal_rpc_protocol.h, executes the requested peripheral operation, and
 * (for requests, not notifications) sends back an encoded response.
 * Contains no application/business logic beyond scheduling the sensor
 * sampling task; each RPC method maps directly to a peripheral register
 * operation or to reading the sample buffer.
 */
#ifndef SF_MCU_PERIPHERAL_SERVER_HPP
#define SF_MCU_PERIPHERAL_SERVER_HPP

namespace sf_mcu
{

/**
 * @brief Initialise the peripherals the server can dispatch to.
 *
 * Must be called once, after @c transport_init(), before the first
 * @c peripheral_server_run() call. Configures GPIO and I2C peripheral
 * hardware to a known idle state and arms the sensor sampling timer.
 */
void peripheral_server_init();

/**
 * @brief Block waiting for and servicing one msgpack-rpc message.
 *
 * Reads one message from the transport link, dispatches it to the matching
 * handler by method name, and, for requests, writes back the encoded
 * response. Notifications are executed but produce no response. Intended
 * to be called in a tight loop from @c main(); sensor sampling runs on its
 * own hardware-timer schedule independent of this loop, per the
 * buffered/polled design (samples are pulled by a dedicated read method
 * rather than pushed here).
 */
void peripheral_server_run();

} // namespace sf_mcu

#endif // SF_MCU_PERIPHERAL_SERVER_HPP
