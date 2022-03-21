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
 * @file eeprom_update.c
 * @brief Example application to update the EEPROM image through Aries.
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
    int i2cBus = 1;
    int ariesSlaveAddress = 0x20;

    // Enable SDK-level debug prints
    asteraLogSetLevel(1); // ASTERA_INFO type statements (or higher)

    if (argc < 4) {
        printf("USAGE: %s i2cbus devaddr fwfile\n", argv[0]);
        exit(-1);
    }

    i2cBus = strtol(argv[1], NULL, 0);
    ariesSlaveAddress = strtol(argv[2], NULL, 0);

    if ((i2cBus == 0)||(ariesSlaveAddress == 0)) {
        printf("i2cbus or devaddr invalid\n");
        exit(-2);
    }

    // Open connection to Aries Retimer
    ariesHandle = asteraI2COpenConnection(i2cBus, ariesSlaveAddress);

    // Initialize I2C Driver for SDK transactions
    i2cDriver = (AriesI2CDriverType*) malloc(sizeof(AriesI2CDriverType));
    i2cDriver->handle = ariesHandle;
    i2cDriver->slaveAddr = ariesSlaveAddress;
    i2cDriver->pecEnable = ARIES_I2C_PEC_DISABLE;
    i2cDriver->i2cFormat = ARIES_I2C_FORMAT_ASTERA;
    // Flag to indicate lock has not been initialized. Call ariesInitDevice()
    // later to initialize.
    i2cDriver->lockInit = 0;

    // Initialize Aries device structure
    ariesDevice = (AriesDeviceType*) malloc(sizeof(AriesDeviceType));
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

    // -------------------------------------------------------------------------
    // PROGRAMMING
    // -------------------------------------------------------------------------
    // Update the firmware image stored in EEPROM
    rc = ariesUpdateFirmware(ariesDevice, argv[3]);
    if (rc != ARIES_SUCCESS)
    {
        ASTERA_ERROR("Failed to update the firmware image. RC = %d", rc);
    }

    // Reboot device to check if FW version was applied
    // Assert HW reset
    ASTERA_INFO("Performing PCIE HW reset ...");
    rc = ariesSetPcieHwReset(ariesDevice, 1);
    // Wait 10 ms before de-asserting
    usleep(10000);
    // De-assert HW reset
    rc = ariesSetPcieHwReset(ariesDevice, 0);

    // It takes 1.8 sec for Retimer to reload firmware. Hence set
    // wait to 2 secs before reading the FW version again
    usleep(2000000);

    rc = ariesInitDevice(ariesDevice);
    if (rc != ARIES_SUCCESS)
    {
        ASTERA_ERROR("Init device failed");
        return -1;
    }

    // Print new FW version
    ASTERA_INFO("Updated FW Version is %d.%d.%d", ariesDevice->fwVersion.major,
        ariesDevice->fwVersion.minor, ariesDevice->fwVersion.build);

    // Close all open connections
    closeI2CConnection(ariesHandle);

    return ARIES_SUCCESS;
}
