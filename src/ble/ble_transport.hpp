#ifndef __BLE_TRANSPORT_HPP__
#define __BLE_TRANSPORT_HPP__

/**
 * @file ble_transport.hpp
 * @brief Packet definitions and sizing for Smartfin BLE transport layer.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#include "ble_config.hpp"

#include <cstddef>
#include <cstdint>

namespace sf
{
namespace ble
{
namespace transport
{
/** @brief Maximum on-air packet size (matches BLE notify MTU planning). */
inline constexpr std::size_t MAX_PACKET_SIZE = SF_BLE_MAX_PACKET_SIZE;
/** @brief Protocol version byte placed in each header. */
inline constexpr std::uint8_t PROTOCOL_VERSION = SF_BLE_PROTOCOL_VERSION;

/**
 * @brief Packet type identifiers carried in the BLE header.
 */
enum PacketType : std::uint8_t
{
    PACKET_TYPE_TELEMETRY = 1, //!< Streaming data payload
    PACKET_TYPE_STATUS    = 2, //!< Device/ride status
};

#pragma pack(push, 1)
/**
 * @brief Fixed header prepended to every BLE packet.
 */
struct PacketHeader
{
    std::uint8_t version;    //!< Protocol version (PROTOCOL_VERSION).
    std::uint8_t type;       //!< One of PacketType.
    std::uint16_t seq;       //!< Monotonic sequence number.
    std::uint16_t payloadLen; //!< Number of payload bytes after the header.
};
#pragma pack(pop)

/** @brief Size of PacketHeader in bytes. */
constexpr std::size_t HEADER_SIZE = sizeof(PacketHeader);
/** @brief Maximum payload bytes that fit under MAX_PACKET_SIZE. */
constexpr std::size_t MAX_PAYLOAD_SIZE = MAX_PACKET_SIZE - HEADER_SIZE;

/**
 * @brief Transmit-ready packet buffer.
 */
struct TxPacket
{
    std::size_t len;                    //!< Total bytes valid in `bytes`.
    std::uint8_t bytes[MAX_PACKET_SIZE]; //!< Header + payload storage.
};

} // namespace transport
} // namespace ble
} // namespace sf

#endif // __BLE_TRANSPORT_HPP__
