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
 * @file aries_api.c
 * @brief Implementation of public functions for the SDK.
 */

#include "include/aries_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/**< Bifurcation modes lookup*/
extern AriesBifurcationParamsType bifurcationModes[36];

/*
 * Return the SDK version
 */
const uint8_t* ariesGetSDKVersion(void)
{
    return (uint8_t *)(ARIES_SDK_VERSION);
}


/*
 * Check the status of firmware
 */
AriesErrorType ariesFWStatusCheck(
        AriesDeviceType* device)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    uint8_t dataWord[2];
    uint8_t dataBytes[4];

    // Read Code Load reg
    rc = ariesReadBlockData(device->i2cDriver, ARIES_CODE_LOAD_REG, 1,
      dataBytes);
    CHECK_SUCCESS(rc);

    if (dataBytes[0] < 0xe)
    {
        ASTERA_WARN("Code Load reg unexpected. Not all modules are loaded");
        device->codeLoadOkay = false;
    }
    else
    {
        device->codeLoadOkay = true;
    }

    // Check Main Micro heartbeat
    // If heartbeat value does not change for 100 tries, no MM heartbeat
    // Else heartbeat present even if one value changes
    uint8_t numTries = 100;
    uint8_t tryIndex = 0;
    uint8_t heartbeatVal;
    bool heartbeatSet = false;
    rc = ariesReadByteData(device->i2cDriver, ARIES_MM_HEARTBEAT_ADDR,
        dataByte);
    CHECK_SUCCESS(rc);
    heartbeatVal = dataByte[0];
    while (tryIndex < numTries)
    {
        rc = ariesReadByteData(device->i2cDriver, ARIES_MM_HEARTBEAT_ADDR,
            dataByte);
        CHECK_SUCCESS(rc);
        if (dataByte[0] != heartbeatVal)
        {
            heartbeatSet = true;
            device->mmHeartbeatOkay = true;
            break;
        }
        tryIndex++;
    }

    // Read FW version
    // If heartbeat not there, set default FW values to 0.0.0
    // and return ARIES_SUCCESS
    device->fwVersion.major = 0;
    device->fwVersion.minor = 0;
    device->fwVersion.build = 0;
    if (!heartbeatSet)
    {
        ASTERA_WARN("No Main Micro Heartbeat");
        device->mmHeartbeatOkay = false;
        return ARIES_SUCCESS;
    }

    // Get FW version (major)
    rc = ariesReadBlockDataMainMicroIndirect(device->i2cDriver,
        (ARIES_MAIN_MICRO_FW_INFO+ARIES_MM_FW_VERSION_MAJOR), 1, dataByte);
    CHECK_SUCCESS(rc);
    device->fwVersion.major  = dataByte[0];

    // Get FW version (minor)
    rc = ariesReadBlockDataMainMicroIndirect(device->i2cDriver,
        (ARIES_MAIN_MICRO_FW_INFO+ARIES_MM_FW_VERSION_MINOR), 1, dataByte);
    CHECK_SUCCESS(rc);
    device->fwVersion.minor  = dataByte[0];

    // Get FW version (build)
    rc = ariesReadBlockDataMainMicroIndirect(device->i2cDriver,
        (ARIES_MAIN_MICRO_FW_INFO+ARIES_MM_FW_VERSION_BUILD), 2, dataWord);
    CHECK_SUCCESS(rc);
    device->fwVersion.build  = (dataWord[1] << 8) + dataWord[0];

    return ARIES_SUCCESS;
}


/*
 * Initialize the i2c device
 */
AriesErrorType ariesInitDevice(
        AriesDeviceType* device)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    uint8_t dataWord[2];
    uint8_t dataBytes[4];

    rc = ariesCheckConnectionHealth(device, 0x55);
    CHECK_SUCCESS(rc);

    // Set lock = 0 (if it hasnt been set before)
    if (device->i2cDriver->lockInit == 0)
    {
        device->i2cDriver->lock = 0;
        device->i2cDriver->lockInit = 1;
    }

    // Read Code Load reg
    rc = ariesReadBlockData(device->i2cDriver, ARIES_CODE_LOAD_REG, 1,
      dataBytes);
    CHECK_SUCCESS(rc);

    if (dataBytes[0] < 0xe)
    {
        ASTERA_WARN("Code Load reg unexpected. Not all modules are loaded");
        device->codeLoadOkay = false;
    }
    else
    {
        device->codeLoadOkay = true;
    }

    // Check Main Micro heartbeat
    // If heartbeat value does not change for 100 tries, no MM heartbeat
    // Else heartbeat present even if one value changes
    uint8_t numTries = 100;
    uint8_t tryIndex = 0;
    uint8_t heartbeatVal;
    bool heartbeatSet = false;
    rc = ariesReadByteData(device->i2cDriver, ARIES_MM_HEARTBEAT_ADDR,
        dataByte);
    CHECK_SUCCESS(rc);
    heartbeatVal = dataByte[0];
    while (tryIndex < numTries)
    {
        rc = ariesReadByteData(device->i2cDriver, ARIES_MM_HEARTBEAT_ADDR,
            dataByte);
        CHECK_SUCCESS(rc);
        if (dataByte[0] != heartbeatVal)
        {
            heartbeatSet = true;
            device->mmHeartbeatOkay = true;
            break;
        }
        tryIndex++;
    }

    // Read FW version
    // If heartbeat not there, set default FW values to 0.0.0
    // and return ARIES_SUCCESS
    device->fwVersion.major = 0;
    device->fwVersion.minor = 0;
    device->fwVersion.build = 0;
    if (!heartbeatSet)
    {
        ASTERA_WARN("No Main Micro Heartbeat");
        device->mmHeartbeatOkay = false;
        return ARIES_SUCCESS;
    }

    // Get FW version (major)
    rc = ariesReadBlockDataMainMicroIndirect(device->i2cDriver,
        (ARIES_MAIN_MICRO_FW_INFO+ARIES_MM_FW_VERSION_MAJOR), 1, dataByte);
    CHECK_SUCCESS(rc);
    device->fwVersion.major  = dataByte[0];

    // Get FW version (minor)
    rc = ariesReadBlockDataMainMicroIndirect(device->i2cDriver,
        (ARIES_MAIN_MICRO_FW_INFO+ARIES_MM_FW_VERSION_MINOR), 1, dataByte);
    CHECK_SUCCESS(rc);
    device->fwVersion.minor  = dataByte[0];

    // Get FW version (build)
    rc = ariesReadBlockDataMainMicroIndirect(device->i2cDriver,
        (ARIES_MAIN_MICRO_FW_INFO+ARIES_MM_FW_VERSION_BUILD), 2, dataWord);
    CHECK_SUCCESS(rc);
    device->fwVersion.build  = (dataWord[1] << 8) + dataWord[0];

    // Capture vendor id, device id and rev number
    rc = ariesReadBlockData(device->i2cDriver, 0x4, 4, dataBytes);
    CHECK_SUCCESS(rc);
    device->vendorId = ((dataBytes[3]<<8) + dataBytes[2]);
    device->deviceId = dataBytes[1];
    device->revNumber = dataBytes[0];

    // Get link_path_struct size
    // Prior to FW 1.1.52, this size is 38
    device->linkPathStructSize = ARIES_LINK_PATH_STRUCT_SIZE;
    if ((device->fwVersion.major >= 1) && (device->fwVersion.minor >= 1)
        && (device->fwVersion.build >= 52))
    {
        rc = ariesReadBlockDataMainMicroIndirect(device->i2cDriver,
            ARIES_LINK_PATH_STRUCT_SIZE_ADDR, 1, dataByte);
        CHECK_SUCCESS(rc);
        device->linkPathStructSize = dataByte[0];
    }
    else if ((device->fwVersion.major >= 1) && (device->fwVersion.minor >= 2))
    {
        rc = ariesReadBlockDataMainMicroIndirect(device->i2cDriver,
            ARIES_LINK_PATH_STRUCT_SIZE_ADDR, 1, dataByte);
        CHECK_SUCCESS(rc);
        device->linkPathStructSize = dataByte[0];
    }

    // Get the al print info struct offset for Main Micro
    rc = ariesReadBlockDataMainMicroIndirect(device->i2cDriver,
        (ARIES_MAIN_MICRO_FW_INFO+ARIES_MM_AL_PRINT_INFO_STRUCT_ADDR), 2,
        dataWord);
    CHECK_SUCCESS(rc);
    device->mm_print_info_struct_addr = AL_MAIN_SRAM_DMEM_OFFSET +
        (dataWord[1] << 8) + dataWord[0];

    // Get the gp ctrl status struct offset for Main Micro
    rc = ariesReadBlockDataMainMicroIndirect(device->i2cDriver,
        (ARIES_MAIN_MICRO_FW_INFO+ARIES_MM_GP_CTRL_STS_STRUCT_ADDR), 2,
        dataWord);
    CHECK_SUCCESS(rc);
    device->mm_gp_ctrl_sts_struct_addr = AL_MAIN_SRAM_DMEM_OFFSET +
        (dataWord[1] << 8) + dataWord[0];

    // Get AL print info struct address for path micros
    // All Path Micros will have same address, so get for PM 4 (present on both x16 and x8 devices)
    rc = ariesReadBlockDataPathMicroIndirect(device->i2cDriver, 4,
        (ARIES_PATH_MICRO_FW_INFO_ADDRESS+ARIES_PM_AL_PRINT_INFO_STRUCT_ADDR),
        2, dataWord);
    CHECK_SUCCESS(rc);
    device->pm_print_info_struct_addr = AL_PATH_SRAM_DMEM_OFFSET +
        (dataWord[1] << 8) + dataWord[0];

    // Get GP ctrl status struct address for path micros
    // All Path Micros will have same address, so get for PM 4 (present on both x16 and x8 devices)
    rc = ariesReadBlockDataPathMicroIndirect(device->i2cDriver, 4,
        (ARIES_PATH_MICRO_FW_INFO_ADDRESS+ARIES_PM_GP_CTRL_STS_STRUCT_ADDR),
        2, dataWord);
    CHECK_SUCCESS(rc);
    device->pm_gp_ctrl_sts_struct_addr = AL_PATH_SRAM_DMEM_OFFSET +
        (dataWord[1] << 8) + dataWord[0];

    rc = ariesGetTempCalibrationCodes(device);
    CHECK_SUCCESS(rc);

    rc = ariesGetPinMap(device);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Set the bifurcation mode
 */
AriesErrorType ariesSetBifurcationMode(
        AriesDeviceType* device,
        AriesBifurcationType bifur)
{
    uint8_t glbParam[4];
    uint8_t ec;
    ec = ariesReadBlockData(device->i2cDriver, 0x0, 4, glbParam);
    if (ec != ARIES_SUCCESS)
    {
        return(ec);
    }
    else
    {
        // Bifurcation setting is in bits 12:7
        glbParam[0] = ((bifur & 0x01) << 7) | (glbParam[0] & 0x7f);
        glbParam[1] = ((bifur & 0x3e) >> 1) | (glbParam[1] & 0xe0);
        return(ariesWriteBlockData(device->i2cDriver, 0x0, 4, glbParam));
    }
}


/*
 * Get the bifurcation mode
 */
AriesErrorType ariesGetBifurcationMode(
        AriesDeviceType* device,
        AriesBifurcationType* bifur)
{
    AriesErrorType rc;
    uint8_t glbParam[4];

    rc = ariesReadBlockData(device->i2cDriver, 0x0, 4, glbParam);
    CHECK_SUCCESS(rc);
    *bifur = ((glbParam[1] & 0x1f) << 1) + ((glbParam[0] & 0x80) >> 7);

    return ARIES_SUCCESS;
}


/*
 * Set the PCIe Protocol Reset.
 */
AriesErrorType ariesSetPcieReset(
        AriesLinkType* link,
        uint8_t reset)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    if (reset == 1)
    {
        rc = ariesReadByteData(link->device->i2cDriver, 0x604, dataByte);
        CHECK_SUCCESS(rc);
        dataByte[0] &= ~(1 << link->config.linkId);
        rc = ariesWriteByteData(link->device->i2cDriver, 0x604, dataByte);
        CHECK_SUCCESS(rc);
    }
    else if (reset == 0)
    {
        rc = ariesReadByteData(link->device->i2cDriver, 0x604, dataByte);
        CHECK_SUCCESS(rc);
        dataByte[0] |= (1 << link->config.linkId);
        rc = ariesWriteByteData(link->device->i2cDriver, 0x604, dataByte);
        CHECK_SUCCESS(rc);
    }
    else {
        return ARIES_INVALID_ARGUMENT;
    }

    return ARIES_SUCCESS;
}


/*
 * Set the PCIe Hw Reset.
 */
AriesErrorType ariesSetPcieHwReset(
        AriesDeviceType* device,
        uint8_t reset)
{
    AriesErrorType rc;
    uint8_t dataWord[2];

    if (reset == 1) // Put retimer into reset
    {
        dataWord[0] = 0xff;
        dataWord[1] = 0x06;
        rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, dataWord);
        CHECK_SUCCESS(rc);
    }
    else if (reset == 0)  // Take retimer out of reset (FW will reload)
    {
        dataWord[0] = 0x0;
        dataWord[1] = 0x0;
        rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, dataWord);
        CHECK_SUCCESS(rc);
    }
    else {
        return ARIES_INVALID_ARGUMENT;
    }

    return ARIES_SUCCESS;
}


/*
 * Update the FW image in the EEPROM connected to the Retimer.
 */
