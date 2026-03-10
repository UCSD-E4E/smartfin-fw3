/**
 * @file ble_packet_builder.cpp
 * @brief Implementation of BLE packet assembly helper.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#include "ble_packet_builder.hpp"

#include <cstring>

namespace sf
{
    namespace ble
    {
        namespace transport
        {

            /**
             * @brief Construct an empty PacketBuilder with sequence set to 0.
             */
            PacketBuilder::PacketBuilder() : nextSeq(0), payloadLen(0)
            {
                std::memset(payload, 0, sizeof(payload));
            }

            /**
             * @brief Clear buffered payload and leave sequence counter untouched.
             */
            void PacketBuilder::reset()
            {
                payloadLen = 0;
                std::memset(payload, 0, sizeof(payload));
            }

            /** @brief Return true when no payload bytes are buffered. */
            bool PacketBuilder::empty() const
            {
                return payloadLen == 0;
            }

            /** @brief Return true when any payload bytes are buffered. */
            bool PacketBuilder::hasData() const
            {
                return payloadLen != 0;
            }

            /** @brief Current buffered payload length in bytes. */
            std::size_t PacketBuilder::payloadSize() const
            {
                return payloadLen;
            }

            /** @brief Remaining bytes available before reaching MAX_PAYLOAD_SIZE. */
            std::size_t PacketBuilder::remainingPayload() const
            {
                return MAX_PAYLOAD_SIZE - payloadLen;
            }

            /**
             * @brief Check if a segment of length `len` fits in remaining payload.
             * @return true if it fits and len > 0.
             */
            bool PacketBuilder::canAppend(std::size_t len) const
            {
                if (len == 0)
                {
                    return false;
                }

                return len <= remainingPayload();
            }

            /**
             * @brief Copy bytes into the payload buffer.
             * @param pData Pointer to source bytes.
             * @param len Number of bytes to append.
             * @return true on success, false on overflow/null input.
             */
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

            /**
             * @brief Finalize a TxPacket with header + payload and reset builder.
             * @param out Destination packet to fill.
             * @param type PacketType to encode in header.
             * @return true if payload existed and packet was produced.
             */
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
