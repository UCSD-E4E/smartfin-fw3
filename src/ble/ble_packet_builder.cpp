#include "ble_packet_builder.hpp"

#include <cstring>

/**
 * @file ble_packet_builder.cpp
 * @brief Implementation of BLE packet assembly helper.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

namespace sf
{
    namespace ble
    {
        namespace transport
        {

            PacketBuilder::PacketBuilder() : nextSeq(0), payloadLen(0)
            {
                std::memset(payload, 0, sizeof(payload));
            }

            void PacketBuilder::reset()
            {
                payloadLen = 0;
                std::memset(payload, 0, sizeof(payload));
            }

            bool PacketBuilder::empty() const
            {
                return payloadLen == 0;
            }

            bool PacketBuilder::hasData() const
            {
                return payloadLen != 0;
            }

            std::size_t PacketBuilder::payloadSize() const
            {
                return payloadLen;
            }

            std::size_t PacketBuilder::remainingPayload() const
            {
                return MAX_PAYLOAD_SIZE - payloadLen;
            }

            bool PacketBuilder::canAppend(std::size_t len) const
            {
                if (len == 0)
                {
                    return false;
                }

                return len <= remainingPayload();
            }

            bool PacketBuilder::appendEnsemble(const void *pData, std::size_t len)
            {
                if (pData == nullptr || !canAppend(len))
                {
                    return false;
                }

                std::memcpy(&payload[payloadLen], pData, len);
                payloadLen += len;
                return true;
            }

            bool PacketBuilder::finalize(TxPacket &out, PacketType type)
            {
                if (!hasData())
                {
                    return false;
                }

                PacketHeader header;
                header.version = PROTOCOL_VERSION;
                header.type = static_cast<std::uint8_t>(type);
                header.seq = nextSeq++;
                header.payloadLen = static_cast<std::uint16_t>(payloadLen);

                std::memcpy(out.bytes, &header, sizeof(header));
                std::memcpy(out.bytes + sizeof(header), payload, payloadLen);
                out.len = sizeof(header) + payloadLen;

                reset();
                return true;
            }

        } // namespace transport
    } // namespace ble
} // namespace sf