AriesErrorType ariesUpdateFirmware(
        AriesDeviceType* device,
        char* filename)
{
    AriesErrorType rc;
    bool legacyMode = false;
    bool checksumVerifyFailed = false;
    uint8_t image[ARIES_EEPROM_NUM_BYTES];

    // Load ihx file
    rc = ariesLoadIhxFile(filename, image);
    if (rc != ARIES_SUCCESS)
    {
        ASTERA_INFO("Failed to load the .ihx file. RC = %d, using binary format", rc);
        rc = ariesLoadBinFile(filename, image);
        if (rc != ARIES_SUCCESS) {
            ASTERA_ERROR("Failed to load the bin file. RC = %d", rc);
        }
    }

    // Enable legacy mode if ARP is enabled or not running valid FW
    if (device->arpEnable || !device->mmHeartbeatOkay)
    {
        legacyMode = true;
    }

    // Program EEPROM image
    rc = ariesWriteEEPROMImage(device, image, legacyMode);
    if (rc != ARIES_SUCCESS)
    {
        ASTERA_ERROR("Failed to program the EEPROM. RC = %d", rc);
    }

    if (!legacyMode)
    {
        // Verify EEPROM programming by reading EEPROM and computing a checksum
        rc = ariesVerifyEEPROMImageViaChecksum(device, image);
        if (rc != ARIES_SUCCESS)
        {
            ASTERA_ERROR("Failed to verify the EEPROM using checksum. RC = %d", rc);
            checksumVerifyFailed = true;
        }
    }

    // If the EEPROM verify via checksum failed, attempt the byte by byte verify
    // Optionally, it can be manually enabled by sending a 1 as the 4th argument
    if (legacyMode || checksumVerifyFailed)
    {
        // Verify EEPROM programming by reading EEPROM and comparing data with
        // expected image. In case there is a failure, the API will attempt a
        // rewrite once
        rc = ariesVerifyEEPROMImage(device, image, legacyMode);
        if (rc != ARIES_SUCCESS)
        {
            ASTERA_ERROR("Failed to read and verify the EEPROM. RC = %d", rc);
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Load a FW image into the EEPROM connected to the Retimer.
 */
AriesErrorType ariesWriteEEPROMImage(
        AriesDeviceType* device,
        uint8_t* values,
        bool legacyMode)
{
    int currentPage = 0;
    AriesErrorType rc;

    // Do not call initDevice() with legacy mode, since the FW on the device
    // is not expected to be stable
    if (!legacyMode)
    {
        rc = ariesInitDevice(device);
        CHECK_SUCCESS(rc);
    }

    // Deassert HW and SW resets
    uint8_t tmpData[2];
    tmpData[0] = 0;
    tmpData[1] = 0;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst
    CHECK_SUCCESS(rc);
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    // If operating in legacy mode, put MM in reset
    if (legacyMode)
    {
        tmpData[0] = 0;
        tmpData[1] = 4;
        rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
        CHECK_SUCCESS(rc);
        tmpData[0] = 0;
        tmpData[1] = 6;
        /*rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst*/
        /*CHECK_SUCCESS(rc);*/
        rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
        CHECK_SUCCESS(rc);
        tmpData[0] = 0;
        tmpData[1] = 4;
        /*rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst*/
        /*CHECK_SUCCESS(rc);*/
        rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
        CHECK_SUCCESS(rc);
    }
    else
    {
        tmpData[0] = 0;
        tmpData[1] = 2;
        rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
        CHECK_SUCCESS(rc);
        tmpData[0] = 0;
        tmpData[1] = 0;
        rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
        CHECK_SUCCESS(rc);

    }

    rc = ariesI2cMasterSoftReset(device->i2cDriver);
    CHECK_SUCCESS(rc);
    usleep(2000);

    int addr = 0;
    int burst;
    int eepromWriteDelta = 0;
    int addrFlag = -1;
    int addrDiff = 0;
    int addrDiffDelta = 0;

    time_t start_t, end_t;

    uint8_t data[ARIES_MAX_BURST_SIZE];
    int addrMSB = 0;
    int addrI2C = 0;

    int eepromEnd;
    int eepromEndLoc = ariesGetEEPROMImageEnd(values);
    eepromEnd = eepromEndLoc;

    // Update EEPROM end index to match end of valid portion of EEPROM
    // If operating in legacy mode, always write entire EEPROM
    if (eepromEndLoc == -1)
    {
        eepromEnd = ARIES_EEPROM_NUM_BYTES;
    }
    else
    {
        eepromEnd += 8;
        eepromWriteDelta = eepromEnd % ARIES_EEPROM_BLOCK_WRITE_SIZE;
        if (eepromWriteDelta)
        {
            eepromEnd += ARIES_EEPROM_BLOCK_WRITE_SIZE - eepromWriteDelta;
        }
        // Calculate the location of the last 256 bytes
        addrDiff = (eepromEnd % ARIES_EEPROM_PAGE_SIZE);
        addrFlag = eepromEnd - addrDiff;
        // The last write needs to be 16 bytes, so set location accordingly
        addrDiffDelta = addrDiff % ARIES_EEPROM_BLOCK_WRITE_SIZE;
        if (addrDiffDelta)
        {
            addrDiff += ARIES_EEPROM_BLOCK_WRITE_SIZE - addrDiffDelta;
        }
    }

    // Start timer
    time(&start_t);

    // Init I2C Master
    rc = ariesI2CMasterInit(device->i2cDriver);
    CHECK_SUCCESS(rc);

    // Set Page address
    rc = ariesI2CMasterSetPage(device->i2cDriver, currentPage);
    CHECK_SUCCESS(rc);

    bool mainMicroWriteAssist = false;
    if (!legacyMode)
    {
        if ((device->fwVersion.major >= 1) && (device->fwVersion.minor >= 1))
        {
            mainMicroWriteAssist = true;
        }
        else if ((device->fwVersion.major >= 1) &&
            /*(device->fwVersion.minor >= 0) && */ (device->fwVersion.build >= 48))
        {
            mainMicroWriteAssist = true;
        }
    }

    if ((!legacyMode) && mainMicroWriteAssist)
    {
        ASTERA_INFO("Starting Main Micro assisted EEPROM write");

        while (addr < eepromEnd)
        {
            // Set MSB and local addresses for this page
            addrMSB = addr / 65536;
            addrI2C = addr % 65536;

            if (addrMSB != currentPage)
            {
                // Increment page num when you increment MSB
                rc = ariesI2CMasterSetPage(device->i2cDriver, addrMSB);
                CHECK_SUCCESS(rc);
                currentPage = addrMSB;
            }

            if (!(addrI2C % 8192))
            {
                ASTERA_INFO("Slv: 0x%02x, Reg: 0x%04x", (0x50+addrMSB), addrI2C);
            }

            // Send blocks of data (defined by burst size) to speed up process
            burst = 0;
            while (burst < ARIES_EEPROM_PAGE_SIZE)
            {
                int addrBurst = addrI2C + burst;
                int indx;
                // In last iteration, no need to write all 256 bytes
                if (addr == addrFlag)
                {
                    for (indx = 0; indx < addrDiff; indx++)
                    {
                        data[indx] = values[(addr+burst+indx)];
                    }
                    // Send a block of bytes to the EEPROM starting at address
                    // addrBurst
                    rc = ariesI2CMasterMultiBlockWrite(device->i2cDriver,
                            addrBurst, addrDiff, data);
                    CHECK_SUCCESS(rc);
                }
                else
                {
                    for (indx = 0; indx < ARIES_MAX_BURST_SIZE; indx++)
                    {
                        data[indx] = values[(addr+burst+indx)];
                    }
                    // Send a block of bytes to the EEPROM starting at address
                    // addrBurst
                    rc = ariesI2CMasterMultiBlockWrite(device->i2cDriver,
                        addrBurst, ARIES_MAX_BURST_SIZE, data);
                    CHECK_SUCCESS(rc);
                }
                usleep(ARIES_DATA_BLOCK_PROGRAM_TIME_USEC);
                burst += ARIES_MAX_BURST_SIZE;
            }
            addr += ARIES_EEPROM_PAGE_SIZE;
        }
    }
    else // Block writes not supported here. Must read one byte a a time
    {
        ASTERA_INFO("Starting legacy mode EEPROM write");

        while (addr < eepromEnd)
        {
            // Set MSB and local addresses for this page
            addrMSB = addr / 65536;
            addrI2C = addr % 65536;

            if (addrMSB != currentPage)
            {
                // Increment page num when you increment MSB
                rc = ariesI2CMasterSetPage(device->i2cDriver, addrMSB);
                CHECK_SUCCESS(rc);
                currentPage = addrMSB;
            }

            if (!(addrI2C % 8192))
            {
                ASTERA_INFO("Slv: 0x%02x, Reg: 0x%04x", (0x50+addrMSB), addrI2C);
            }

            // Send blocks of data (defined by burst size) to speed up process
            burst = 0;
            while (burst < ARIES_EEPROM_PAGE_SIZE)
            {
                int addrBurst = addrI2C + burst;
                int indx;
                // In last iteration, no need to write all 256 bytes
                if (addr == addrFlag)
                {
                    for (indx = 0; indx < addrDiff; indx++)
                    {
                        data[indx] = values[(addr+burst+indx)];
                    }
                    // Send a block of bytes to the EEPROM starting at address
                    // addrBurst
                    rc = ariesI2CMasterSendByteBlockData(device->i2cDriver,
                        addrBurst, addrDiff, data);
                    CHECK_SUCCESS(rc);
                }
                else
                {
                    for (indx = 0; indx < ARIES_MAX_BURST_SIZE; indx++)
                    {
                        data[indx] = values[(addr+burst+indx)];
                    }
                    // Send bytes to the EEPROM starting at address
                    // addrBust
                    rc = ariesI2CMasterSendByteBlockData(device->i2cDriver,
                        addrBurst, ARIES_MAX_BURST_SIZE, data);
                    CHECK_SUCCESS(rc);
                }
                usleep(ARIES_DATA_BLOCK_PROGRAM_TIME_USEC);
                burst += ARIES_MAX_BURST_SIZE;
            }
            addr += ARIES_EEPROM_PAGE_SIZE;
        }
    }
    ASTERA_INFO("Ending write");

    // Stop timer
    time(&end_t);
    ASTERA_INFO("EEPROM load time: %.2f seconds", difftime(end_t, start_t));

    // Assert HW resets for I2C master interface
    tmpData[0] = 0x00;
    tmpData[1] = 0x02;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst
    CHECK_SUCCESS(rc);
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    usleep(1000);

    return ARIES_SUCCESS;
}


/*
 * Verify the FW image in the EEPROM connected to the Retimer.
 */
AriesErrorType ariesVerifyEEPROMImage(
        AriesDeviceType* device,
        uint8_t* values,
        bool legacyMode)
{
    int currentPage;
    bool firstByte;
    int addr;
    uint8_t dataByte[1];
    uint8_t reWriteByte[1];
    uint8_t expectedByte;
    AriesErrorType rc;
    AriesErrorType matchError;

    // Not calling ariesInitDevice() here since it is already done by the write
    currentPage = 0;
    matchError = ARIES_SUCCESS;
    firstByte = true;

    // Deassert HW and SW resets
    uint8_t tmpData[2];
    tmpData[0] = 0;
    tmpData[1] = 0;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst
    CHECK_SUCCESS(rc);
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    if (legacyMode)
    {
        tmpData[0] = 0;
        tmpData[1] = 4;
        rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
        CHECK_SUCCESS(rc);

        tmpData[0] = 0;
        tmpData[1] = 6;
        /*rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst*/
        /*CHECK_SUCCESS(rc);*/
        rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
        CHECK_SUCCESS(rc);

        tmpData[0] = 0;
        tmpData[1] = 4;
        /*rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst*/
        /*CHECK_SUCCESS(rc);*/
        rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
        CHECK_SUCCESS(rc);
    }
    else
    {
        tmpData[0] = 0;
        tmpData[1] = 2;
        rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
        CHECK_SUCCESS(rc);
        tmpData[0] = 0;
        tmpData[1] = 0;
        rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
        CHECK_SUCCESS(rc);

    }

    rc = ariesI2cMasterSoftReset(device->i2cDriver);
    CHECK_SUCCESS(rc);
    usleep(2000);

    // Set page address
    rc = ariesI2CMasterSetPage(device->i2cDriver, 0);
    CHECK_SUCCESS(rc);

    // Send EEPROM address 0
    dataByte[0] = 0;
    rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 2);
    CHECK_SUCCESS(rc);
    dataByte[0] = 0;
    rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 1);
    CHECK_SUCCESS(rc);

    time_t start_t, end_t;

    uint8_t dataBytes[ARIES_EEPROM_BLOCK_WRITE_SIZE];
    int addrMSB = 0;
    int addrI2C = 0;
    int mismatchCount = 0;

    int eepromEnd;
    int eepromWriteDelta;
    int eepromEndLoc = ariesGetEEPROMImageEnd(values);
    eepromEnd = eepromEndLoc;

    // Update EEPROM end index to match end of valid portion of EEPROM
    // If operating in legacy mode, always write entire EEPROM
    if (eepromEndLoc == -1)
    {
        eepromEnd = ARIES_EEPROM_NUM_BYTES;
    }
    else
    {
        eepromEnd += 8;
        eepromWriteDelta = eepromEnd % ARIES_EEPROM_BLOCK_WRITE_SIZE;
        if (eepromWriteDelta)
        {
            eepromEnd += ARIES_EEPROM_BLOCK_WRITE_SIZE - eepromWriteDelta;
        }
    }

    // Start timer
    time(&start_t);

    bool lastByte = false;
    bool mainMicroWriteAssist = false;
    bool mainMicroSeqReadAssist = false;

    if (!legacyMode)
    {
        if ((device->fwVersion.major >= 1) && (device->fwVersion.minor >= 1))
        {
            mainMicroWriteAssist = true;
            mainMicroSeqReadAssist = true;
        }
        else if ((device->fwVersion.major >= 1) &&
            /*(device->fwVersion.minor >= 0) && */(device->fwVersion.build >= 115))
        {
            mainMicroWriteAssist = true;
            mainMicroSeqReadAssist = true;
        }
        else if ((device->fwVersion.major >= 1) &&
            /*(device->fwVersion.minor >= 0) && */(device->fwVersion.build >= 50))
        {
            mainMicroWriteAssist = true;
            mainMicroSeqReadAssist = false;
        }
    }

    if ((!legacyMode) && mainMicroWriteAssist)
    {
        bool rewriteFlag;
        ASTERA_INFO("Starting Main Micro assisted EEPROM verify");
        for (addr = 0; addr < eepromEnd; addr+=ARIES_EEPROM_BLOCK_WRITE_SIZE)
        {
            addrMSB = addr / 65536;
            addrI2C = addr % 65536;

            if (addrMSB != currentPage)
            {
                // Set updated page address
                rc = ariesI2CMasterSetPage(device->i2cDriver, addrMSB);
                CHECK_SUCCESS(rc);
                currentPage = addrMSB;
                // Send EEPROM address 0 after page update
                dataByte[0] = 0;
                rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 2);
                CHECK_SUCCESS(rc);
                dataByte[0] = 0;
                rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 1);
                CHECK_SUCCESS(rc);
                firstByte = true;
            }

            if (!(addrI2C % 8192))
            {
                ASTERA_INFO("Slv: 0x%02x, Reg: 0x%04x, Mismatch count: %d", (0x50+addrMSB), addrI2C, mismatchCount);
            }

            // Receive byte(s)
            // Address decided starting at what was set after setting
            // page address
            // Read entire page as a continuous set of 16 block bytes.
            if (lastByte)
            {
                rc = ariesI2CMasterReceiveByteBlock(device->i2cDriver,
                    dataBytes);
                CHECK_SUCCESS(rc);
                lastByte = false;
            }
            else
            {
                // Sequential mode reads are post FW version 1.0.115
                if (mainMicroSeqReadAssist)
                {
                    rc = ariesI2CMasterReceiveContinuousByteBlock(
                                device->i2cDriver, dataBytes);
                    CHECK_SUCCESS(rc);
                }
                else
                {
                    rc = ariesI2CMasterReceiveByteBlock(device->i2cDriver,
                        dataBytes);
                    CHECK_SUCCESS(rc);
                }
            }

            // Check if next block of bytes contains the last byte for this page
            if (floor((addr+ARIES_EEPROM_BLOCK_WRITE_SIZE)/65536) != currentPage)
            {
                lastByte = true;
            }

            int byteIdx;
            rewriteFlag = false;
            for (byteIdx = 0; byteIdx < ARIES_EEPROM_BLOCK_WRITE_SIZE; byteIdx++)
            {
                expectedByte = values[(addr+byteIdx)];
                if (expectedByte != dataBytes[byteIdx])
                {
                    mismatchCount += 1;
                    reWriteByte[0] = expectedByte;
                    ASTERA_ERROR("Data mismatch");
                    ASTERA_ERROR("    (Addr: %d) Expected: 0x%02x, Received: 0x%02x",
                            (addr+byteIdx), expectedByte, dataBytes[byteIdx]);
                    ASTERA_INFO("    Re-trying ...");
                    rc = ariesI2CMasterRewriteAndVerifyByte(device->i2cDriver,
                            (addr+byteIdx), reWriteByte);
                    // If re-verify step failed, mark error as verify failure
                    // and dont return error. Else, return error if not success
                    if (rc == ARIES_EEPROM_VERIFY_FAILURE)
                    {
                        matchError = ARIES_EEPROM_VERIFY_FAILURE;
                    }
                    else if (rc != ARIES_SUCCESS)
                    {
                        return rc;
                    }
                    rewriteFlag = true;
                }
            }

            // Rewrite address to start of next block, since it was reset in
            // rewrite and verify step
            if (rewriteFlag)
            {
                rc = ariesI2CMasterSendAddress(device->i2cDriver,
                        (addr+ARIES_EEPROM_BLOCK_WRITE_SIZE));
                CHECK_SUCCESS(rc);
            }
        }
    }
    else
    {
        ASTERA_INFO("Starting legacy mode EEPROM verify");
        uint8_t value[1];
        for (addr = 0; addr < eepromEnd; addr++)
        {
            addrMSB = addr / 65536;
            addrI2C = addr % 65536;

            if (addrMSB != currentPage)
            {
                // Set updated page address
                rc = ariesI2CMasterSetPage(device->i2cDriver, addrMSB);
                CHECK_SUCCESS(rc);
                currentPage = addrMSB;
                // Send EEPROM address 0 after page update
                dataByte[0] = 0;
                rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 2);
                CHECK_SUCCESS(rc);
                dataByte[0] = 0;
                rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 1);
                CHECK_SUCCESS(rc);
                firstByte = true;
            }

            if (!(addrI2C % 8192))
            {
                ASTERA_INFO("Slv: 0x%02x, Reg: 0x%04x, Mismatch count: %d", (0x50+addrMSB), addrI2C, mismatchCount);
            }

            if (firstByte == true)
            {
                // Receive byte
                // Address decided starting at what was set after setting
                // page address
                rc = ariesI2CMasterReceiveByte(device->i2cDriver, value);
                CHECK_SUCCESS(rc);
            }
            else
            {
                // Receive continuous stream of bytes to speed up process
                rc = ariesI2CMasterReceiveContinuousByte(device->i2cDriver,
                    value);
                CHECK_SUCCESS(rc);
            }

            expectedByte = values[addr];
            if (expectedByte != value[0])
            {
                mismatchCount += 1;
                reWriteByte[0] = expectedByte;
                ASTERA_ERROR("Data mismatch");
                ASTERA_ERROR("    (Addr: %d) Expected: 0x%02x, Received: 0x%02x",
                        addr, expectedByte, value[0]);
                ASTERA_INFO("    Re-trying ...");
                rc = ariesI2CMasterRewriteAndVerifyByte(device->i2cDriver, addr,
                        reWriteByte);

                // If re-verify step failed, mark error as verify failure
                // and dont return error. Else, return error if not success
                if (rc == ARIES_EEPROM_VERIFY_FAILURE)
                {
                    matchError = ARIES_EEPROM_VERIFY_FAILURE;
                }
                else if (rc != ARIES_SUCCESS)
                {
                    return rc;
                }
            }
        }
    }
    ASTERA_INFO("Ending verify");

    // Stop timer
    time(&end_t);
    ASTERA_INFO("EEPROM verify time: %.2f seconds", difftime(end_t, start_t));

    // Assert HW resets for I2C master interface
    tmpData[0] = 0x00;
    tmpData[1] = 0x02;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst
    CHECK_SUCCESS(rc);
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);
    usleep(2000);

    return matchError;
}


