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
 * @file astera_test.c
 * @brief Example application targeting basic testing of the Aries SDK.
 * This is recommended for:
 *        - Setting up Aries device and driver data structures
 *        - Reading general purpose registers
 *        - Getting the FW version
 */

#include "include/aries_api.h"
#include "aspeed.h"

#include <unistd.h>

int main(int argc, char* argv[])
{
    // -------------------------------------------------------------------------
    // SETUP
    // -------------------------------------------------------------------------
    // This portion of the example shows how to set up data structures for
    // accessing and configuring Aries Retimers
    AriesDeviceType* ariesDevice;
    AriesI2CDriverType* i2cDriver;
    AriesErrorType rc;
    int ariesHandle;
    int i2cBus;
    int ariesSlaveAddress;

    // Enable SDK-level debug prints
    asteraLogSetLevel(1); // ASTERA_DEBUG type statements (or higher)

    if (argc < 3)
    {
        printf("USAGE: %s bus slaveaddr\n", argv[0]);
        exit(-1);
    }

    i2cBus = strtol(argv[1], NULL, 0);
    ariesSlaveAddress = strtol(argv[2], NULL, 0);

    if ((i2cBus == 0) || (ariesSlaveAddress == 0))
    {
        printf("Invalid bus or slave address\n");
        exit(-2);
    }

    // Open connection to Aries Retimer
    ariesHandle = asteraI2COpenConnection(i2cBus, ariesSlaveAddress);

    // Initialize I2C Driver for SDK transactions
    i2cDriver = (AriesI2CDriverType*)malloc(sizeof(AriesI2CDriverType));
    i2cDriver->handle = ariesHandle;
    i2cDriver->slaveAddr = ariesSlaveAddress;
    i2cDriver->pecEnable = ARIES_I2C_PEC_DISABLE;
    i2cDriver->i2cFormat = ARIES_I2C_FORMAT_ASTERA;
    // Flag to indicate lock has not been initialized. Call ariesInitDevice()
    // later to initialize.
    i2cDriver->lockInit = 0;

    // Initialize Aries device structure
    ariesDevice = (AriesDeviceType*)malloc(sizeof(AriesDeviceType));
    ariesDevice->i2cDriver = i2cDriver;
    ariesDevice->i2cBus = i2cBus;
    ariesDevice->partNumber = ARIES_PTX16;

    // -------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------
    // Check Connection and Init device
    // If the connection is not good, the ariesInitDevice() API will enable ARP
    // and update the i2cDriver with the new address. It also checks for the
    // Main Micro heartbeat before reading the FW version. In case the heartbeat
    // is not up, it sets the firmware version to 0.0.0.
    rc = ariesInitDevice(ariesDevice);
    if (rc != ARIES_SUCCESS)
    {
        ASTERA_ERROR("Init device failed");
        return rc;
    }

    // Print SDK version
    ASTERA_INFO("SDK Version: %s", ariesGetSDKVersion());

    // Print FW version
    ASTERA_INFO("FW Version: %d.%d.%d", ariesDevice->fwVersion.major,
                ariesDevice->fwVersion.minor, ariesDevice->fwVersion.build);

    uint8_t dataBytes[4];
    rc = ariesReadBlockData(i2cDriver, 0, 4, dataBytes);
    if (rc != ARIES_SUCCESS)
    {
        ASTERA_ERROR("Failed to read gbl_param_reg0");
        return rc;
    }
    int glb_param_reg0 = (dataBytes[3] << 24) + (dataBytes[2] << 16) +
                         (dataBytes[1] << 8) + dataBytes[0];
    ASTERA_INFO("glb_param_reg0 = 0x%08x", glb_param_reg0);

    /*Read Temperature*/
    rc = ariesGetCurrentTemp(ariesDevice);
    CHECK_SUCCESS(rc);
    ASTERA_INFO("Current Temp: %.2f C", ariesDevice->currentTempC);

    rc = ariesGetMaxTemp(ariesDevice);
    CHECK_SUCCESS(rc);
    ASTERA_INFO("Max Temp Seen: %.2f C", ariesDevice->maxTempC);

    closeI2CConnection(ariesHandle);
    return 0;
}
