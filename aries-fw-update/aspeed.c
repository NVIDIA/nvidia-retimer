/*
 * Copyright 2020 Astera Labs, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may not
 * use this file except in compliance with the License. You may obtain a copy
 * of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/*
 * @file aspeed.c
 * @brief Implementation of helper functions used across SDK on the A-speed
 *          system.
 */

#include "aspeed.h"

#include <sys/ioctl.h>

int asteraI2COpenConnection(int i2cBus, int slaveAddress)
{
    int file;
    int quiet = 0;
    char filename[20];
    int size = sizeof(filename);

    snprintf(filename, size, "/dev/i2c/%d", i2cBus);
    filename[size - 1] = '\0';
    file = open(filename, O_RDWR);

    if (file < 0 && (errno == ENOENT || errno == ENOTDIR))
    {
        sprintf(filename, "/dev/i2c-%d", i2cBus);
        file = open(filename, O_RDWR);
    }

    if (file < 0 && !quiet)
    {
        if (errno == ENOENT)
        {
            fprintf(stderr,
                    "Error: Could not open file "
                    "`/dev/i2c-%d' or `/dev/i2c/%d': %s\n",
                    i2cBus, i2cBus, strerror(ENOENT));
        }
        else
        {
            fprintf(stderr,
                    "Error: Could not open file "
                    "`%s': %s\n",
                    filename, strerror(errno));
            if (errno == EACCES)
            {
                fprintf(stderr, "Run as root?\n");
            }
        }
    }
    setSlaveAddress(file, slaveAddress, 0);
    return file;
}

int asteraI2CWriteBlockData(int handle, uint8_t cmdCode, uint8_t numBytes,
                            uint8_t* buf)
{
    int r = i2c_smbus_write_i2c_block_data(handle, cmdCode, numBytes, buf);
    /* function can return positive on success, APIs don't expect that */
    if (r < 0)
        return r;
    return 0;
}

int asteraI2CReadBlockData(int handle, uint8_t cmdCode, uint8_t numBytes,
                           uint8_t* buf)
{
    return i2c_smbus_read_i2c_block_data(handle, cmdCode, numBytes, buf);
}

int asteraI2CBlock(int handle)
{
    (void)handle;
    return 0; // Equivalent to ARIES_SUCCESS
}

int asteraI2CUnblock(int handle)
{
    (void)handle;
    return 0; // Equivalent to ARIES_SUCCESS
}

/*
 * Set I2C Slave address
 */
int setSlaveAddress(int file, int address, int force)
{
    /* With force, let the user read from/write to the registers
       even when a driver is also running */
    if (ioctl(file, force ? I2C_SLAVE_FORCE : I2C_SLAVE, address) < 0)
    {
        fprintf(stderr, "Error: Could not set address to 0x%02x: %s\n", address,
                strerror(errno));
        return -errno;
    }
    return 0;
}

/*
 * Close I2C handle
 */
void closeI2CConnection(int file)
{
    close(file);
}