/*
 * Verify EEPROM via checksum
 */
AriesErrorType ariesVerifyEEPROMImageViaChecksum(
        AriesDeviceType* device,
        uint8_t* image)
{
    int currentPage;
    int addr;
    uint8_t dataByte[1];
    AriesErrorType rc;

    // Not calling ariesInitDevice() here since it is already done by the write
    // Set current page to 0
    currentPage = 0;

    ASTERA_INFO("Starting Main Micro assisted EEPROM verify via checksum");

    // Deassert HW and SW resets
    uint8_t tmpData[2];
    tmpData[0] = 0;
    tmpData[1] = 0;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst
    CHECK_SUCCESS(rc);
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    // Reset I2C Master
    tmpData[0] = 0;
    tmpData[1] = 2;
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);
    tmpData[0] = 0;
    tmpData[1] = 0;
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    // Set page address
    rc = ariesI2CMasterSetPage(device->i2cDriver, 0);
    CHECK_SUCCESS(rc);

    // Send EEPROM address 0
    dataByte[0] = 0;
    rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 2);
    CHECK_SUCCESS(rc);
    dataByte[0] = 0;
    rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 1);
    CHECK_SUCCESS(rc);

    time_t start_t, end_t;

    // Calculate EEPROM end address
    int eepromEnd;
    int eepromWriteDelta;
    int eepromEndLoc = ariesGetEEPROMImageEnd(image);
    eepromEnd = eepromEndLoc;

    // Update EEPROM end index to match end of valid portion of EEPROM
    if (eepromEndLoc == -1)
    {
        eepromEnd = ARIES_EEPROM_NUM_BYTES;
    }
    else
    {
        eepromEnd += 8;
        eepromWriteDelta = eepromEnd % ARIES_EEPROM_BLOCK_WRITE_SIZE;
        if (eepromWriteDelta)
        {
            eepromEnd += ARIES_EEPROM_BLOCK_WRITE_SIZE - eepromWriteDelta;
        }
    }

    // Start timer
    time(&start_t);

    // Calculate expected checksum values for each block
    uint8_t eepromBlockEnd = floor(eepromEnd/ARIES_EEPROM_BANK_SIZE);
    uint16_t eepromBlockEndDelta = eepromEnd - (eepromBlockEnd*ARIES_EEPROM_BANK_SIZE);

    uint32_t eepromBlockChecksum[ARIES_EEPROM_NUM_BANKS];
    uint16_t blockIdx;
    uint32_t blockSum;
    uint32_t byteIdx;
    bool isPass = true;
    for (blockIdx = 0; blockIdx < ARIES_EEPROM_NUM_BANKS; blockIdx++)
    {
        blockSum = 0;
        if (blockIdx == eepromBlockEnd)
        {
            for (byteIdx = 0; byteIdx < eepromBlockEndDelta; byteIdx++)
            {
                blockSum += image[(ARIES_EEPROM_BANK_SIZE*blockIdx) + byteIdx];
            }
        }
        else
        {
            for (byteIdx = 0; byteIdx < ARIES_EEPROM_BANK_SIZE; byteIdx++)
            {
                blockSum += image[(ARIES_EEPROM_BANK_SIZE*blockIdx) + byteIdx];
            }
        }
        eepromBlockChecksum[blockIdx] = blockSum;

    }

    uint8_t addrMSB;
    bool eepromBlockEndFlag = false;
    uint32_t checksum;

    for (addr = 0; addr < eepromEnd; addr+=ARIES_EEPROM_BANK_SIZE)
    {
        addrMSB = addr / 65536;
        eepromBlockEndFlag = false;
        if (addrMSB != currentPage)
        {
            // Set updated page address
            rc = ariesI2CMasterSetPage(device->i2cDriver, addrMSB);
            CHECK_SUCCESS(rc);
            currentPage = addrMSB;
            // Send EEPROM address 0 after page update
            dataByte[0] = 0;
            rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 2);
            CHECK_SUCCESS(rc);
            dataByte[0] = 0;
            rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 1);
            CHECK_SUCCESS(rc);

            if (currentPage == eepromBlockEnd)
            {
                eepromBlockEndFlag = true;
            }
        }

        checksum = 0;

        if (eepromBlockEndFlag)
        {
            rc = ariesI2CMasterGetChecksumPartial(device->i2cDriver,
                eepromBlockEndDelta, &checksum);
            CHECK_SUCCESS(rc);
        }
        else
        {
            rc = ariesI2CMasterGetChecksum(device->i2cDriver, &checksum);
            CHECK_SUCCESS(rc);
        }

        if (checksum != eepromBlockChecksum[currentPage])
        {
            ASTERA_ERROR("Page %d: checksum did not match expected value", currentPage);
            ASTERA_ERROR("    Expected: %d", eepromBlockChecksum[currentPage]);
            ASTERA_ERROR("    Received: %d", checksum);
            isPass = false;
        }
        else
        {
            ASTERA_INFO("Page %d: checksums matched", currentPage);
        }

        if (eepromBlockEndFlag)
        {
            ASTERA_INFO("Ending verify");
            // Stop timer
            time(&end_t);
            ASTERA_INFO("EEPROM verify time: %.2f seconds", difftime(end_t, start_t));
            if (isPass)
            {
                return ARIES_SUCCESS;
            }
            else
            {
                return ARIES_EEPROM_VERIFY_FAILURE;
            }
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Calculate block CRCs from data in EEPROM
 */
AriesErrorType ariesCheckEEPROMCrc(
        AriesDeviceType* device,
        uint8_t* image)
{
    AriesErrorType rc;

    uint8_t crcBytesEEPROM[ARIES_EEPROM_MAX_NUM_CRC_BLOCKS];
    uint8_t crcBytesImg[ARIES_EEPROM_MAX_NUM_CRC_BLOCKS];

    uint8_t numCrcBytesEEPROM;
    uint8_t numCrcBytesImg;

    rc = ariesCheckEEPROMImageCrcBytes(device, crcBytesEEPROM, &numCrcBytesEEPROM);
    CHECK_SUCCESS(rc);
    ariesGetCrcBytesImage(image, crcBytesImg, &numCrcBytesImg);

    if (numCrcBytesImg != numCrcBytesEEPROM)
    {
        // FAIL
        ASTERA_ERROR("CRC block size mismatch. Please check FW version");
        return ARIES_EEPROM_CRC_BLOCK_NUM_FAIL;
    }

    uint8_t byteIdx;
    for (byteIdx = 0; byteIdx < numCrcBytesEEPROM; byteIdx++)
    {
        if (crcBytesEEPROM[byteIdx] != crcBytesImg[byteIdx])
        {
            // Mismatch
            ASTERA_ERROR("CRC byte mismatch. Please check FW version");
            ASTERA_ERROR("    EEPROM CRC: %x, FILE CRC: %x", crcBytesEEPROM[byteIdx], crcBytesImg[byteIdx]);
            return ARIES_EEPROM_CRC_BYTE_FAIL;
        }
    }

    ASTERA_INFO("EEPROM Block CRCs match with expected FW image");

    return ARIES_SUCCESS;
}


/*
 * Calculate block CRCs from data in EEPROM
 */
AriesErrorType ariesCheckEEPROMImageCrcBytes(
        AriesDeviceType* device,
        uint8_t* crcBytes,
        uint8_t* numCrcBytes)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    // Deassert HW and SW resets
    uint8_t tmpData[2];
    tmpData[0] = 0;
    tmpData[1] = 0;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst
    CHECK_SUCCESS(rc);
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    // Init I2C Master
    rc = ariesI2CMasterInit(device->i2cDriver);
    CHECK_SUCCESS(rc);

    // Set page address
    rc = ariesI2CMasterSetPage(device->i2cDriver, 0);
    CHECK_SUCCESS(rc);

    // Send EEPROM address 0
    dataByte[0] = 0;
    rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 2);
    CHECK_SUCCESS(rc);
    dataByte[0] = 0;
    rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 1);
    CHECK_SUCCESS(rc);

    // Get block start address
    int numBlocks = 0;
    int blockStartAddr = 0;
    rc = ariesGetEEPROMFirstBlock(device->i2cDriver, &blockStartAddr);
    CHECK_SUCCESS(rc);

    uint8_t blockType;
    int blockLength;
    uint8_t crcByte;

    while (numBlocks < ARIES_EEPROM_MAX_NUM_CRC_BLOCKS)
    {
        rc = ariesGetEEPROMBlockType(device->i2cDriver, blockStartAddr,
            &blockType);
        CHECK_SUCCESS(rc);

        if (blockType != 0xff)
        {
            rc = ariesEEPROMGetBlockLength(device->i2cDriver, blockStartAddr,
                &blockLength);
            CHECK_SUCCESS(rc);
            rc = ariesGetEEPROMBlockCrcByte(device->i2cDriver, blockStartAddr,
                blockLength, &crcByte);
            CHECK_SUCCESS(rc);

            crcBytes[numBlocks] = crcByte;

            blockStartAddr += blockLength+13;
            numBlocks++;
        }
        else
        {
            // Last Page
            break;
        }
    }

    *numCrcBytes = numBlocks;

    // Assert HW resets for I2C master interface
    tmpData[0] = 0x00;
    tmpData[1] = 0x02;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst
    CHECK_SUCCESS(rc);
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);
    usleep(2000);

    return ARIES_SUCCESS;
}


/*
 * Program EEPROM, but only write bytes which are different from current
 * step
 */
AriesErrorType ariesWriteEEPROMImageDelta(
        AriesDeviceType* device,
        uint8_t* imageCurrent,
        int sizeCurrent,
        uint8_t* imageNew,
        int sizeNew)
{
    AriesErrorType rc;
    AriesEEPROMDeltaType differences[ARIES_EEPROM_NUM_BYTES];

    if (sizeCurrent != sizeNew)
    {
        ASTERA_WARN("Image sizes need to be equal");
        return ARIES_EEPROM_WRITE_ERROR;
    }


    // Iterate over array and check differences
    int addrIdx;
    int diffIdx = 0;
    for (addrIdx = 0; addrIdx < sizeNew; addrIdx++)
    {
        if (imageCurrent[addrIdx] != imageNew[addrIdx])
        {

            differences[diffIdx].address = addrIdx;
            differences[diffIdx].data = imageNew[addrIdx];
            diffIdx++;
        }
    }

    // If less than 25% of image is different, we can use this mode
    // Else recommend MM-assist mode
    if (diffIdx > ARIES_EEPROM_NUM_BYTES/4)
    {
        ASTERA_INFO("Image difference large");
        ASTERA_INFO("Please use MM-assist write mode to program EEPROM");
        return ARIES_EEPROM_WRITE_ERROR;
    }

    // De-assert HW and SW resets and reset I2C Master
    uint8_t tmpData[2];
    tmpData[0] = 0;
    tmpData[1] = 0;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst
    CHECK_SUCCESS(rc);
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    tmpData[0] = 0;
    tmpData[1] = 4;
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    tmpData[0] = 0;
    tmpData[1] = 6;
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    tmpData[0] = 0;
    tmpData[1] = 4;
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    rc = ariesI2cMasterSoftReset(device->i2cDriver);
    CHECK_SUCCESS(rc);
    usleep(2000);

    // Init I2C Master
    rc = ariesI2CMasterInit(device->i2cDriver);
    CHECK_SUCCESS(rc);

    // Set Page address to 0
    uint8_t currentPage = 0;
    rc = ariesI2CMasterSetPage(device->i2cDriver, currentPage);
    CHECK_SUCCESS(rc);

    int wrIndex;
    uint8_t dataByte[1];
    for (wrIndex = 0; wrIndex < diffIdx; wrIndex++)
    {
        // Check if page needs to be updated
        int addr = differences[diffIdx].address;
        uint8_t pageNum = floor(addr/ARIES_EEPROM_BANK_SIZE);
        if (pageNum != currentPage)
        {
            rc = ariesI2CMasterSetPage(device->i2cDriver, pageNum);
            CHECK_SUCCESS(rc);
            currentPage = pageNum;
        }

        dataByte[0] = differences[diffIdx].data;

        rc = ariesI2CMasterRewriteAndVerifyByte(device->i2cDriver, addr,
                dataByte);
        CHECK_SUCCESS(rc);
    }

    return ARIES_SUCCESS;
}


