#ifndef I2C_H
#define I2C_H

#include <stdint.h>

#define I2C_NO_ERROR    (0)
#define I2C_ERROR       (-1)

class I2C {
public:
    /** Read from an I2C slave
     *
     * Performs a complete read transaction. The bottom bit of
     * the address is forced to 1 to indicate a read.
     *
     *  @param address 8-bit I2C slave address [ addr | 1 ]
     *  @param data Pointer to the byte-array to read data in to
     *  @param length Number of bytes to read
     *  @param repeated Repeated start, true - don't send stop at end
     *
     *  @returns
     *       0 on success (ack),
     *       nonzero on failure (nack)
     */
    int read(uint8_t address, char *data, int length, bool repeated);

    /** Write to an I2C slave
     *
     * Performs a complete write transaction. The bottom bit of
     * the address is forced to 0 to indicate a write.
     *
     *  @param address 8-bit I2C slave address [ addr | 0 ]
     *  @param data Pointer to the byte-array data to send
     *  @param length Number of bytes to send
     *  @param repeated Repeated start, true - do not send stop at end
     *
     *  @returns
     *       0 on success (ack),
     *       nonzero on failure (nack)
     */
    int write(uint8_t address, const char *data, int length, bool repeated);
};

#endif // I2C_H
