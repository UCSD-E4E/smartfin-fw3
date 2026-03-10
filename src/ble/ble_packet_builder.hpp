#ifndef __BLE_PACKET_BUILDER_HPP__
#define __BLE_PACKET_BUILDER_HPP__

/**
 * @file ble_packet_builder.hpp
 * @brief Helper to accumulate ensembles into BLE-sized packets.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#include "ble_transport.hpp"

#include <cstddef>
#include <cstdint>

namespace sf
{
namespace ble
{
namespace transport
{

/**
 * @brief Assembles payload bytes and emits framed TxPacket objects.
 */
class PacketBuilder
{
public:
    /** @brief Construct an empty builder with sequence counter at 0. */
    PacketBuilder();

    /** @brief Clear accumulated payload and leave sequence untouched. */
    void reset();
    /** @brief Returns true when no payload is buffered. */
    bool empty() const;
    /** @brief Returns true when any payload is buffered. */
    bool hasData() const;

    /** @brief Current payload size in bytes. */
    std::size_t payloadSize() const;
    /** @brief Remaining payload capacity before hitting MAX_PAYLOAD_SIZE. */
    std::size_t remainingPayload() const;

    /**
     * @brief Check if a payload segment of length `len` would fit.
     * @param len Bytes to append.
     * @return true if it fits, otherwise false.
     */
    bool canAppend(std::size_t len) const;

    /**
     * @brief Append an ensemble blob to the payload buffer.
     * @param pData Pointer to data to copy.
     * @param len Number of bytes to append.
     * @return true on success, false if null or overflow.
     */
    bool appendEnsemble(const void* pData, std::size_t len);

    /**
     * @brief Build a TxPacket with header+payload and reset builder.
     * @param out Output packet buffer to fill.
     * @param type PacketType to set in header.
     * @return true if a packet was emitted, false if no payload was present.
     */
    bool finalize(TxPacket& out, PacketType type = PACKET_TYPE_TELEMETRY);

private:
    std::uint16_t nextSeq;                 //!< Next sequence number to write.
    std::size_t payloadLen;                //!< Bytes currently buffered.
    std::uint8_t payload[MAX_PAYLOAD_SIZE]; //!< Payload scratch buffer.
};

} // namespace transport
} // namespace ble
} // namespace sf

#endif // __BLE_PACKET_BUILDER_HPP__