/*
 * Read a byte from EEPROM.
 */
AriesErrorType ariesReadEEPROMByte(
        AriesDeviceType* device,
        int addr,
        uint8_t* value)
{
    AriesErrorType rc;

    // Deassert HW reset for I2C master interface
    uint8_t tmpData[2];
    tmpData[0] = 0;
    tmpData[1] = 0;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst
    CHECK_SUCCESS(rc);

    // Toggle SW reset for I2C master interface
    tmpData[0] = 0;
    tmpData[1] = 2;
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);
    tmpData[0] = 0;
    tmpData[1] = 0;
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    // Set page address
    uint8_t pageNum = floor(addr/ARIES_EEPROM_BANK_SIZE);
    rc = ariesI2CMasterSetPage(device->i2cDriver, pageNum);
    CHECK_SUCCESS(rc);

    // The EEPROM access
    rc = ariesEEPROMGetRandomByte(device->i2cDriver, addr, value);
    CHECK_SUCCESS(rc);

    // Assert HW/SW resets for I2C master interface
    tmpData[0] = 0x00;
    tmpData[1] = 0x02;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst
    CHECK_SUCCESS(rc);
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Write a byte to EEPROM.
 */
AriesErrorType ariesWriteEEPROMByte(
        AriesDeviceType* device,
        int addr,
        uint8_t* value)
{
    AriesErrorType rc;

    // Deassert HW reset for I2C master interface
    uint8_t tmpData[2];
    tmpData[0] = 0;
    tmpData[1] = 0;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst
    CHECK_SUCCESS(rc);

    // Toggle SW reset for I2C master interface
    tmpData[0] = 0;
    tmpData[1] = 2;
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);
    tmpData[0] = 0;
    tmpData[1] = 0;
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    // Set page address
    uint8_t pageNum = floor(addr/ARIES_EEPROM_BANK_SIZE);
    rc = ariesI2CMasterSetPage(device->i2cDriver, pageNum);
    CHECK_SUCCESS(rc);

    // The EEPROM access
    rc = ariesI2CMasterSendByteBlockData(device->i2cDriver, addr, 1, value);
    CHECK_SUCCESS(rc);

    // Assert HW/SW resets for I2C master interface
    tmpData[0] = 0x00;
    tmpData[1] = 0x02;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, tmpData); // hw_rst
    CHECK_SUCCESS(rc);
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, tmpData); // sw_rst
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Start the self check process of SRAM
 */
AriesErrorType ariesMMSRAMCheckStart(
        AriesDeviceType* device)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    dataByte[0] = 1;
    rc = ariesWriteByteData(device->i2cDriver, ARIES_MM_SRAM_STATUS, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Get status self check process of SRAM
 */
AriesErrorType ariesMMSRAMCheckStatus(
        AriesDeviceType* device,
        AriesSramMemoryCheckType* status)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    rc = ariesReadByteData(device->i2cDriver, ARIES_MM_SRAM_STATUS, dataByte);
    CHECK_SUCCESS(rc);
    *status = dataByte[0];

    return ARIES_SUCCESS;
}


/*
 * Check connection health
 */
AriesErrorType ariesCheckConnectionHealth(
        AriesDeviceType* device,
        uint8_t slaveAddress)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    device->arpEnable = false;

    rc = ariesReadByteData(device->i2cDriver, ARIES_CODE_LOAD_REG, dataByte);
    if (rc != ARIES_SUCCESS)
    {
        // Failure to read code, run ARP
        ASTERA_WARN("Failed to read code_load, Run ARP");
        // Perform Address Resolution Protocol (ARP) to set the Aries slave
        // address. Aries slave will respond to 0x61 during ARP.
        // NOTE: In most cases, Aries firmware will disable ARP and the Retimer
        // will take on a fixed SMBus address: 0x20, 0x21, ..., 0x27.
        int arpHandle = asteraI2COpenConnection(device->i2cBus, 0x61);
        rc = ariesRunArp(arpHandle, slaveAddress); // Run ARP, user addr
        if (rc != ARIES_SUCCESS)
        {
            ASTERA_ERROR("ARP connection unsuccessful");
            return -1;
        }

        // Update Aries SMBus driver
        device->i2cDriver->handle = asteraI2COpenConnection(device->i2cBus, slaveAddress);
        device->arpEnable = true;
        rc = ariesReadByteData(device->i2cDriver, ARIES_CODE_LOAD_REG,
            dataByte);
        if (rc != ARIES_SUCCESS)
        {
            ASTERA_ERROR("Failed to read code_load after ARP");
            return -1;
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Check if device has loaded firmware properly, and if retimer slave address
 * is correct
 */
AriesErrorType ariesCheckDeviceHealth(
        AriesDeviceType* device)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    uint8_t dataBytes[4];

    device->deviceOkay = true;

    // Check if retimer slave address is correct
    // This can be done by perfroming a simple read
    rc = ariesReadBlockData(device->i2cDriver, 0x0, 4, dataBytes);
    if (rc != ARIES_SUCCESS)
    {
        ASTERA_ERROR("Reads to retimer aren't working");
        ASTERA_ERROR("Check slave address and/or connections to retimer");
        device->deviceOkay = false;
        return rc;
    }

    // Check if EEPROM has loaded successfully
    rc = ariesReadByteData(device->i2cDriver, ARIES_CODE_LOAD_REG, dataByte);
    CHECK_SUCCESS(rc);
    if (dataByte[0] < ARIES_LOAD_CODE)
    {
        ASTERA_ERROR("Device firmware load unsuccessful");
        ASTERA_ERROR("Must attempt firmware rewrite to EEPROM");
        device->deviceOkay = false;
    }

    return ARIES_SUCCESS;
}


/*
 * Check link health.
 * Check for eye height, width against thresholds
 * Check for temperature thresholds
 */
AriesErrorType ariesCheckLinkHealth(
        AriesLinkType* link)
{
    AriesErrorType rc;

    link->state.linkOkay = true;

    rc = ariesGetLinkState(link);
    CHECK_SUCCESS(rc);

    // Get current temperature and check against threshold
    rc = ariesReadPmaAvgTemp(link->device);
    CHECK_SUCCESS(rc);
    if ((link->device->currentTempC + ARIES_TEMP_CALIBRATION_OFFSET) >=
            link->device->tempAlertThreshC)
    {
        ASTERA_ERROR("Temperature alert! Current (average) temp observed is above threshold");
        ASTERA_ERROR("    Cur Temp observed (+uncertainty) = %f", (link->device->currentTempC+ARIES_TEMP_CALIBRATION_OFFSET));
        ASTERA_ERROR("    Alert threshold = %f", link->device->tempAlertThreshC);
        link->state.linkOkay = false;
    }
    else if ((link->device->currentTempC + ARIES_TEMP_CALIBRATION_OFFSET) >=
            link->device->tempWarnThreshC)
    {
        ASTERA_WARN("Temperature warn! Current (average) temp observed is above threshold");
        ASTERA_WARN("    Cur Temp observed (+uncertainty) = %f", (link->device->currentTempC+ARIES_TEMP_CALIBRATION_OFFSET));
        ASTERA_WARN("    Warn threshold = %f", link->device->tempWarnThreshC);
    }

    // Get max temp stat and check against threshold
    rc = ariesReadPmaTempMax(link->device);
    CHECK_SUCCESS(rc);
    if ((link->device->maxTempC + ARIES_TEMP_CALIBRATION_OFFSET) >=
            link->device->tempAlertThreshC)
    {
        ASTERA_ERROR("Temperature alert! All-time max temp observed is above threshold");
        ASTERA_ERROR("    Max Temp observed (+uncertainty) = %f", (link->device->maxTempC+ARIES_TEMP_CALIBRATION_OFFSET));
        ASTERA_ERROR("    Alert threshold = %f", link->device->tempAlertThreshC);
        link->state.linkOkay = false;
    }
    else if ((link->device->maxTempC + ARIES_TEMP_CALIBRATION_OFFSET) >=
            link->device->tempWarnThreshC)
    {
        ASTERA_WARN("Temperature warn! All-time max temp observed is above threshold");
        ASTERA_WARN("    Max Temp observed (+uncertainty) = %f", (link->device->maxTempC+ARIES_TEMP_CALIBRATION_OFFSET));
        ASTERA_WARN("    Warn threshold = %f", link->device->tempWarnThreshC);
    }

    // Check over-temp alert
    // Read bit 0 from 0xD to check over-temp flag
    uint8_t dataByte[1];
    rc = ariesReadByteData(link->device->i2cDriver, 0xD, dataByte);
    CHECK_SUCCESS(rc);
    // If bit 0 is 1, set temp alert as true
    if (dataByte[0] & 0x1)
    {
        link->device->overtempAlert = true;
    }
    else
    {
        link->device->overtempAlert = false;
    }

    // Get orientation - normal or reversed
    // 0 is normal, 1 is reversed
    int orientation;
    rc = ariesGetPortOrientation(link->device, &orientation);
    CHECK_SUCCESS(rc);

    int upstreamSide;
    int downstreamSide;

    if (orientation == 0)
    {
        upstreamSide = 1;
        downstreamSide = 0;
    }
    else
    {
        upstreamSide = 0;
        downstreamSide = 1;
    }

    // Check Link State
    // Set linkOkay to false if link is not in FWd state
    if (link->state.state != ARIES_STATE_FWD)
    {
        link->state.linkOkay = false;
    }

    // Check link FoM values
    int laneIndex;
    int absLane;

    int pathID;
    int lane;

    int startLane = ariesGetStartLane(link);

    uint8_t dataWord[2];
    uint8_t minFoM = 0xff;
    uint8_t thisLaneFoM;
    char* minFoMRx = "A_PER0";
    for (laneIndex = 0; laneIndex < link->state.width; laneIndex++)
    {
        absLane = startLane + laneIndex;
        pathID = floor(absLane/4) * 4;
        lane = absLane % 4;

        rc = ariesGetMinFoMVal(link->device, upstreamSide, pathID, lane,
                ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_ADAPT_FOM_ADDRESS, dataWord);
        CHECK_SUCCESS(rc);

        // FoM vaue is 7:0 of word.
        thisLaneFoM = dataWord[0];
        if (thisLaneFoM <= minFoM)
        {
            minFoM = thisLaneFoM;
            if (orientation == 0)
            {
                minFoMRx = link->device->pins[absLane].pinSet1.rxPin;
            }
            else
            {
                minFoMRx = link->device->pins[absLane].pinSet2.rxPin;
            }
        }

        rc = ariesGetMinFoMVal(link->device, downstreamSide, pathID, lane,
                ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_ADAPT_FOM_ADDRESS, dataWord);
        CHECK_SUCCESS(rc);

        // FoM vaue is 7:0 of word.
        thisLaneFoM = dataWord[0];
        if (thisLaneFoM <= minFoM)
        {
            minFoM = thisLaneFoM;
            if (orientation == 0)
            {
                minFoMRx = link->device->pins[absLane].pinSet2.rxPin;
            }
            else
            {
                minFoMRx = link->device->pins[absLane].pinSet1.rxPin;
            }
        }
    }

    // For Gen-2 and below, FoM values do not make sense. Hence we set it to
    // default values
    if (link->state.rate >= 3)
    {
        link->state.linkMinFoM = minFoM;
        link->state.linkMinFoMRx = minFoMRx;
    }
    else
    {
        link->state.linkMinFoM = 0xff;
        link->state.linkMinFoMRx = " ";
    }

    // Trigger alerts for Gen 3 and above only
    if ((link->state.linkMinFoM <= link->device->minLinkFoMAlert)
            && (link->state.rate >= 3))
    {
        link->state.linkOkay = false;
        ASTERA_ERROR("Lane FoM alert! %s FoM below threshold (Val: 0x%02x)",
                link->state.linkMinFoMRx, link->state.linkMinFoM);
    }

    // Capture the recovery count
    int recoveryCount = 0;
    rc = ariesGetLinkRecoveryCount(link, &recoveryCount);
    CHECK_SUCCESS(rc);

    // Take a median of 7 readings
    uint16_t DPLLfreqs[ARIES_NUM_DPLL_FREQ_READINGS];
    uint16_t DPLLVal;
    uint16_t DPLLFreq;
    uint8_t dpll_i;
    uint8_t try_i;

    // Initialize mins and maxs
    link->state.usppState.minDPLLCode = 0xffff;
    link->state.usppState.maxDPLLCode = 0x0;
    link->state.dsppState.minDPLLCode = 0xffff;
    link->state.dsppState.maxDPLLCode = 0x0;

    for (laneIndex = 0; laneIndex < link->state.width; laneIndex++)
    {
        int absLane = startLane + laneIndex;

        try_i = 0;
        while (try_i < ARIES_NUM_DPLL_FREQ_READING_TRIES)
        {
            for (dpll_i = 0; dpll_i < ARIES_NUM_DPLL_FREQ_READINGS; dpll_i++)
            {
                rc = ariesGetDPLLFreq(link, upstreamSide, absLane, &DPLLVal);
                CHECK_SUCCESS(rc);
                DPLLfreqs[dpll_i] = DPLLVal;
            }

            DPLLFreq = ariesGetMedian(DPLLfreqs, ARIES_NUM_DPLL_FREQ_READINGS);

            if ((DPLLFreq >= 4098) && (DPLLFreq <= 12288))
            {
                break;
            }
            try_i++;
        }

        link->state.usppState.rxState[laneIndex].DPLLCode = DPLLFreq;
        if (DPLLFreq < link->state.usppState.minDPLLCode)
        {
            link->state.usppState.minDPLLCode = DPLLFreq;
        }
        if (DPLLFreq > link->state.usppState.maxDPLLCode)
        {
            link->state.usppState.maxDPLLCode = DPLLFreq;
        }

        if (link->device->minDPLLFreqAlert > DPLLFreq)
        {
            ASTERA_WARN("DPLL Frequency low [Side: %d, Lane: %d, Freq: %d]",
                    upstreamSide, laneIndex, DPLLFreq);
        }
        else if (link->device->maxDPLLFreqAlert < DPLLFreq)
        {
            ASTERA_WARN("DPLL Frequency high [Side: %d, Lane: %d, Freq: %d]",
                    upstreamSide, laneIndex, DPLLFreq);
        }

        try_i = 0;
        while (try_i < ARIES_NUM_DPLL_FREQ_READING_TRIES)
        {
            for (dpll_i = 0; dpll_i < ARIES_NUM_DPLL_FREQ_READINGS; dpll_i++)
            {
                rc = ariesGetDPLLFreq(link, downstreamSide, absLane, &DPLLVal);
                CHECK_SUCCESS(rc);
                DPLLfreqs[dpll_i] = DPLLVal;
            }

            DPLLFreq = ariesGetMedian(DPLLfreqs, ARIES_NUM_DPLL_FREQ_READINGS);

            if ((DPLLFreq >= 4098) && (DPLLFreq <= 12288))
            {
                break;
            }
            try_i++;
        }
        link->state.dsppState.rxState[laneIndex].DPLLCode = DPLLFreq;
        if (DPLLFreq < link->state.dsppState.minDPLLCode)
        {
            link->state.dsppState.minDPLLCode = DPLLFreq;
        }
        if (DPLLFreq > link->state.dsppState.maxDPLLCode)
        {
            link->state.dsppState.maxDPLLCode = DPLLFreq;
        }

        if (link->device->minDPLLFreqAlert > DPLLFreq)
        {
            ASTERA_WARN("DPLL Frequency low [Side: %d, Lane: %d, Freq: %d]",
                    downstreamSide, laneIndex, DPLLFreq);
        }
        else if (link->device->maxDPLLFreqAlert < DPLLFreq)
        {
            ASTERA_WARN("DPLL Frequency high [Side: %d, Lane: %d, Freq: %d]",
                    downstreamSide, laneIndex, DPLLFreq);
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Get the Link recovery counter value.
 */
AriesErrorType ariesGetLinkRecoveryCount(
        AriesLinkType* link,
        int* recoveryCount)
{
    AriesErrorType rc;
    int baseAddress;
    uint8_t byteVal[1];
    int address;

    // Initialize Main Micro logger
    baseAddress = link->device->mm_print_info_struct_addr;

    // Set One Batch Mode
    address = baseAddress + ARIES_PRINT_INFO_STRUCT_LNK_RECOV_ENTRIES_PTR_OFFSET + link->config.linkId;
    rc = ariesReadByteDataMainMicroIndirect(link->device->i2cDriver, address, byteVal);
    CHECK_SUCCESS(rc);

    // Update the value in the struct
    link->state.recoveryCount = byteVal[0];

    *recoveryCount = byteVal[0];
    return ARIES_SUCCESS;
}


/*
 * Clear the Link recovery counter value.
 */
AriesErrorType ariesClearLinkRecoveryCount(
        AriesLinkType* link)
{
    AriesErrorType rc;
    int baseAddress;
    uint8_t dataByte[1];
    int address;

    // Initialize Main Micro logger
    baseAddress = link->device->mm_print_info_struct_addr;

    // Set One Batch Mode
    dataByte[0] = 0;
    address = baseAddress + ARIES_PRINT_INFO_STRUCT_LNK_RECOV_ENTRIES_PTR_OFFSET + link->config.linkId;
    rc = ariesWriteByteDataMainMicroIndirect(link->device->i2cDriver, address, dataByte);
    CHECK_SUCCESS(rc);

    // Update the value in the struct
    link->state.recoveryCount = 0;

    return ARIES_SUCCESS;
}


/*
 * Get the maximum junction temperature.
 */
AriesErrorType ariesGetMaxTemp(
        AriesDeviceType* device)
{
    return ariesReadPmaTempMax(device);
}


/*
 * Get the current average temperature across all sensors
 */
AriesErrorType ariesGetCurrentTemp(
        AriesDeviceType* device)
{
    return ariesReadPmaAvgTemp(device);
}


/*
 * Get the current Link state.
 */
AriesErrorType ariesGetLinkState(
        AriesLinkType* link)
{
    int linkNum;
    AriesErrorType rc;
    AriesBifurcationType bifMode;
    int baseAddress;
    uint8_t byteVal[1];
    int linkIdx;
    int addressOffset;

    // Get current bifurcation settings
    rc = ariesGetBifurcationMode(link->device, &bifMode);
    CHECK_SUCCESS(rc);

    int startLane = ariesGetStartLane(link);

    // Get link number in bifurcation mode
    // Iterate over links in bifurcation lookup table and find start lane
    // Lane number is part of bifurcation link properties
    bool laneFound = false;
    for (linkIdx = 0; linkIdx < bifurcationModes[bifMode].numLinks; linkIdx++)
    {
        if (startLane == (bifurcationModes[bifMode].links[linkIdx].startLane))
        {
            linkNum = bifurcationModes[bifMode].links[linkIdx].linkId;
            laneFound = true;
            break;
        }
    }

    if (laneFound == false)
    {
        return ARIES_LINK_CONFIG_INVALID;
    }

    // Get link path struct offsets
    // The link path struct sits on top of the MM link struct
    // Hence need to compute this offset first, before getting link struct
    // parameters
    addressOffset = ARIES_MAIN_MICRO_FW_INFO +
        ARIES_MM_LINK_STRUCT_ADDR_OFFSET +
        (linkNum*ARIES_LINK_ADDR_EL_SIZE);

    uint8_t dataWord[2];
    rc = ariesReadBlockDataMainMicroIndirect(link->device->i2cDriver,
            addressOffset, 2, dataWord);
    CHECK_SUCCESS(rc);
    int linkStructAddr = dataWord[0] + (dataWord[1] << 8);

    // Compute offset, at which link struct members are available
    baseAddress = AL_MAIN_SRAM_DMEM_OFFSET + linkStructAddr +
        (link->device->linkPathStructSize*2);

    // Read link width
    // Detected link width is offset at 46 from base address
    /*addressOffset = baseAddress + ARIES_LINK_STRUCT_DETECTED_WIDTH_OFFSET;*/
    addressOffset = baseAddress + ARIES_LINK_STRUCT_WIDTH_OFFSET;
    rc = ariesReadByteDataMainMicroIndirect(link->device->i2cDriver,
        addressOffset, byteVal);
    CHECK_SUCCESS(rc);
    link->state.width = byteVal[0];

    // Read current state
    // Current state is offset at 10 from base address
    addressOffset = baseAddress + ARIES_LINK_STRUCT_STATE_OFFSET;
    rc = ariesReadByteDataMainMicroIndirect(link->device->i2cDriver,
            addressOffset, byteVal);
    CHECK_SUCCESS(rc);
    link->state.state = byteVal[0];

    // Read current rate
    // Current rate is offset at 6 from base address
    // Rate value is offset by 1, i.e. 0: Gen1, 1: Gen2, ... 4: Gen5
    // Hence update rate value by 1
    addressOffset = baseAddress + ARIES_LINK_STRUCT_RATE_OFFSET;
    rc = ariesReadByteDataMainMicroIndirect(link->device->i2cDriver,
            addressOffset, byteVal);
    CHECK_SUCCESS(rc);
    link->state.rate = byteVal[0] + 1;

    return ARIES_SUCCESS;
}


/*
 * Get detailed link state
 */
AriesErrorType ariesGetLinkStateDetailed(
        AriesLinkType* link)
{
    AriesErrorType rc;
    float usppSpeed = 2.5;
    float dsppSpeed = 2.5;
    uint8_t dataWord[2];

    // Update link state parameters
    //rc = ariesGetLinkState(link);
    rc = ariesCheckLinkHealth(link); // Refresh Link and temp sensor readings
    CHECK_SUCCESS(rc);

    int width = link->state.width;
    int startLane = ariesGetStartLane(link);
    int laneIndex;
    int upstreamSide;
    int downstreamSide;
    int upstreamDirection;
    int downstreamDirection;
    int upstreamPinSet;
    int downstreamPinSet;
    int usppTxDirection;
    int dsppTxDirection;
    int usppRxDirection;
    int dsppRxDirection;

    // Get orientation - normal or reversed
    // 0 is normal, 1 is reversed
    int orientation;
    rc = ariesGetPortOrientation(link->device, &orientation);
    CHECK_SUCCESS(rc);

    if (orientation == 0)  // Normal Orientation
    {
        upstreamSide = 1;
        downstreamSide = 0;
        upstreamDirection = 1;
        downstreamDirection = 0;
        upstreamPinSet = 0;
        downstreamPinSet = 1;
        usppRxDirection = 0;
        usppTxDirection = 1;
        dsppRxDirection = 1;
        dsppTxDirection = 0;
    }
    else // Reversed orientation
    {
        upstreamSide = 0;
        downstreamSide = 1;
        upstreamDirection = 0;
        downstreamDirection = 1;
        upstreamPinSet = 1;
        downstreamPinSet = 0;
        usppRxDirection = 1;
        usppTxDirection = 0;
        dsppRxDirection = 0;
        dsppTxDirection = 1;
    }

    // Get Current Speeds
    // Upstream path speed
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int absLane = startLane + laneIndex;
        int rxTerm;
        rc = ariesGetLinkRxTerm(link, upstreamSide, absLane, &rxTerm);
        CHECK_SUCCESS(rc);
        if (rxTerm == 1)
        {
            rc = ariesGetLinkCurrentSpeed(link, absLane, upstreamDirection,
                    &usppSpeed);
            CHECK_SUCCESS(rc);
            break;
        }
    }

    // Downstream path speed
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int absLane = startLane + laneIndex;
        int rxTerm;
        rc = ariesGetLinkRxTerm(link, downstreamSide, absLane, &rxTerm);
        CHECK_SUCCESS(rc);
        if (rxTerm == 1)
        {
            rc = ariesGetLinkCurrentSpeed(link, absLane, downstreamDirection,
                    &dsppSpeed);
            CHECK_SUCCESS(rc);
            break;
        }
    }

    link->state.usppSpeed = usppSpeed;
    link->state.dsppSpeed = dsppSpeed;

    // Get Logical Lane for both upstream and downstream
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int laneNum;
        int absLane = startLane + laneIndex;

        // Upstream direction (RX and TX)
        rc = ariesGetLogicalLaneNum(link, absLane, usppRxDirection, &laneNum);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].logicalLaneNum = laneNum;
        rc = ariesGetLogicalLaneNum(link, absLane, usppTxDirection, &laneNum);
        CHECK_SUCCESS(rc);
        link->state.usppState.txState[laneIndex].logicalLaneNum = laneNum;

        // Downstream direction (RX and TX)
        rc = ariesGetLogicalLaneNum(link, absLane, dsppRxDirection, &laneNum);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].logicalLaneNum = laneNum;
        rc = ariesGetLogicalLaneNum(link, absLane, dsppTxDirection, &laneNum);
        CHECK_SUCCESS(rc);
        link->state.dsppState.txState[laneIndex].logicalLaneNum = laneNum;
    }

    //////////////////////// USPP & DSPP Parameters ////////////////////////

    // Physical Pin Info
    // This is from a fixed array defined in aries_globals.c
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int absLane = startLane + laneIndex;

        if (orientation == 0)
        {
            link->state.usppState.rxState[laneIndex].physicalPinName =
                link->device->pins[absLane].pinSet1.rxPin;
            link->state.usppState.txState[laneIndex].physicalPinName =
                link->device->pins[absLane].pinSet1.txPin;
            link->state.dsppState.rxState[laneIndex].physicalPinName =
                link->device->pins[absLane].pinSet2.rxPin;
            link->state.dsppState.txState[laneIndex].physicalPinName =
                link->device->pins[absLane].pinSet2.txPin;
        }
        else
        {
            link->state.dsppState.rxState[laneIndex].physicalPinName =
                link->device->pins[absLane].pinSet1.rxPin;
            link->state.dsppState.txState[laneIndex].physicalPinName =
                link->device->pins[absLane].pinSet1.txPin;
            link->state.usppState.rxState[laneIndex].physicalPinName =
                link->device->pins[absLane].pinSet2.rxPin;
            link->state.usppState.txState[laneIndex].physicalPinName =
                link->device->pins[absLane].pinSet2.txPin;
        }
    }


    // Current TX Pre-Cursor value (valid for Gen-3 and above)
    // else 0
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        if ((usppSpeed == 2.5) || (usppSpeed == 5))
        {
            link->state.usppState.txState[laneIndex].pre = 0;
            link->state.dsppState.txState[laneIndex].pre = 0;
        }
        else
        {
            // Physical Path N drives the TX de-emphasis values for Path 1-N
            // Hence upstream stats will be in downstream path
            int txPre;
            int absLane = startLane + laneIndex;
            // Upstream direction
            rc = ariesGetTxPre(link, absLane, downstreamDirection, &txPre);
            CHECK_SUCCESS(rc);
            link->state.usppState.txState[laneIndex].pre = txPre;

            // Downstream parameters
            rc = ariesGetTxPre(link, absLane, upstreamDirection, &txPre);
            CHECK_SUCCESS(rc);
            link->state.dsppState.txState[laneIndex].pre = txPre;
        }
    }

    // Get TX Current Cursor value (valid for Gen-3 and above)
    // else 0
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        if ((usppSpeed == 2.5) || (usppSpeed == 5))
        {
            link->state.usppState.txState[laneIndex].cur = 0;
            link->state.dsppState.txState[laneIndex].cur = 0;
        }
        else
        {
            // Physical Path N drives the TX de-emphasis values for Path 1-N
            // Hence upstream stats will be in downstream path
            int txCur;
            int absLane = startLane + laneIndex;
            // Upstream parameters
            rc = ariesGetTxCur(link, absLane, downstreamDirection, &txCur);
            CHECK_SUCCESS(rc);
            link->state.usppState.txState[laneIndex].cur = txCur;

            // Downstream parameters
            rc = ariesGetTxCur(link, absLane, upstreamDirection, &txCur);
            CHECK_SUCCESS(rc);
            link->state.dsppState.txState[laneIndex].cur = txCur;
        }
    }

    // Get RX Polarity
    // Get RX pseudoport param - opposite paths
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int rxPolarity;
        int absLane = startLane + laneIndex;
        // Upstream parameters
        rc = ariesGetRxPolarityCode(link, absLane, usppRxDirection,
                upstreamPinSet, &rxPolarity);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].polarity = rxPolarity;

        // Downstream parameters
        rc = ariesGetRxPolarityCode(link, absLane, dsppRxDirection,
                downstreamPinSet, &rxPolarity);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].polarity = rxPolarity;
    }

    // Get TX Post Cursor Value (valid for gen-3 and above)
    // else get post val from tx-pre val
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        // Physical Path N drives the TX de-emphasis values for Path 1-N
        // Hence upstream stats will be in downstream path
        if ((usppSpeed == 2.5) || (usppSpeed == 5))
        {
            // For gen 1 and 2, pre-cursor mode has de-emphasis value
            // Compute pst from this value
            int txPre;
            float txPst;
            int absLane = startLane + laneIndex;

            // Upstream parameters (USPP)
            rc = ariesGetTxPre(link, absLane, downstreamDirection, &txPre);
            CHECK_SUCCESS(rc);
            if (txPre == 0)
            {
                txPst = -6;
            }
            else if (txPre == 1)
            {
                txPst = -3.5;
            }
            else
            {
                txPst = 0;
            }
            link->state.usppState.txState[laneIndex].pst = txPst;
            // De-emphasis value is in tx pre cursor val
            link->state.usppState.txState[laneIndex].de = txPre;

            // Downstream parameters (DSPP)
            rc = ariesGetTxPre(link, absLane, upstreamDirection, &txPre);
            CHECK_SUCCESS(rc);
            if (txPre == 0)
            {
                txPst = -6;
            }
            else if (txPre == 1)
            {
                txPst = -3.5;
            }
            else
            {
                txPst = 0;
            }
            link->state.dsppState.txState[laneIndex].pst = txPst;
            // De-emphasis value is in tx pre cursor val
            link->state.dsppState.txState[laneIndex].de = txPre;
        }
        else
        {
            int txPst;
            int absLane = startLane + laneIndex;
            // Upstream parameters
            rc = ariesGetTxPst(link, absLane, downstreamDirection, &txPst);
            CHECK_SUCCESS(rc);
            link->state.usppState.txState[laneIndex].pst = txPst;
            // De-emphasis value does not apply here
            link->state.usppState.txState[laneIndex].de = 0;

            // Downstream parameters
            rc = ariesGetTxPst(link, absLane, upstreamDirection, &txPst);
            CHECK_SUCCESS(rc);
            link->state.dsppState.txState[laneIndex].pst = txPst;
            // De-emphasis value does not apply here
            link->state.dsppState.txState[laneIndex].de = 0;
        }
    }

    // Get RX TERM (Calculate based on pseudo-port path)
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int rxTerm;
        int absLane = startLane + laneIndex;

        // Upstream path
        rc = ariesGetLinkRxTerm(link, upstreamSide, absLane, &rxTerm);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].termination = rxTerm;
        // Downstream path
        rc = ariesGetLinkRxTerm(link, downstreamSide, absLane, &rxTerm);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].termination = rxTerm;
    }

    // Get RX ATT, VGA, CTLE Boost
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int boostCode;
        int attCode;
        float attValDb;
        int vgaCode;
        float boostValDb;
        int absLane = startLane + laneIndex;

        // Upstream parameters
        rc = ariesGetRxCtleBoostCode(link, upstreamSide, absLane, &boostCode);
        CHECK_SUCCESS(rc);
        rc = ariesGetRxAttCode(link, upstreamSide, absLane, &attCode);
        CHECK_SUCCESS(rc);
        attValDb = attCode * -1.5;
        rc = ariesGetRxVgaCode(link, upstreamSide, absLane, &vgaCode);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].attdB = attValDb;
        link->state.usppState.rxState[laneIndex].vgadB = vgaCode * 0.9;
        boostValDb = ariesGetRxBoostValueDb(boostCode, attValDb, vgaCode);
        link->state.usppState.rxState[laneIndex].ctleBoostdB = boostValDb;

        // Downstream parameters
        rc = ariesGetRxCtleBoostCode(link, downstreamSide, absLane, &boostCode);
        CHECK_SUCCESS(rc);
        rc = ariesGetRxAttCode(link, downstreamSide, absLane, &attCode);
        CHECK_SUCCESS(rc);
        attValDb = attCode * -1.5;
        rc = ariesGetRxVgaCode(link, downstreamSide, absLane, &vgaCode);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].attdB = attValDb;
        link->state.dsppState.rxState[laneIndex].vgadB = vgaCode * 0.9;
        boostValDb = ariesGetRxBoostValueDb(boostCode, attValDb, vgaCode);
        link->state.dsppState.rxState[laneIndex].ctleBoostdB = boostValDb;
    }

    // Get RX Pole Code
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int poleCode;
        int absLane = startLane + laneIndex;

        // Upstream parameters
        rc = ariesGetRxCtlePoleCode(link, upstreamSide, absLane, &poleCode);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].ctlePole = poleCode;

        // Downstream parameters
        rc = ariesGetRxCtlePoleCode(link, downstreamSide, absLane, &poleCode);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].ctlePole = poleCode;
    }

    // Get RX DFE Values
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int tap = 1;
        int code;
        float dfe;
        int absLane = startLane + laneIndex;

        for (tap = 1; tap <= 8; tap++)
        {
            // Upstream parameters
            rc = ariesGetRxDfeCode(link, upstreamSide, absLane, tap, &code);
            CHECK_SUCCESS(rc);

            switch(tap)
            {
                case 1:
                    dfe = code * 1.85;
                    link->state.usppState.rxState[laneIndex].dfe1 = dfe;
                    break;
                case 2:
                    dfe = code * 0.35;
                    link->state.usppState.rxState[laneIndex].dfe2 = dfe;
                    break;
                case 3:
                    dfe = code * 0.7;
                    link->state.usppState.rxState[laneIndex].dfe3 = dfe;
                    break;
                case 4:
                    dfe = code * 0.35;
                    link->state.usppState.rxState[laneIndex].dfe4 = dfe;
                    break;
                case 5:
                    dfe = code * 0.35;
                    link->state.usppState.rxState[laneIndex].dfe5 = dfe;
                    break;
                case 6:
                    dfe = code * 0.35;
                    link->state.usppState.rxState[laneIndex].dfe6 = dfe;
                    break;
                case 7:
                    dfe = code * 0.35;
                    link->state.usppState.rxState[laneIndex].dfe7 = dfe;
                    break;
                case 8:
                    dfe = code * 0.35;
                    link->state.usppState.rxState[laneIndex].dfe8 = dfe;
                    break;
                default:
                    ASTERA_ERROR("Invalid DFE tap argument");
                    return ARIES_INVALID_ARGUMENT;
                    break;
            }

            // Downstream parameters
            rc = ariesGetRxDfeCode(link, downstreamSide, absLane, tap, &code);
            CHECK_SUCCESS(rc);

            switch(tap)
            {
                case 1:
                    dfe = code * 1.85;
                    link->state.dsppState.rxState[laneIndex].dfe1 = dfe;
                    break;
                case 2:
                    dfe = code * 0.35;
                    link->state.dsppState.rxState[laneIndex].dfe2 = dfe;
                    break;
                case 3:
                    dfe = code * 0.7;
                    link->state.dsppState.rxState[laneIndex].dfe3 = dfe;
                    break;
                case 4:
                    dfe = code * 0.35;
                    link->state.dsppState.rxState[laneIndex].dfe4 = dfe;
                    break;
                case 5:
                    dfe = code * 0.35;
                    link->state.dsppState.rxState[laneIndex].dfe5 = dfe;
                    break;
                case 6:
                    dfe = code * 0.35;
                    link->state.dsppState.rxState[laneIndex].dfe6 = dfe;
                    break;
                case 7:
                    dfe = code * 0.35;
                    link->state.dsppState.rxState[laneIndex].dfe7 = dfe;
                    break;
                case 8:
                    dfe = code * 0.35;
                    link->state.dsppState.rxState[laneIndex].dfe8 = dfe;
                    break;
                default:
                    ASTERA_ERROR("Invalid DFE tap argument");
                    return ARIES_INVALID_ARGUMENT;
                    break;
            }
        }
    }

    // Get Temperature Values
    // Each PMA has a temp sensor. Read that and store value accordingly in
    // lane indexed array
    // ariesReadPmaTemp() returns max temp in 16 readings
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int absLane = startLane + laneIndex;

        // startLane = link->config.startLane;
        int pmaNum = ariesGetPmaNumber(absLane);

        // Upstream values
        float utemp;
        rc = ariesReadPmaTemp(link->device, upstreamSide,
            pmaNum, &utemp);
        CHECK_SUCCESS(rc);

        link->state.coreState.usppTempC[laneIndex] = utemp;

        // Downstream values
        float dtemp;
        rc = ariesReadPmaTemp(link->device, downstreamSide,
            pmaNum, &dtemp);
        CHECK_SUCCESS(rc);
        link->state.coreState.dsppTempC[laneIndex] = dtemp;

        link->state.coreState.usppTempAlert[laneIndex] = false;
        link->state.coreState.dsppTempAlert[laneIndex] = false;

        float thresh = link->device->tempAlertThreshC;
        if (utemp >= thresh)
        {
            link->state.coreState.usppTempAlert[laneIndex] = true;
        }
        if (dtemp >= thresh)
        {
            link->state.coreState.dsppTempAlert[laneIndex] = true;
        }
    }

    // Get Path HW State
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int absLane = startLane + laneIndex;
        int state;
        rc = ariesGetPathHWState(link, absLane, upstreamDirection, &state);
        CHECK_SUCCESS(rc);
        link->state.coreState.usppPathHWState[laneIndex] = state;
        rc = ariesGetPathHWState(link, absLane, downstreamDirection, &state);
        CHECK_SUCCESS(rc);
        link->state.coreState.dsppPathHWState[laneIndex] = state;
    }

    // Get DESKEW status (in nanoseconds)
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int status;
        int absLane = startLane + laneIndex;

        // Upstream Path
        rc = ariesGetDeskewClks(link, absLane, upstreamDirection, &status);
        CHECK_SUCCESS(rc);

        int clkPeriod;
        if (usppSpeed == 32)
        {
            clkPeriod = 1; // ns
        }
        else if (usppSpeed == 16)
        {
            clkPeriod = 2; // ns
        }
        else if (usppSpeed == 8)
        {
            clkPeriod = 4; // ns
        }
        else if (usppSpeed == 5)
        {
            clkPeriod = 8; // ns
        }
        else
        {
            // GEN 1, speed = 2.5
            clkPeriod = 16; // ns
        }

        link->state.coreState.usDeskewNs[laneIndex] = status*clkPeriod;

        // Downstream Values
        rc = ariesGetDeskewClks(link, absLane, downstreamDirection, &status);
        CHECK_SUCCESS(rc);

        // Get Clock Speed (ns)
        if (dsppSpeed == 32)
        {
            clkPeriod = 1; // ns
        }
        else if (dsppSpeed == 16)
        {
            clkPeriod = 2; // ns
        }
        else if (dsppSpeed == 8)
        {
            clkPeriod = 4; // ns
        }
        else if (dsppSpeed == 5)
        {
            clkPeriod = 8; // ns
        }
        else
        {
            // GEN 1, speed = 2.5
            clkPeriod = 16; // ns
        }

        link->state.coreState.dsDeskewNs[laneIndex] = status*clkPeriod;
    }

    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int absLane = startLane + laneIndex;
        int pathID = floor(absLane/4) * 4;
        int lane = absLane % 4;

        rc = ariesGetMinFoMVal(link->device, upstreamSide, pathID, lane,
                ARIES_PMA_RAWLANE_DIG_RX_CTL_RX_ADAPT_MM_FOM_ADDRESS, dataWord);
        CHECK_SUCCESS(rc);
        // FoM value is 7:0 of word.
        link->state.usppState.rxState[laneIndex].FoM = dataWord[0];

        rc = ariesGetMinFoMVal(link->device, downstreamSide, pathID, lane,
                ARIES_PMA_RAWLANE_DIG_RX_CTL_RX_ADAPT_MM_FOM_ADDRESS, dataWord);
        CHECK_SUCCESS(rc);
        // FoM value is 7:0 of word.
        link->state.dsppState.rxState[laneIndex].FoM = dataWord[0];
    }

    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int absLane = laneIndex + startLane;
        int state;
        // FW state
        rc = ariesGetPathFWState(link, absLane, usppRxDirection, &state);
        CHECK_SUCCESS(rc);
        link->state.coreState.usppPathFWState[laneIndex] = state;
        // FW state
        rc = ariesGetPathFWState(link, absLane, dsppRxDirection, &state);
        CHECK_SUCCESS(rc);
        link->state.coreState.dsppPathFWState[laneIndex] = state;
    }

    // Get DPLL Codes
    // Take a median of 7 readings
    uint16_t DPLLfreqs[ARIES_NUM_DPLL_FREQ_READINGS];
    uint16_t DPLLVal;
    uint16_t DPLLFreq;
    uint8_t dpll_i;
    uint8_t try_i;
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int absLane = laneIndex + startLane;
        try_i = 0;
        while (try_i < ARIES_NUM_DPLL_FREQ_READING_TRIES)
        {
            for (dpll_i = 0; dpll_i < ARIES_NUM_DPLL_FREQ_READINGS; dpll_i++)
            {
                rc = ariesGetDPLLFreq(link, upstreamSide, absLane, &DPLLVal);
                CHECK_SUCCESS(rc);
                DPLLfreqs[dpll_i] = DPLLVal;
            }

            DPLLFreq = ariesGetMedian(DPLLfreqs, ARIES_NUM_DPLL_FREQ_READINGS);

            if ((DPLLFreq >= 4098) && (DPLLFreq <= 12288))
            {
                break;
            }
            try_i++;
        }
        link->state.usppState.rxState[laneIndex].DPLLCode = DPLLFreq;

        try_i = 0;
        while (try_i < ARIES_NUM_DPLL_FREQ_READING_TRIES)
        {
            for (dpll_i = 0; dpll_i < ARIES_NUM_DPLL_FREQ_READINGS; dpll_i++)
            {
                rc = ariesGetDPLLFreq(link, downstreamSide, absLane, &DPLLVal);
                CHECK_SUCCESS(rc);
                DPLLfreqs[dpll_i] = DPLLVal;
            }

            DPLLFreq = ariesGetMedian(DPLLfreqs, ARIES_NUM_DPLL_FREQ_READINGS);

            if ((DPLLFreq >= 4098) && (DPLLFreq <= 12288))
            {
                break;
            }
            try_i++;
        }
        link->state.dsppState.rxState[laneIndex].DPLLCode = DPLLFreq;
    }

    // Get RX EQ parameters
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        int absLane = startLane + laneIndex;
        int speed;
        int presetReq;
        int preReq;
        int curReq;
        int pstReq;
        int req;
        int req1;
        int req2;
        int req3;
        int fom;
        int fom1;
        int fom2;
        int fom3;

        ///////////////////// Upstream parameters ///////////////////

        // EQ Speed RX Param
        rc = ariesGetLastEqSpeed(link, absLane, usppRxDirection, &speed);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].lastEqRate = speed;

        // EQ Speed TX Param
        rc = ariesGetLastEqSpeed(link, absLane, usppTxDirection, &speed);
        CHECK_SUCCESS(rc);
        link->state.usppState.txState[laneIndex].lastEqRate = speed;

        // Get EQ Preset values
        rc = ariesGetLastEqReqPreset(link, absLane, usppTxDirection,
                &presetReq);
        CHECK_SUCCESS(rc);
        link->state.usppState.txState[laneIndex].lastPresetReq = presetReq;

        // If final request was a preset, this is the pre value
        rc = ariesGetLastEqReqPre(link, absLane, usppTxDirection,
                &preReq);
        CHECK_SUCCESS(rc);
        link->state.usppState.txState[laneIndex].lastPreReq = preReq;

        // If final request was a preset, this is the current value
        rc = ariesGetLastEqReqCur(link, absLane, usppTxDirection,
                &curReq);
        CHECK_SUCCESS(rc);
        link->state.usppState.txState[laneIndex].lastCurReq = curReq;

        // If final request was a preset, this is the post value
        rc = ariesGetLastEqReqPst(link, absLane, usppTxDirection,
                &pstReq);
        CHECK_SUCCESS(rc);
        link->state.usppState.txState[laneIndex].lastPstReq = pstReq;

        // Final preset request
        rc = ariesGetLastEqPresetReq(link, absLane, usppRxDirection, 3,
                &req);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].lastPresetReq = req;

        // Final-1 preset request
        rc = ariesGetLastEqPresetReq(link, absLane, usppRxDirection, 2,
                &req1);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].lastPresetReqM1 = req1;

        // Final-2 preset request
        rc = ariesGetLastEqPresetReq(link, absLane, usppRxDirection, 1,
                &req2);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].lastPresetReqM2 = req2;

        // Final-3 preset request
        rc = ariesGetLastEqPresetReq(link, absLane, usppRxDirection, 0,
                &req3);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].lastPresetReqM3 = req3;

        // Final preset request FOM value
        rc = ariesGetLastEqPresetReqFOM(link, absLane, usppRxDirection, 3,
                &fom);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].lastPresetReqFom = fom;

        // Final-1 preset request FOM value
        rc = ariesGetLastEqPresetReqFOM(link, absLane, usppRxDirection, 2,
                &fom1);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].lastPresetReqFomM1 = fom1;

        // Final-2 preset request FOM Value
        rc = ariesGetLastEqPresetReqFOM(link, absLane, usppRxDirection, 1,
                &fom2);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].lastPresetReqFomM2 = fom2;

        // Final-3 preset request FOM Value
        rc = ariesGetLastEqPresetReqFOM(link, absLane, usppRxDirection, 0,
                &fom3);
        CHECK_SUCCESS(rc);
        link->state.usppState.rxState[laneIndex].lastPresetReqFomM3 = fom3;

        ///////////////////// Downstream parameters ///////////////////

        // Downstream EQ speed param (RX)
        rc = ariesGetLastEqSpeed(link, absLane, dsppRxDirection, &speed);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].lastEqRate = speed;

        // Downstream EQ speed param (TX)
        rc = ariesGetLastEqSpeed(link, absLane, dsppTxDirection, &speed);
        CHECK_SUCCESS(rc);
        link->state.dsppState.txState[laneIndex].lastEqRate = speed;

        // Get EQ Preset values
        rc = ariesGetLastEqReqPreset(link, absLane, dsppTxDirection,
                &presetReq);
        CHECK_SUCCESS(rc);
        link->state.dsppState.txState[laneIndex].lastPresetReq = presetReq;

        // If final request was a preset, this is the pre value
        rc = ariesGetLastEqReqPre(link, absLane, dsppTxDirection,
                &preReq);
        CHECK_SUCCESS(rc);
        link->state.dsppState.txState[laneIndex].lastPreReq = preReq;

        // If final request was a preset, this is the cur value
        rc = ariesGetLastEqReqCur(link, absLane, dsppTxDirection,
                &curReq);
        CHECK_SUCCESS(rc);
        link->state.dsppState.txState[laneIndex].lastCurReq = curReq;

        // If final request was a preset, this is the post value
        rc = ariesGetLastEqReqPst(link, absLane, dsppTxDirection,
                &pstReq);
        CHECK_SUCCESS(rc);
        link->state.dsppState.txState[laneIndex].lastPstReq = pstReq;

        // Final preset request
        rc = ariesGetLastEqPresetReq(link, absLane, dsppRxDirection, 3,
                &req);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].lastPresetReq = req;

        // Final-1 preset request
        rc = ariesGetLastEqPresetReq(link, absLane, dsppRxDirection, 2,
                &req1);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].lastPresetReqM1 = req1;

        // Final-2 preset request
        rc = ariesGetLastEqPresetReq(link, absLane, dsppRxDirection, 1,
                &req2);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].lastPresetReqM2 = req2;

        // Final-3 preset request
        rc = ariesGetLastEqPresetReq(link, absLane, dsppRxDirection, 0,
                &req3);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].lastPresetReqM3 = req3;

        // Final preset request FOM
        rc = ariesGetLastEqPresetReqFOM(link, absLane, dsppRxDirection, 3,
                &fom);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].lastPresetReqFom = fom;

        // Final-1 preset request FOM
        rc = ariesGetLastEqPresetReqFOM(link, absLane, dsppRxDirection, 2,
                &fom1);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].lastPresetReqFomM1 = fom1;

        // Final-2 preset request FOM
        rc = ariesGetLastEqPresetReqFOM(link, absLane, dsppRxDirection, 1,
                &fom2);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].lastPresetReqFomM2 = fom2;

        // Final-3 preset request FOM
        rc = ariesGetLastEqPresetReqFOM(link, absLane, dsppRxDirection, 0,
                &fom3);
        CHECK_SUCCESS(rc);
        link->state.dsppState.rxState[laneIndex].lastPresetReqFomM3 = fom3;
    }

    return ARIES_SUCCESS;
}


/*
 * Initialize LTSSM logger
 */
AriesErrorType ariesLTSSMLoggerInit(
        AriesLinkType* link,
        uint8_t oneBatchModeEn,
        AriesLTSSMVerbosityType verbosity)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    // Need larger buffer to write print enables
    uint8_t dataBytes8[8];

    int width = link->state.width;
    int laneIndex;
    int absLane;
    int address;
    AriesI2CDriverType* i2cDriver = link->device->i2cDriver;
    int startLane = ariesGetStartLane(link);
    int baseAddress;

    // Initialize Main Micro logger
    baseAddress = link->device->mm_print_info_struct_addr;

    // Set One Batch Mode
    dataByte[0] = oneBatchModeEn;
    address = baseAddress + ARIES_PRINT_INFO_STRUCT_ONE_BATCH_MODE_EN_OFFSET;
    rc = ariesWriteByteDataMainMicroIndirect(i2cDriver, address, dataByte);
    CHECK_SUCCESS(rc);

    // Set print enables
    int pcIndex;
    switch(verbosity)
    {
        case ARIES_LTSSM_VERBOSITY_HIGH:
            for (pcIndex = 0; pcIndex < ARIES_MM_PRINT_INFO_NUM_PRINT_CLASS_EN;
                pcIndex++)
            {
                dataBytes8[pcIndex] = 0xff;
            }
            break;
        default:
            ASTERA_ERROR("Invalid LTSSM logger verbosity");
            return ARIES_INVALID_ARGUMENT;
            break;
    }
    address = baseAddress + ARIES_MM_PRINT_INFO_STRUCT_PRINT_CLASS_EN_OFFSET;
    rc = ariesWriteBlockDataMainMicroIndirect(i2cDriver, address, 8,
        dataBytes8);
    CHECK_SUCCESS(rc);

    // Initialize Path Micro logger
    baseAddress = link->device->pm_print_info_struct_addr;

    // Use laneIndex as path since lane is split between 2 paths and min lanes
    // per link is two. For example - lanes 2 and 3 would be path 2 and 3.
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        // absLane = link->config.startLane + laneIndex;
        absLane = startLane + laneIndex;

        // Downstream paths
        // Set One Batch Mode
        dataByte[0] = oneBatchModeEn;
        address = baseAddress +
            ARIES_PRINT_INFO_STRUCT_ONE_BATCH_MODE_EN_OFFSET;
        rc = ariesWriteByteDataPathMicroIndirect(i2cDriver, absLane, address,
                dataByte);
        CHECK_SUCCESS(rc);

        // Set Print Class enables
        // Can only write micros 8 bytes at a time. Hence we have to spilt the
        // writes. There are 16 entires, so split writes into 2
        int pcCount;
        for (pcCount = 0; pcCount < 2; pcCount++)
        {
            address = baseAddress +
                ARIES_PM_PRINT_INFO_STRUCT_PRINT_CLASS_EN_OFFSET +
                (pcCount*8);
            for (pcIndex = 0; pcIndex < 8; pcIndex++)
            {
                dataBytes8[pcIndex] = 0xff;
            }
            rc = ariesWriteBlockDataPathMicroIndirect(i2cDriver, absLane,
                    address, 8, dataBytes8);
            CHECK_SUCCESS(rc);
        }

    }

    return ARIES_SUCCESS;
}


/*
 * Set Print EN in logger
 */
AriesErrorType ariesLTSSMLoggerPrintEn(
        AriesLinkType* link,
        uint8_t printEn)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    int width = link->state.width;
    int laneIndex;
    int absLane;
    int address;
    int baseAddress;

    int startLane = ariesGetStartLane(link);

    // Do for Main Micro
    baseAddress = link->device->mm_print_info_struct_addr;
    address = baseAddress + ARIES_PRINT_INFO_STRUCT_PRINT_EN_OFFSET;
    dataByte[0] = printEn;
    rc = ariesWriteByteDataMainMicroIndirect(link->device->i2cDriver, address,
            dataByte);
    CHECK_SUCCESS(rc);

    // Do for Path Micros
    // Use laneIndex as path since lane is split between 2 paths and min lanes
    // per link is two. For example - lanes 2 and 3 would be path 2 and 3.
    baseAddress = link->device->pm_print_info_struct_addr;

    // Use laneIndex as path since lane is split between 2 paths and min lanes
    // per link is two. For example - lanes 2 and 3 would be path 2 and 3.
    for (laneIndex = 0; laneIndex < width; laneIndex++)
    {
        // absLane = link->config.startLane + laneIndex;
        absLane = startLane + laneIndex;
        address = baseAddress + ARIES_PRINT_INFO_STRUCT_PRINT_EN_OFFSET;
        dataByte[0] = printEn;

        rc = ariesWriteByteDataPathMicroIndirect(link->device->i2cDriver,
                absLane, address, dataByte);
        CHECK_SUCCESS(rc);
    }

    return ARIES_SUCCESS;
}


/*
 * Read an entry from the LTSSM logger
 */
AriesErrorType ariesLTSSMLoggerReadEntry(
        AriesLinkType* link,
        AriesLTSSMLoggerEnumType logType,
        int* offset,
        AriesLTSSMEntryType* entry)
{
    AriesErrorType rc;
    int baseAddress;
    int address;
    uint8_t dataByte[1];

    // Main Micro entry
    if (logType == ARIES_LTSSM_LINK_LOGGER)
    {
        baseAddress = link->device->mm_print_info_struct_addr +
            ARIES_PRINT_INFO_STRUCT_PRINT_BUFFER_OFFSET;
        address = baseAddress + *offset;
        rc = ariesReadByteDataMainMicroIndirect(link->device->i2cDriver,
                address, dataByte);
        CHECK_SUCCESS(rc);
        entry->data = dataByte[0];
        entry->offset = *offset;
        (*offset)++;
    }
    else
    {
        baseAddress = link->device->pm_print_info_struct_addr +
            ARIES_PRINT_INFO_STRUCT_PRINT_BUFFER_OFFSET;
        address = baseAddress + *offset;
        rc = ariesReadByteDataPathMicroIndirect(link->device->i2cDriver,
                logType, address, dataByte);
        CHECK_SUCCESS(rc);
        entry->data = dataByte[0];
        entry->offset = *offset;
        (*offset)++;
    }
    return ARIES_SUCCESS;
}


/*
 * Set max data rate
 */
AriesErrorType ariesSetMaxDataRate(
        AriesDeviceType* device,
        AriesMaxDataRateType rate)
{
    AriesErrorType rc;
    uint8_t dataBytes[4];
    uint32_t val;
    uint32_t mask;

    rc = ariesReadBlockData(device->i2cDriver, 0, 4, dataBytes);
    CHECK_SUCCESS(rc);
    val = dataBytes[0] + (dataBytes[1] << 8) + (dataBytes[2] << 16)
        + (dataBytes[3] << 24);

    mask = ~(7 << 24) & 0xffffffff;
    val &= mask;
    val |= (rate << 24);

    dataBytes[0] = val & 0xff;
    dataBytes[1] = (val >> 8) & 0xff;
    dataBytes[2] = (val >> 16) & 0xff;
    dataBytes[3] = (val >> 24) & 0xff;

    rc = ariesWriteBlockData(device->i2cDriver, 0, 4, dataBytes);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Set the GPIO value
 */
AriesErrorType ariesSetGPIO(
        AriesDeviceType* device,
        int gpioNum,
        bool value)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    rc = ariesReadByteData(device->i2cDriver, 0x916, dataByte);
    CHECK_SUCCESS(rc);
    if (value == 1)
    {
        // Set the corresponding GPIO bit to 1
        dataByte[0] |= (1 << gpioNum);
    }
    else
    {
        // Clear the corresponding GPIO bit to 0
        dataByte[0] &= ~(1 << gpioNum);
    }

    rc = ariesWriteByteData(device->i2cDriver, 0x916, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Get the GPIO value
 */
AriesErrorType ariesGetGPIO(
        AriesDeviceType* device,
        int gpioNum,
        bool* value)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    rc = ariesReadByteData(device->i2cDriver, 0x915, dataByte);
    CHECK_SUCCESS(rc);
    *value = (dataByte[0] >> gpioNum) & 1;

    return ARIES_SUCCESS;
}


/**
 * Toggle the GPIO value
 */
AriesErrorType ariesToggleGPIO(
        AriesDeviceType* device,
        int gpioNum)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    rc = ariesReadByteData(device->i2cDriver, 0x916, dataByte);
    CHECK_SUCCESS(rc);

    dataByte[0] ^= (1 << gpioNum);

    rc = ariesWriteByteData(device->i2cDriver, 0x916, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Set the GPIO direction
 */
AriesErrorType ariesSetGPIODirection(
        AriesDeviceType* device,
        int gpioNum,
        bool value)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    rc = ariesReadByteData(device->i2cDriver, 0x917, dataByte);
    CHECK_SUCCESS(rc);
    if (value == 1)
    {
        // Set the corresponding GPIO bit to 1
        dataByte[0] |= (1 << gpioNum);
    }
    else
    {
        // Clear the corresponding GPIO bit to 0
        dataByte[0] &= ~(1 << gpioNum);
    }

    rc = ariesWriteByteData(device->i2cDriver, 0x917, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Get the GPIO direction
 */
AriesErrorType ariesGetGPIODirection(
        AriesDeviceType* device,
        int gpioNum,
        bool* value)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    rc = ariesReadByteData(device->i2cDriver, 0x917, dataByte);
    CHECK_SUCCESS(rc);
    *value = (dataByte[0] >> gpioNum) & 1;

    return ARIES_SUCCESS;
}


/*
 * Enable Aries Test Mode for PRBS generation and checking
 */
AriesErrorType ariesTestModeEnable(
        AriesDeviceType* device)
{
    AriesErrorType rc;
    int side = 0;
    int lane = 0;
    uint8_t dataByte[1];
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;

    // Assert register PERST
    ASTERA_INFO("Assert internal PERST");
    dataByte[0] = 0x00;
    rc = ariesWriteByteData(device->i2cDriver, 0x604, dataByte); // Assert for Links[7:0]
    CHECK_SUCCESS(rc);
    usleep(100000); // 100 ms

    // Put MM in reset
    ASTERA_INFO("Put MM into reset");
    rc = ariesSetMMReset(device, true);
    CHECK_SUCCESS(rc);
    usleep(100000); // 100 ms

    // Verify
    //rc = ariesReadBlockData(device->i2cDriver, 0x602, 2, dataWord);
    //CHECK_SUCCESS(rc);
    //uint8_t mm_reset = dataWord[1] >> 2;
    //ASTERA_INFO("Read back MM reset: %d", mm_reset);

    // MM could have been halted in the middle of a temp sensor reading. Undo those overrides.
    for (side = 0; side < 2; side++)
    {
        for (lane = 0; lane < 16; lane++)
        {
            qs = lane / 4;
            qsLane = lane % 4;
            // Disable the temp sensor (one control bit per PMA instance)
            if (qsLane == 0)
            {
                rc = ariesReadWordPmaIndirect(device->i2cDriver, side, qs, 0xed, dataWord);
                CHECK_SUCCESS(rc);
                dataWord[0] &= ~(1 << 3);
                rc = ariesWriteWordPmaIndirect(device->i2cDriver, side, qs, 0xed, dataWord);
                CHECK_SUCCESS(rc);
                rc = ariesReadWordPmaIndirect(device->i2cDriver, side, qs, 0xea, dataWord);
                CHECK_SUCCESS(rc);
                dataWord[0] &= ~(1 << 5);
                dataWord[0] &= ~(1 << 6);
                rc = ariesWriteWordPmaIndirect(device->i2cDriver, side, qs, 0xea, dataWord);
                CHECK_SUCCESS(rc);
            }
            // Un-freeze PMA FW
            rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane, 0x2060, dataWord);
            CHECK_SUCCESS(rc);
            dataWord[1] &= ~(1 << 6);
            rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane, 0x2060, dataWord);
            CHECK_SUCCESS(rc);
        }
    }
    // In some versions of FW, MPLLB output divider is overriden to /2. Undo this change.
    for (side = 0; side < 2; side++)
    {
        for (lane = 0; lane < 16; lane++)
        {
            qs = lane / 4;
            qsLane = lane % 4;
            // one control bit per PMA instance
            if (qsLane == 0)
            {
                dataWord[0] = 0x20;
                dataWord[1] = 0x00;
                rc = ariesWriteWordPmaIndirect(device->i2cDriver, side, qs,
                    ARIES_PMA_SUP_DIG_MPLLB_OVRD_IN_0, dataWord);
                CHECK_SUCCESS(rc);
            }
        }
    }
    // Put Receivers into standby
    for (side = 0; side < 2; side++)
    {
        for (lane = 0; lane < 16; lane++)
        {
            rc = ariesPipeRxStandbySet(device, side, lane, true);
            CHECK_SUCCESS(rc);
        }
    }
    usleep(10000); // 10 ms

    // Transition to Powerdown=0 (P0)
    for (side = 0; side < 2; side++)
    {
        // One powerdown control for every grouping of two lanes
        for (lane = 0; lane < 16; lane += 2)
        {
            rc = ariesPipePowerdownSet(device, side, lane, ARIES_PIPE_POWERDOWN_P0);
            CHECK_SUCCESS(rc);
            usleep(10000); // 10 ms
            rc = ariesPipePowerdownCheck(device, side, lane, ARIES_PIPE_POWERDOWN_P0);
            CHECK_SUCCESS(rc);
        }
    }
    usleep(10000); // 10 ms

    // Enable RX terminations
    for (side = 0; side < 2; side++)
    {
        for (lane = 0; lane < 16; lane++)
        {
            rc = ariesPipeRxTermSet(device, side, lane, true);
            CHECK_SUCCESS(rc);
        }
    }
    usleep(10000); // 10 ms

    // Disable PCS block align control
    for (side = 0; side < 2; side++)
    {
        // One blockaligncontrol control for every grouping of two lanes
        for (lane = 0; lane < 16; lane += 2)
        {
            rc = ariesPipeBlkAlgnCtrlSet(device, side, lane, false);
            CHECK_SUCCESS(rc);
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Disable Aries Test Mode for PRBS generation and checking
 */
AriesErrorType ariesTestModeDisable(
        AriesDeviceType* device)
{
    AriesErrorType rc;
    int side = 0;
    int lane = 0;
    uint8_t dataByte[1];
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;

    for (side = 0; side < 2; side++)
    {
        for (lane = 0; lane < 16; lane++)
        {
            qs = lane / 4;
            qsLane = lane % 4;
            // Undo RxReq override
            rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
                ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataWord);
            CHECK_SUCCESS(rc);
            dataWord[0] &= ~(1 << 3);
            rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
                ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataWord);
            CHECK_SUCCESS(rc);
            // Disable Rx terminations
            rc = ariesPipeRxTermSet(device, side, lane, false);
            CHECK_SUCCESS(rc);
            // Undo Rx inversion overrides
            rc = ariesPMARxInvertSet(device, side, lane, false, false);
            CHECK_SUCCESS(rc);
            // Set de-emphasis back to -3.5 dB
            rc = ariesPipeDeepmhasisSet(device, side, lane, 1, ARIES_PIPE_DEEMPHASIS_PRESET_NONE, 0, 44, 0);
            // Turn off PRBS generators
            rc = ariesPMABertPatGenConfig(device, side, lane, 0);
            CHECK_SUCCESS(rc);
            // Turn off PRBS checkers
            rc = ariesPMABertPatChkConfig(device, side, lane, 0);
            CHECK_SUCCESS(rc);
            // Undo block align control override
            rc = ariesPipeBlkAlgnCtrlSet(device, side, lane, false);
            CHECK_SUCCESS(rc);
            // Undo Tx/Rx data enable override
            rc = ariesPMATxDataEnSet(device, side, lane, false);
            CHECK_SUCCESS(rc);
            rc = ariesPMARxDataEnSet(device, side, lane, false);
            CHECK_SUCCESS(rc);
            // Rate change to Gen1
            if ((lane % 2) == 0)
            {
                // Rate is controlled for each group of two lanes, so only do this once per pair
                rc = ariesPipeRateChange(device, side, lane, 1); // rate=1 for Gen1
                CHECK_SUCCESS(rc);
            }
            // Powerdown to P1 (P1 is value 2)
            rc = ariesPipePowerdownSet(device, side, lane, ARIES_PIPE_POWERDOWN_P1);
            CHECK_SUCCESS(rc);
            // Undo Rxstandby override
            rc = ariesPipeRxStandbySet(device, side, lane, false);
            CHECK_SUCCESS(rc);
        }
    }

    // Take MM out of reset
    rc = ariesSetMMReset(device, false);
    CHECK_SUCCESS(rc);
    // De-assert register PERST
    dataByte[0] = 0xff;
    rc = ariesWriteByteData(device->i2cDriver, 0x604, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Set the desired Aries Test Mode data rate
 */
AriesErrorType ariesTestModeRateChange(
        AriesDeviceType* device,
        AriesMaxDataRateType rate)
{
    AriesErrorType rc;
    int side = 0;
    int lane = 0;

    // PCS indexes rate from 0
    for (side = 0; side < 2; side++)
    {
        // Rate is controlled for each grouping of two lanes, so only do this once per pair
        for (lane = 0; lane < 16; lane += 2)
        {
            rc = ariesPipeRateChange(device, side, lane, rate); // rate = 1, 2, ..., 5
            CHECK_SUCCESS(rc);
            usleep(50000); // 50 ms
        }
        // Confirm that every lane changed rate successfully
        for (lane = 0; lane < 16; lane++)
        {
            // Confirm rate change by reading PMA registers
            rc = ariesPipeRateCheck(device, side, lane, rate); // rate = 1, 2, ..., 5
            CHECK_SUCCESS(rc);
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Aries Test Mode transmitter configuration
 */
AriesErrorType ariesTestModeTxConfig(
        AriesDeviceType* device,
        AriesPRBSPatternType pattern,
        int preset,
        bool enable)
{
    AriesErrorType rc;
    int side = 0;
    int lane = 0;
    uint8_t dataByte[1];
    int rate = 0;
    int de = 0;
    AriesPRBSPatternType mode = DISABLED;

    // Decode the pattern argument
    if (enable)
    {
        // Set the generator mode (pattern)
        mode = pattern;

        // Check any Path's rate (assumption is they're all the same)
        // qs_2, pth_wrap_0 is absolute lane 8
        rc = ariesReadRetimerRegister(device->i2cDriver, 0, 8,
            ARIES_RET_PTH_GBL_MAC_PHY_RATE_AND_PCLK_RATE_ADDR, 1, dataByte);
        CHECK_SUCCESS(rc);
        rate = dataByte[0] & 0x7;
        if (rate >= 2) // rate==2 is Gen3
        {
            // For Gen3/4/5, use presets
            de = ARIES_PIPE_DEEMPHASIS_DE_NONE;
            if (preset < 0)
            {
                preset = 0;
            }
            else if (preset > 10)
            {
                preset = 10;
            }
        }
        else
        {
            // For Gen1/2, use de-emphasis -3.5dB (default)
            de = 1;
            preset = ARIES_PIPE_DEEMPHASIS_PRESET_NONE;
        }
        for (side = 0; side < 2; side++)
        {
            for (lane = 0; lane < 16; lane++)
            {
                rc = ariesPipeDeepmhasisSet(device, side, lane, de, preset, 0, 44, 0);
                CHECK_SUCCESS(rc);
            }
        }
    }
    for (side = 0; side < 2; side++)
    {
        for (lane = 0; lane < 16; lane++)
        {
            rc = ariesPMABertPatGenConfig(device, side, lane, mode);
            CHECK_SUCCESS(rc);
            usleep(10000); // 10 ms
            rc = ariesPMATxDataEnSet(device, side, lane, enable);
            CHECK_SUCCESS(rc);
            usleep(10000); // 10 ms
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Aries Test Mode receiver configuration
 */
AriesErrorType ariesTestModeRxConfig(
        AriesDeviceType* device,
        AriesPRBSPatternType pattern,
        bool enable)
{
    AriesErrorType rc;
    int side = 0;
    int lane = 0;
    uint8_t dataByte[1];
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;
    int rate = 0;

    if (enable)
    {
        for (side = 0; side < 2; side++)
        {
            for (lane = 0; lane < 16; lane++)
            {
                rc = ariesPMAPCSRxReqBlock(device, side, lane);
                CHECK_SUCCESS(rc);
                rc = ariesPMARxDataEnSet(device, side, lane, true);
                CHECK_SUCCESS(rc);
                usleep(10000); // 10 ms
            }
        }
        usleep(500000); // 500 ms
        // Adapt the receivers for Gen3/4/5
        // Check any Path's rate (assumption is they're all the same)
        // qs_2, pth_wrap_0 is absolute lane 8
        rc = ariesReadRetimerRegister(device->i2cDriver, 0, 8,
            ARIES_RET_PTH_GBL_MAC_PHY_RATE_AND_PCLK_RATE_ADDR, 1, dataByte);
        CHECK_SUCCESS(rc);
        rate = dataByte[0] & 0x7;
        if (rate >= 2) // rate==2 is Gen3
        {
            ASTERA_INFO("Run Rx adaptation....");
            for (side = 0; side < 2; side++)
            {
                for (lane = 0; lane < 16; lane++)
                {
                    rc = ariesPipeRxAdapt(device, side, lane);
                    CHECK_SUCCESS(rc);
                    usleep(10000); // 10 ms
                }
            }
        }
        usleep(500000); // 500 ms
        // Configure pattern checker
        for (side = 0; side < 2; side++)
        {
            for (lane = 0; lane < 16; lane++)
            {
                rc = ariesPMABertPatChkConfig(device, side, lane, pattern);
                CHECK_SUCCESS(rc);
                usleep(10000); // 10 ms
            }
        }
        // Clear patter checkers
        //rc = ariesTestModeRxEcountClear(device);
        //CHECK_SUCCESS(rc);
        // Detect/correct polarity
        for (side = 0; side < 2; side++)
        {
            for (lane = 0; lane < 16; lane++)
            {
                rc = ariesPMABertPatChkDetectCorrectPolarity(device, side, lane);
                CHECK_SUCCESS(rc);
                usleep(10000); // 10 ms
            }
        }
    }
    else
    {
        for (side = 0; side < 2; side++)
        {
            for (lane = 0; lane < 16; lane++)
            {
                qs = lane / 4;
                qsLane = lane % 4;
                rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
                    ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataWord);
                CHECK_SUCCESS(rc);
                dataWord[0] &= ~(1 << 3);
                rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
                    ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataWord);
                CHECK_SUCCESS(rc);
                rc = ariesPMARxDataEnSet(device, side, lane, false);
                CHECK_SUCCESS(rc);
                rc = ariesPMABertPatChkConfig(device, side, lane, DISABLED);
                CHECK_SUCCESS(rc);
            }
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Aries Test Mode read error count
 */
AriesErrorType ariesTestModeRxEcountRead(
        AriesDeviceType* device,
        int* ecount)
{
    AriesErrorType rc;
    int side = 0;
    int lane = 0;
    int ecountVal[1];

    for (side = 0; side < 2; side++)
    {
        for (lane = 0; lane < 16; lane++)
        {
            rc = ariesPMABertPatChkSts(device, side, lane, ecountVal);
            CHECK_SUCCESS(rc);
            //ASTERA_INFO("Side:%d, Lane:%02d, ECOUNT = %d", side, lane, ecountVal[0]);
            int index = side*16 + lane;
            ecount[index] = ecountVal[0];
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Aries Test Mode clear error count
 */
AriesErrorType ariesTestModeRxEcountClear(
        AriesDeviceType* device)
{
    AriesErrorType rc;
    int side = 0;
    int lane = 0;

    for (side = 0; side < 2; side++)
    {
        for (lane = 0; lane < 16; lane++)
        {
            rc = ariesPMABertPatChkToggleSync(device, side, lane);
            CHECK_SUCCESS(rc);
            rc = ariesPMABertPatChkToggleSync(device, side, lane);
            CHECK_SUCCESS(rc);
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Aries Test Mode read FoM
 */
AriesErrorType ariesTestModeRxFomRead(
        AriesDeviceType* device,
        int* fom)
{
    AriesErrorType rc;
    int side;
    int lane;
    int fomVal[1];

    for (side = 0; side < 2; side++)
    {
        for (lane = 0; lane < 16; lane++)
        {
            rc = ariesPipeFomGet(device, side, lane, fomVal);
            CHECK_SUCCESS(rc);
            //ASTERA_INFO("Side:%d, Lane:%02d, FOM = 0x%x", side, lane, fomVal[0]);
            int index = side*16 + lane;
            fom[index] = fomVal[0];
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Aries Test Mode read Rx valid
 */
AriesErrorType ariesTestModeRxValidRead(
        AriesDeviceType* device)
{
    AriesErrorType rc;
    int side = 0;
    int lane = 0;
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;
    bool rxvalid = 0;

    for (side = 0; side < 2; side++)
    {
        for (lane = 0; lane < 16; lane++)
        {
            qs = lane / 4;
            qsLane = lane % 4;
            rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane, 0x1033, dataWord);
            CHECK_SUCCESS(rc);
            rxvalid = (dataWord[0] >> 1) & 0x1;
            ASTERA_INFO("Side:%d, Lane:%02d, PHY rxvalid = %d", side, lane, rxvalid);
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Aries Test Mode inject error
 */
AriesErrorType ariesTestModeTxErrorInject(
        AriesDeviceType* device)
{
    AriesErrorType rc;
    int side = 0;
    int lane = 0;
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;

    for (side = 0; side < 2; side++)
    {
        for (lane = 0; lane < 16; lane++)
        {
            qs = lane / 4;
            qsLane = lane % 4;
            rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane, 0x1072, dataWord);
            CHECK_SUCCESS(rc);
            dataWord[0] |= (1 << 4);
            rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane, 0x1072, dataWord);
            CHECK_SUCCESS(rc);
            dataWord[0] &= ~(1 << 4);
            rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane, 0x1072, dataWord);
            CHECK_SUCCESS(rc);
        }
    }

    return ARIES_SUCCESS;
}

#ifdef __cplusplus
}
#endif
