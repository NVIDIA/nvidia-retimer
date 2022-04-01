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
 * @file aries_misc.c
 * @brief Implementation of helper functions for the SDK.
 */

#include "include/aries_misc.h"

#ifdef __cplusplus
extern "C" {
#endif

AriesErrorType ariesReadFwVersion(
        AriesI2CDriverType* i2cDriver,
        int offset,
        uint8_t* dataVal)
{
    AriesErrorType rc;
    int addr = ARIES_MAIN_MICRO_FW_INFO + offset;
    if (offset == ARIES_MM_FW_VERSION_BUILD)
    {
        rc = ariesReadBlockDataMainMicroIndirect(i2cDriver, addr, 2, dataVal);
        CHECK_SUCCESS(rc);
    }
    else
    {
        rc = ariesReadByteDataMainMicroIndirect(i2cDriver, addr, dataVal);
        CHECK_SUCCESS(rc);
    }
    return ARIES_SUCCESS;
}

/*
 * I2C Master init for EEPROM Write-Thru
 */
AriesErrorType ariesI2CMasterInit(
        AriesI2CDriverType* i2cDriver)
{
    uint8_t dataByte[1];
    uint8_t dataWord[2];
    AriesErrorType rc;

    dataByte[0] = 0;
    rc = ariesI2CMasterWriteCtrlReg(i2cDriver, 0x6c, 1, dataByte);
    CHECK_SUCCESS(rc);
    dataWord[0] = 0xe5;
    dataWord[1] = 0xf;
    rc = ariesI2CMasterWriteCtrlReg(i2cDriver, 0, 2, dataWord);
    CHECK_SUCCESS(rc);
    dataByte[0] = 0x50;
    rc = ariesI2CMasterWriteCtrlReg(i2cDriver, 0x04, 1, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 0;
    rc = ariesI2CMasterWriteCtrlReg(i2cDriver, 0x38, 1, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 4;
    rc = ariesI2CMasterWriteCtrlReg(i2cDriver, 0x3c, 1, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 1;
    rc = ariesI2CMasterWriteCtrlReg(i2cDriver, 0x6c, 1, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

/*
 * Write to I2C Master ctrl register
 */
AriesErrorType ariesI2CMasterWriteCtrlReg(
        AriesI2CDriverType* i2cDriver,
        uint32_t address,
        uint8_t lengthDataBytes,
        uint8_t* values)
{
    AriesErrorType rc;
    uint8_t addr[1];
    addr[0] = address;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_IC_CMD_ADDR, addr);
    CHECK_SUCCESS(rc);
    uint8_t dataBytes[4] = {0, 0, 0, 0};
    uint16_t dataAddress[4] = {
        ARIES_I2C_MST_DATA0_ADDR,
        ARIES_I2C_MST_DATA1_ADDR,
        ARIES_I2C_MST_DATA2_ADDR,
        ARIES_I2C_MST_DATA3_ADDR
    };

    int indx;
    for (indx = 0; indx < lengthDataBytes; indx++)
    {
        dataBytes[indx] = values[indx];
    }

    for (indx = 0; indx < 4; indx++)
    {
        uint8_t tmpVal[1];
        tmpVal[0] = dataBytes[indx];
        rc = ariesWriteByteData(i2cDriver, dataAddress[indx], tmpVal);
        CHECK_SUCCESS(rc);
    }

    // self.wr_csr_cmd(1)
    uint8_t cmd[1];
    cmd[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, cmd);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


AriesErrorType ariesI2CMasterSetFrequency(
        AriesI2CDriverType* i2cDriver,
        int frequencyHz)
{
    AriesErrorType rc;

    // Check if desired frequency is within supported range
    // return invalid arg error if not
    if (frequencyHz > 1000000)
    {
        ASTERA_ERROR("Cannot set I2C Master frequency greater than 1MHz");
        return ARIES_INVALID_ARGUMENT;
    }
    else if (frequencyHz < 400000)
    {
        ASTERA_ERROR("Cannot set I2C Master frequency less than 400KHz");
        return ARIES_INVALID_ARGUMENT;
    }

    int defaultSclLowCnt = 0x28a;
    int defaultSclHighCnt = 0x12c;
    int defaultFreqHz = 935000;

    int newSclLowCnt = (defaultFreqHz/frequencyHz) * defaultSclLowCnt;
    int newSclHighCnt = (defaultFreqHz/frequencyHz) * defaultSclHighCnt;

    uint8_t dataWord[2];
    uint8_t dataByte[1];

    // Reset I2C IP
    // self.csr.misc.hw_rst = 0x200
    dataWord[0] = 0x0;
    dataWord[1] = 0x2;
    rc = ariesWriteBlockData(i2cDriver, ARIES_HW_RST_ADDR, 2, dataWord);
    CHECK_SUCCESS(rc);

    // Set IC_ENABLE=0 to allow changing the LCNT/HCNT settings
    // self.csr.muc.al_main_ext_csr.i2c_mst_addr   = 0x6c
    dataByte[0] = 0x6c;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_IC_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    // self.csr.muc.al_main_ext_csr.i2c_mst_data_0 = 0
    dataByte[0] = 0x0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    // self.csr.muc.al_main_ext_csr.i2c_mst_cmd    = 1
    dataByte[0] = 0x1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    // U-reset I2C IP
    // self.csr.misc.hw_rst = 0x0
    dataWord[0] = 0x0;
    dataWord[1] = 0x0;
    rc = ariesWriteBlockData(i2cDriver, ARIES_HW_RST_ADDR, 2, dataWord);
    CHECK_SUCCESS(rc);

    // Set IC_FS_SCL_HCNT
    // self.csr.muc.al_main_ext_csr.i2c_mst_addr   = 0x1c
    dataByte[0] = 0x1c;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_IC_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    // self.csr.muc.al_main_ext_csr.i2c_mst_data_0 = new_fs_scl_hcnt & 0xff
    dataByte[0] = newSclHighCnt & 0xff;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    // self.csr.muc.al_main_ext_csr.i2c_mst_data_1 = (new_fs_scl_hcnt >> 8) & 0xff
    dataByte[0] = (newSclHighCnt >> 8) & 0xff;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA1_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    // self.csr.muc.al_main_ext_csr.i2c_mst_cmd    = 1
    dataByte[0] = 0x1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    // Set IC_FS_SCL_LCNT
    // self.csr.muc.al_main_ext_csr.i2c_mst_addr   = 0x20
    dataByte[0] = 0x20;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_IC_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    // self.csr.muc.al_main_ext_csr.i2c_mst_data_0 = new_fs_scl_lcnt & 0xff
    dataByte[0] = newSclLowCnt & 0xff;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    // self.csr.muc.al_main_ext_csr.i2c_mst_data_1 = (new_fs_scl_lcnt >> 8) & 0xff
    dataByte[0] = (newSclLowCnt >> 8) & 0xff;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA1_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    // self.csr.muc.al_main_ext_csr.i2c_mst_cmd    = 1
    dataByte[0] = 0x1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    // Set IC_ENABLE=1
    // self.csr.muc.al_main_ext_csr.i2c_mst_addr   = 0x6c
    dataByte[0] = 0x6c;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_IC_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    // self.csr.muc.al_main_ext_csr.i2c_mst_data_0 = 1
    dataByte[0] = 0x1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    // self.csr.muc.al_main_ext_csr.i2c_mst_cmd    = 1
    dataByte[0] = 0x1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

/*
 * Get the index of the eeprom sequence - which is defined by the
 * following pattern - a55aa55ff00000000ffff in the hex file
 */
int ariesGetEEPROMImageEnd(
        uint8_t* data)
{
    int dataIdx = 0;
    int seqIdx = 0;
    int location;

    uint8_t sequence[11] = {0xa5, 0x5a, 0xa5, 0x5a, 0xff, 0x0, 0x0, 0x0, 0x0,
        0xff, 0xff};
    int seqLen = 11;

    while ((dataIdx < ARIES_EEPROM_NUM_BYTES) && (seqIdx < seqLen))
    {
        // Increment both pointers if element matches
        if (data[dataIdx] == sequence[seqIdx])
        {
            dataIdx++;
            seqIdx++;
            // Check if sequence is fully traversed
            if (seqIdx == seqLen)
            {
                location = dataIdx;
                return location;
            }
        }
        else
        {
            dataIdx = dataIdx - seqIdx + 1;
            seqIdx = 0;
        }
    }
    return -1;
}

/*
 * Write multiple bytes to the I2C Master via Main Micro
 */
AriesErrorType ariesI2CMasterMultiBlockWrite(
        AriesI2CDriverType* i2cDriver,
        uint16_t address,
        int numBytes,
        uint8_t* values)
{
    uint8_t dataByte[1];
    uint8_t dataBytes1[4];
    uint8_t dataBytes2[4];
    uint8_t dataBytes3[4];
    uint8_t dataBytes4[4];
    uint8_t addr15To8;
    uint8_t addr7To0;
    AriesErrorType rc;

    // IC Data command
    dataByte[0] = 0x10;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_IC_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    // Prepare Flag Byte
    dataByte[0] = 0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA1_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    // Send Address
    addr15To8 = (address >> 8) & 0xff;
    addr7To0 = address & 0xff;
    dataByte[0] = addr15To8;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = addr7To0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    int numIters = numBytes/ARIES_EEPROM_BLOCK_WRITE_SIZE;
    int iterIdx;
    int dataIdx;

    int count = 0;
    uint8_t cmd;
    int try;
    int numTries = 30;
    bool mmBusy = false;
    for (iterIdx = 0; iterIdx < numIters; iterIdx++)
    {
        for (dataIdx = 0; dataIdx < 4; dataIdx++)
        {
            dataBytes1[dataIdx] = values[(count+dataIdx)];
            dataBytes2[dataIdx] = values[(count+dataIdx+4)];
            dataBytes3[dataIdx] = values[(count+dataIdx+8)];
            dataBytes4[dataIdx] = values[(count+dataIdx+12)];
        }

        cmd = 1;
        if (iterIdx == (numIters-1))
        {
            cmd = 2;
        }
        // Write data
        rc = ariesWriteBlockData(i2cDriver, ARIES_MM_EEPROM_ASSIST_DATA_ADDR,
                4, dataBytes1);
        CHECK_SUCCESS(rc);
        rc = ariesWriteBlockData(i2cDriver,
                (ARIES_MM_EEPROM_ASSIST_DATA_ADDR+4), 4, dataBytes2);
        CHECK_SUCCESS(rc);
        rc = ariesWriteBlockData(i2cDriver,
                (ARIES_MM_EEPROM_ASSIST_DATA_ADDR+8), 4, dataBytes3);
        CHECK_SUCCESS(rc);
        rc = ariesWriteBlockData(i2cDriver,
                (ARIES_MM_EEPROM_ASSIST_DATA_ADDR+12), 4, dataBytes4);
        CHECK_SUCCESS(rc);

        // Write cmd
        dataByte[0] = cmd;
        rc = ariesWriteByteData(i2cDriver, ARIES_MM_EEPROM_ASSIST_CMD_ADDR,
                dataByte);
        CHECK_SUCCESS(rc);

        rc = ariesReadByteData(i2cDriver, ARIES_MM_EEPROM_ASSIST_CMD_ADDR,
                dataByte);
        CHECK_SUCCESS(rc);

        // Verify Command returned back to zero
        mmBusy = true;
        for (try = 0; try < numTries; try++)
        {
            rc = ariesReadByteData(i2cDriver, ARIES_MM_EEPROM_ASSIST_CMD_ADDR,
                    dataByte);
            CHECK_SUCCESS(rc);
            if (dataByte[0] == 0)
            {
                mmBusy = false;
                break;
            }
            usleep(ARIES_MM_STATUS_TIME);
        }

        // If status not reset to 0, return BUSY error
        if (mmBusy)
        {
            ASTERA_TRACE("ERROR: Main Micro busy. Did not commit write");
            return ARIES_EEPROM_MM_STATUS_BUSY;
        }

        count += ARIES_EEPROM_BLOCK_WRITE_SIZE;
    }

    return ARIES_SUCCESS;
}

/*
 * Re-write data byte and verify
 */
AriesErrorType ariesI2CMasterRewriteAndVerifyByte(
        AriesI2CDriverType* i2cDriver,
        int address,
        uint8_t* value)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    uint8_t readByte[1];

    // Write byte
    rc = ariesI2CMasterSendByteBlockData(i2cDriver, address, 1, value);
    CHECK_SUCCESS(rc);
    usleep(ARIES_I2C_MASTER_WRITE_DELAY);

    rc = ariesI2CMasterSendAddress(i2cDriver, address);
    CHECK_SUCCESS(rc);

    // Read byte
    dataByte[0] = 0x3;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA1_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 0x1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    usleep(ARIES_I2C_MASTER_CMD_RST);
    dataByte[0] = 0x0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    rc = ariesReadByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, readByte);
    CHECK_SUCCESS(rc);

    if (value[0] == readByte[0])
    {
        ASTERA_INFO("        Re-write succeeded");
        rc = ARIES_SUCCESS;
    }
    else
    {
        ASTERA_INFO("        Re-write failed. Expected %d but got %d", value[0],
                readByte[0]);
        rc = ARIES_EEPROM_VERIFY_FAILURE;
    }

    return rc;
}


/*
 * Send eeprom address to the I2C Bus
 */
AriesErrorType ariesI2CMasterSendAddress(
        AriesI2CDriverType* i2cDriver,
        int address)
{
    uint8_t dataByte[1];
    uint8_t addr15To8;
    uint8_t addr7To0;
    AriesErrorType rc;

    dataByte[0] = 0x10;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_IC_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    // Prepare Flag Byte
    dataByte[0] = 0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA1_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    // Program address
    addr15To8 = (address >> 8) & 0xff;
    addr7To0 = address & 0xff;
    dataByte[0] = addr15To8;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = addr7To0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Send a block of data to the I2C Bus
 */
AriesErrorType ariesI2CMasterSendByteBlockData(
        AriesI2CDriverType* i2cDriver,
        int address,
        int numBytes,
        uint8_t* value)
{
    // Write CSR Address
    uint8_t dataByte[1];
    uint8_t addr15To8;
    uint8_t addr7To0;
    int byteIndex = 0;
    AriesErrorType rc;

    dataByte[0] = 0x10;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_IC_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    // Prepare Flag Byte
    dataByte[0] = 0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA1_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    addr15To8 = (address >> 8) & 0xff;
    addr7To0 = address & 0xff;
    dataByte[0] = addr15To8;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = addr7To0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    // Write all data bytes except last
    for (byteIndex = 0; byteIndex < (numBytes-1); byteIndex++)
    {
        dataByte[0] = value[byteIndex];
        rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
        CHECK_SUCCESS(rc);
        dataByte[0] = 1;
        rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
        CHECK_SUCCESS(rc);
    }

    // Write last byte
    dataByte[0] = 2;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA1_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = value[(numBytes-1)];
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

/*
 * Send byte to I2C Bus
 */
AriesErrorType ariesI2CMasterSendByte(
        AriesI2CDriverType* i2cDriver,
        uint8_t* value,
        int flag)
{
    uint8_t dataByte[1];
    AriesErrorType rc;

    dataByte[0] = 0x10;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_IC_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, value);
    CHECK_SUCCESS(rc);
    dataByte[0] = flag << 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA1_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 0x1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Receive a block of bytes (16 bytes) from EEPROM via I2c Master
 * Terminate at end of transaction
 */
AriesErrorType ariesI2CMasterReceiveByteBlock(
        AriesI2CDriverType* i2cDriver,
        uint8_t* dataBytes)
{
    uint8_t data[4];
    uint8_t dataByte[1];
    AriesErrorType rc;
    bool mmBusy;

    dataByte[0] = 3;
    rc = ariesWriteByteData(i2cDriver, ARIES_MM_EEPROM_ASSIST_CMD_ADDR,
            dataByte);
    CHECK_SUCCESS(rc);

    usleep(ARIES_MM_READ_CMD_WAIT);

    int numTries = 30;
    int tryIndex;
    mmBusy = true;
    for (tryIndex = 0; tryIndex < numTries; tryIndex++)
    {
        rc = ariesReadByteData(i2cDriver, ARIES_MM_EEPROM_ASSIST_CMD_ADDR,
                dataByte);
        CHECK_SUCCESS(rc);
        if (dataByte[0] == 0)
        {
            mmBusy = false;
            break;
        }
        usleep(ARIES_MM_STATUS_TIME);
    }

    // If status not reset to 0, return BUSY error
    if (mmBusy)
    {
        ASTERA_ERROR("ERROR: Main Micro busy. Read data not ready");
        return ARIES_EEPROM_MM_STATUS_BUSY;
    }

    int byteIdx;
    int dataByteIdx;
    for (byteIdx = 0; byteIdx < ARIES_EEPROM_BLOCK_WRITE_SIZE; byteIdx+=4)
    {
        rc = ariesReadBlockData(i2cDriver,
                (ARIES_MM_EEPROM_ASSIST_DATA_ADDR+byteIdx), 4, data);
        CHECK_SUCCESS(rc);
        for (dataByteIdx = 0; dataByteIdx < 4; dataByteIdx++)
        {
            dataBytes[(byteIdx+dataByteIdx)] = data[dataByteIdx];
        }
    }
    return ARIES_SUCCESS;
}


/*
 * Get checksum of current page (all bytes)
 */
AriesErrorType ariesI2CMasterGetChecksum(
        AriesI2CDriverType* i2cDriver,
        uint32_t* checksum)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    uint8_t dataBytes[4];
    uint8_t try;
    uint8_t byteIdx;
    uint8_t dataByteIdx;
    uint8_t numTries = 100;

    bool mmBusy = true;

    dataByte[0] = ARIES_MM_EEPROM_CHECKSUM_CODE;
    rc = ariesWriteByteData(i2cDriver, ARIES_MM_EEPROM_ASSIST_CMD_ADDR,
            dataByte);
    CHECK_SUCCESS(rc);

    sleep(ARIES_MM_CALC_CHECKSUM_WAIT);

    for (try = 0; try < numTries; try++)
    {
        rc = ariesReadByteData(i2cDriver, ARIES_MM_EEPROM_ASSIST_CMD_ADDR,
                dataByte);
        CHECK_SUCCESS(rc);
        if (dataByte[0] == 0)
        {
            mmBusy = false;
            break;
        }
        usleep(ARIES_MM_CALC_CHECKSUM_TRY_TIME);
    }

    if (mmBusy)
    {
        ASTERA_ERROR("ERROR: Main Micro busy. Read data not ready");
        return ARIES_EEPROM_MM_STATUS_BUSY;
    }

    // Compute checksum
    *checksum = 0;

    for (byteIdx = 0; byteIdx < ARIES_EEPROM_BLOCK_CHECKSUM_WRITE_SIZE; byteIdx+=4)
    {
        rc = ariesReadBlockData(i2cDriver,
                (ARIES_MM_EEPROM_ASSIST_DATA_ADDR+byteIdx), 4, dataBytes);
        CHECK_SUCCESS(rc);
        for (dataByteIdx = 0; dataByteIdx < 4; dataByteIdx++)
        {
            *checksum |= (dataBytes[dataByteIdx] << (8*(dataByteIdx + byteIdx)));
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Get checksum of current page (only partially, until "blockEnd" pointer)
 */
AriesErrorType ariesI2CMasterGetChecksumPartial(
        AriesI2CDriverType* i2cDriver,
        uint16_t blockEnd,
        uint32_t* checksum)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    uint8_t dataBytes[4];
    uint8_t try;
    uint8_t byteIdx;
    uint8_t dataByteIdx;
    uint8_t numTries = 100;

    // Set up end index
    dataBytes[0] = blockEnd & 0xff;
    dataBytes[1] = (blockEnd >> 8) & 0xff;
    dataBytes[2] = 0;
    dataBytes[3] = 0;
    rc = ariesWriteBlockData(i2cDriver, ARIES_MM_EEPROM_ASSIST_DATA_ADDR,
        4, dataBytes);
    CHECK_SUCCESS(rc);

    usleep(1000);

    bool mmBusy = true;
    dataByte[0] = ARIES_MM_EEPROM_CHECKSUM_PARTIAL_CODE;
    rc = ariesWriteByteData(i2cDriver, ARIES_MM_EEPROM_ASSIST_CMD_ADDR,
            dataByte);
    CHECK_SUCCESS(rc);

    sleep(ARIES_MM_CALC_CHECKSUM_WAIT);

    for (try = 0; try < numTries; try++)
    {
        rc = ariesReadByteData(i2cDriver, ARIES_MM_EEPROM_ASSIST_CMD_ADDR,
                dataByte);
        CHECK_SUCCESS(rc);
        if (dataByte[0] == 0)
        {
            mmBusy = false;
            break;
        }
        usleep(ARIES_MM_CALC_CHECKSUM_TRY_TIME);
    }

    if (mmBusy)
    {
        ASTERA_ERROR("ERROR: Main Micro busy. Read data not ready");
        return ARIES_EEPROM_MM_STATUS_BUSY;
    }

    // Compute checksum
    *checksum = 0;

    for (byteIdx = 0; byteIdx < ARIES_EEPROM_BLOCK_CHECKSUM_WRITE_SIZE; byteIdx+=4)
    {
        rc = ariesReadBlockData(i2cDriver,
                (ARIES_MM_EEPROM_ASSIST_DATA_ADDR+byteIdx), 4, dataBytes);
        CHECK_SUCCESS(rc);
        for (dataByteIdx = 0; dataByteIdx < 4; dataByteIdx++)
        {
            *checksum |= (dataBytes[dataByteIdx] << (8*(dataByteIdx + byteIdx)));
        }
    }

    return ARIES_SUCCESS;
}


/*
 * Receive a block of bytes (16 bytes) from EEPROM via I2c Master
 * Do not terminate at end of transaction
 */
AriesErrorType ariesI2CMasterReceiveContinuousByteBlock(
        AriesI2CDriverType* i2cDriver,
        uint8_t* dataBytes)
{
    uint8_t data[4];
    uint8_t dataByte[1];
    AriesErrorType rc;
    bool mmBusy;

    dataByte[0] = 4;
    rc = ariesWriteByteData(i2cDriver, ARIES_MM_EEPROM_ASSIST_CMD_ADDR,
            dataByte);
    CHECK_SUCCESS(rc);

    usleep(ARIES_MM_READ_CMD_WAIT);

    int numTries = 3;
    int tryIndex;
    mmBusy = true;
    for (tryIndex = 0; tryIndex < numTries; tryIndex++)
    {
        rc = ariesReadByteData(i2cDriver, ARIES_MM_EEPROM_ASSIST_CMD_ADDR,
                dataByte);
        CHECK_SUCCESS(rc);
        if (dataByte[0] == 0)
        {
            mmBusy = false;
            break;
        }
        usleep(ARIES_MM_STATUS_TIME);
    }

    // If status not reset to 0, return BUSY error
    if (mmBusy)
    {
        ASTERA_TRACE("ERROR: Main Micro busy. Read data not ready");
        return ARIES_EEPROM_MM_STATUS_BUSY;
    }

    int byteIdx;
    int dataByteIdx;
    for (byteIdx = 0; byteIdx < ARIES_EEPROM_BLOCK_WRITE_SIZE; byteIdx+=4)
    {
        rc = ariesReadBlockData(i2cDriver,
                (ARIES_MM_EEPROM_ASSIST_DATA_ADDR+byteIdx), 4, data);
        CHECK_SUCCESS(rc);
        for (dataByteIdx = 0; dataByteIdx < 4; dataByteIdx++)
        {
            dataBytes[(byteIdx+dataByteIdx)] = data[dataByteIdx];
        }
    }
    return ARIES_SUCCESS;
}

/*
 * Receive a single byte from the I2C bus
 */
AriesErrorType ariesI2CMasterReceiveByte(
        AriesI2CDriverType* i2cDriver,
        uint8_t* value)
{
    uint8_t dataByte[1];
    AriesErrorType rc;

    dataByte[0] = 0x10;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_IC_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 0x3;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA1_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 0x1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    usleep(ARIES_I2C_MASTER_CMD_RST);
    dataByte[0] = 0x0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    rc = ariesReadByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, value);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

/*
 * Receive a continuous stream of bytes from the I2C bus
 */
AriesErrorType ariesI2CMasterReceiveContinuousByte(
        AriesI2CDriverType* i2cDriver,
        uint8_t* value)
{
    uint8_t dataByte[1];
    AriesErrorType rc;

    dataByte[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    dataByte[0] = 0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    rc = ariesReadByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, value);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

/*
 * Set Page address in I2C Master
 */
AriesErrorType ariesI2CMasterSetPage(
        AriesI2CDriverType* i2cDriver,
        int page)
{
    uint8_t tar[1];
    AriesErrorType rc;

    // Power-on default value is 0x50
    //rc = ariesReadByteData(i2cDriver, 0xd0c, tar);
    //CHECK_SUCCESS(rc);
    tar[0] = 0x50 | (page & 3);

    uint8_t dataByte[1];

    dataByte[0] = 0;
    rc = ariesI2CMasterWriteCtrlReg(i2cDriver, 0x6c, 1, dataByte);
    CHECK_SUCCESS(rc);
    rc = ariesI2CMasterWriteCtrlReg(i2cDriver, 0x04, 1, tar);
    CHECK_SUCCESS(rc);
    dataByte[0] = 1;
    rc = ariesI2CMasterWriteCtrlReg(i2cDriver, 0x6c, 1, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

/*
 * De-assert HW and SW resets
 */
AriesErrorType ariesDeassertReset(
        AriesI2CDriverType* i2cDriver)
{
    AriesErrorType rc;
    uint8_t hwRstR[2];
    uint8_t hwRstW[2];
    uint8_t swRstR[2];
    uint8_t swRstW[2];

    // De-assert HW Reset
    rc = ariesReadBlockData(i2cDriver, 0x600, 2, hwRstR);
    CHECK_SUCCESS(rc);
    hwRstW[0] = hwRstR[0] & 0xff;
    hwRstW[1] = hwRstR[1] & 0xD;
    rc = ariesWriteBlockData(i2cDriver, 0x600, 2, hwRstW);
    CHECK_SUCCESS(rc);

    // De-assert SW Reset
    rc = ariesReadBlockData(i2cDriver, 0x602, 2, swRstR);
    CHECK_SUCCESS(rc);
    swRstW[0] = swRstR[0] & 0xff;
    swRstW[1] = swRstR[1] & 0xD;
    rc = ariesWriteBlockData(i2cDriver, 0x602, 2, swRstW);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

/*
 * Get temp caliberation codes and add to device settings
 */
AriesErrorType ariesGetTempCalibrationCodes(
        AriesDeviceType* device)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    uint8_t dataWord[2];
    uint8_t dataBytes5[5];
    uint8_t invalid;
    uint8_t flag;
    uint8_t offset;
    uint8_t calCode;

    // 1. Set up TCK
    // temp_val = astera_reg_read(0x8ec, 5)
    rc = ariesReadBlockData(device->i2cDriver, 0x8ec, 5, dataBytes5);
    CHECK_SUCCESS(rc);
    // Assert bit 25
    dataBytes5[3] |= (1<<1);
    // Write it back to reg
    // astera_reg_write(0x8ec, 5, temp_val | (1<<25)) # Assert bit 25
    rc = ariesWriteBlockData(device->i2cDriver, 0x8ec, 5, dataBytes5);
    CHECK_SUCCESS(rc);

    // 2. Toggle SMS reset
    // astera_reg_write(0x600, 2, 0x800)
    dataWord[0] = 0x0;
    dataWord[1] = 0x8;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, dataWord);
    CHECK_SUCCESS(rc);
    // astera_reg_write(0x602, 2, 0x800)
    dataWord[0] = 0x0;
    dataWord[1] = 0x8;
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, dataWord);
    CHECK_SUCCESS(rc);
    // astera_reg_write(0x600, 2, 0x0)
    dataWord[0] = 0x0;
    dataWord[1] = 0x0;
    rc = ariesWriteBlockData(device->i2cDriver, 0x600, 2, dataWord);
    CHECK_SUCCESS(rc);
    // astera_reg_write(0x602, 2, 0x0)
    dataWord[0] = 0x0;
    dataWord[1] = 0x0;
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, dataWord);
    CHECK_SUCCESS(rc);

    // 3. Assert efuse_load
    // temp_val = astera_reg_read(0x8f6, 1)
    rc = ariesReadByteData(device->i2cDriver, 0x8f6, dataByte);
    CHECK_SUCCESS(rc);
    // astera_reg_write(0x8f6, 1, temp_val | (1<<7)) # Assert bit 7
    dataByte[0] |= (1<<7);
    rc = ariesWriteByteData(device->i2cDriver, 0x8f6, dataByte);

    // 4. Assert smart_mode
    // temp_val = astera_reg_read(0x8ec, 5)
    rc = ariesReadBlockData(device->i2cDriver, 0x8ec, 5, dataBytes5);
    CHECK_SUCCESS(rc);
    // Assert bit 24
    dataBytes5[3] |= (1<<0);
    // Write it back to reg
    rc = ariesWriteBlockData(device->i2cDriver, 0x8ec, 5, dataBytes5);
    CHECK_SUCCESS(rc);

    // 5. De-assert smart_mode
    // temp_val = astera_reg_read(0x8ec, 5)
    rc = ariesReadBlockData(device->i2cDriver, 0x8ec, 5, dataBytes5);
    CHECK_SUCCESS(rc);
    // De-assert bit 24
    dataBytes5[3] &= ~(1<<0);
    // Write it back to reg
    rc = ariesWriteBlockData(device->i2cDriver, 0x8ec, 5, dataBytes5);
    CHECK_SUCCESS(rc);

    // 6. De-assert efuse_load
    // temp_val = astera_reg_read(0x8f6, 1)
    rc = ariesReadByteData(device->i2cDriver, 0x8f6, dataByte);
    CHECK_SUCCESS(rc);
    // astera_reg_write(0x8f6, 1, temp_val ^ (1<<7)) # De-assert bit 7
    dataByte[0] &= ~(1<<7);
    rc = ariesWriteByteData(device->i2cDriver, 0x8f6, dataByte);
    CHECK_SUCCESS(rc);

    // 7. Read eFuse “primary page invalid” bit and adjust offset accordingly
    // astera_reg_write(0x8f6, 1, 63) # Set address
    dataByte[0] = 63;
    rc = ariesWriteByteData(device->i2cDriver, 0x8f6, dataByte);
    CHECK_SUCCESS(rc);
    // invalid = astera_reg_read(0x8f7, 1) # Read data
    rc = ariesReadByteData(device->i2cDriver, 0x8f7, dataByte);
    CHECK_SUCCESS(rc);
    invalid = dataByte[0];

    if (invalid & 0x80)
    {
        offset = 64;
    }
    else
    {
        offset = 0;
    }

    // 8. Determine calibration codes
    // astera_reg_write(0x8f6, 1, 48 + offset) # Set address
    dataByte[0] = 48 + offset;
    rc = ariesWriteByteData(device->i2cDriver, 0x8f6, dataByte);
    CHECK_SUCCESS(rc);
    // flag = astera_reg_read(0x8f7, 1) # Read data
    rc = ariesReadByteData(device->i2cDriver, 0x8f7, dataByte);
    CHECK_SUCCESS(rc);
    flag = dataByte[0];

    // Compute PMA A calibration codes
    int qs;
    for (qs = 0; qs < 4; qs++)
    {
        if (flag & 0x4)
        {
            // astera_reg_write(0x8f6, 1, 34 + qs*4 + offset) # Set address
            dataByte[0] = 34 + (qs*4) + offset;
            rc = ariesWriteByteData(device->i2cDriver, 0x8f6, dataByte);
            CHECK_SUCCESS(rc);
            rc = ariesReadByteData(device->i2cDriver, 0x8f7, dataByte);
            CHECK_SUCCESS(rc);
            calCode = dataByte[0];
            if (calCode == 0)
            {
                calCode = 84;
            }
        }
        else
        {
            calCode = 84;
        }
        device->tempCalCodePmaA[qs] = calCode;
    }

    // Compute PMA B calibration codes
    for (qs = 0; qs < 4; qs++)
    {
        if (flag & 0x04)
        {
            dataByte[0] = 32 + (qs*4) + offset;
            rc = ariesWriteByteData(device->i2cDriver, 0x8f6, dataByte);
            CHECK_SUCCESS(rc);
            rc = ariesReadByteData(device->i2cDriver, 0x8f7, dataByte);
            CHECK_SUCCESS(rc);
            calCode = dataByte[0];
            if (calCode == 0)
            {
                calCode = 84;
            }
        }
        else
        {
            calCode = 84;
        }
        device->tempCalCodePmaB[qs] = calCode;
    }

    // Calcualte the average PMA calibration code
    if (device->partNumber == ARIES_PTX16)
    {
        device->tempCalCodeAvg = (device->tempCalCodePmaA[0]
            + device->tempCalCodePmaA[1]
            + device->tempCalCodePmaA[2]
            + device->tempCalCodePmaA[3]
            + device->tempCalCodePmaB[0]
            + device->tempCalCodePmaB[1]
            + device->tempCalCodePmaB[2]
            + device->tempCalCodePmaB[3]
            + 8/2) / 8; // Add denominator/2 to cause integer rounding
    }
    else if (device->partNumber == ARIES_PTX08)
    {
        device->tempCalCodeAvg = (device->tempCalCodePmaA[1]
            + device->tempCalCodePmaA[2]
            + device->tempCalCodePmaB[1]
            + device->tempCalCodePmaB[2]
            + 4/2) / 4; // Add denominator/2 to cause integer rounding
    }
    else
    {
        return ARIES_INVALID_ARGUMENT;
    }

    // Read 12-byte chip ID
    int b = 0;
    for (b = 0; b < 12; b += 1)
    {
        dataByte[0] = 0 + b + offset; // Chip ID starts at byte 0 in eFuse
        rc = ariesWriteByteData(device->i2cDriver, 0x8f6, dataByte);
        CHECK_SUCCESS(rc);
        rc = ariesReadByteData(device->i2cDriver, 0x8f7, dataByte);
        CHECK_SUCCESS(rc);
        device->chipID[b] = dataByte[0];
    }

    // Read 6-byte lot number
    for (b = 0; b < 6; b += 1)
    {
        dataByte[0] = 16 + b + offset; // Lot number starts at byte 16 in eFuse
        rc = ariesWriteByteData(device->i2cDriver, 0x8f6, dataByte);
        CHECK_SUCCESS(rc);
        rc = ariesReadByteData(device->i2cDriver, 0x8f7, dataByte);
        CHECK_SUCCESS(rc);
        device->lotNumber[b] = dataByte[0];
    }

    return ARIES_SUCCESS;
}


/*
 * Read the "all-time maximum temperature" register, at reg 0x424 (read 2 bytes)
 * This is only after FW version 1.0.42
 */
AriesErrorType ariesReadPmaTempMax(
        AriesDeviceType* device)
{
    uint8_t dataBytes[4];
    int adcCode;
    AriesErrorType rc;

    rc = ariesReadBlockData(device->i2cDriver, ARIES_MAX_TEMP_ADC_CSR, 4,
        dataBytes);
    CHECK_SUCCESS(rc);
    adcCode = (dataBytes[3]<<24) + (dataBytes[2]<<16) + (dataBytes[1]<<8)
        + dataBytes[0];

    device->maxTempC = 110 + ((adcCode - (device->tempCalCodeAvg + 250)) * -0.32);

    return ARIES_SUCCESS;
}


/*
 * Read temp from a PMA reg which computes avg temp across all temp sensors
 * Max temp (ADC code) is available in a reg at 0x42c (read 2 bytes)
 * This is only aftre FW version 1.0.42
 */
AriesErrorType ariesReadPmaAvgTemp(
        AriesDeviceType* device)
{
    uint8_t dataBytes[4];
    int adcCode;
    AriesErrorType rc;

    rc = ariesReadBlockData(device->i2cDriver, ARIES_CURRENT_TEMP_ADC_CSR, 4,
        dataBytes);
    CHECK_SUCCESS(rc);
    adcCode = (dataBytes[3]<<24) + (dataBytes[2]<<16) + (dataBytes[1]<<8)
        + dataBytes[0];

    device->currentTempC = 110 + ((adcCode - (device->tempCalCodeAvg + 250)) * -0.32);

    return ARIES_SUCCESS;
}


/*
 * Enable thermal shutdown in Aries
 */
AriesErrorType ariesEnableThermalShutdown(
        AriesDeviceType* device)
{
    AriesErrorType rc;
    uint8_t dataBytes[4] = {0,0,0,1};
    rc = ariesWriteBlockData(device->i2cDriver, ARIES_EN_THERMAL_SHUTDOWN, 4,
        dataBytes);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Disable thermal shutdown in Aries
 */
AriesErrorType ariesDisableThermalShutdown(
        AriesDeviceType* device)
{
    AriesErrorType rc;
    uint8_t dataBytes[4] = {0,0,0,0};
    rc = ariesWriteBlockData(device->i2cDriver, ARIES_EN_THERMAL_SHUTDOWN, 4,
        dataBytes);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Read temp from a particular PMA directly
 * PMA code is available in a FW reg in the MM space (read 2 bytes)
 * This is only aftre FW version 1.0.42
 */
AriesErrorType ariesReadPmaTemp(
        AriesDeviceType* device,
        int side,
        int qs,
        float *temperature_C)
{
    uint8_t dataWord[2];
    AriesErrorType rc;

    uint32_t pmaCsr = ARIES_MAIN_MICRO_FW_INFO +
        ARIES_MM_PMA_TJ_ADC_CODE_OFFSET;
    uint16_t adcCode;
    uint8_t pmaTempCode;

    rc = ariesReadBlockDataMainMicroIndirect(device->i2cDriver,
            (pmaCsr+(side*8)+(qs*2)), 2, dataWord);
    CHECK_SUCCESS(rc);

    adcCode = (dataWord[1]<<8) + dataWord[0];

    // PMA side not very important since values are quite similar
    if (side == 1)
    {
        pmaTempCode = device->tempCalCodePmaB[qs];
    }
    else
    {
        pmaTempCode = device->tempCalCodePmaA[qs];
    }

    *temperature_C = 110 + ((adcCode - (pmaTempCode + 250)) * -0.32);

    return ARIES_SUCCESS;
}


/*
 * Get port orientation
 */
AriesErrorType ariesGetPortOrientation(
        AriesDeviceType* device,
        int* orientation)
{
    AriesErrorType rc;
    uint8_t regVal[4];
    rc = ariesReadBlockData(device->i2cDriver, 0x10, 4, regVal);
    CHECK_SUCCESS(rc);

    // Oriention Val is bit 8
    *orientation = regVal[1] & (0x01);

    return ARIES_SUCCESS;
}

/*
 * Set port orientation
 */
AriesErrorType ariesSetPortOrientation(
        AriesDeviceType* device,
        uint8_t orientation)
{
    AriesErrorType rc;
    uint8_t dataBytes[4];

    rc = ariesReadBlockData(device->i2cDriver, 0x10, 4, dataBytes);
    CHECK_SUCCESS(rc);

    // Orientation is bit 8 in this reg
    dataBytes[1] &= 0xfe;
    dataBytes[1] |= (orientation & 0x1);

    rc = ariesWriteBlockData(device->i2cDriver, 0x10, 4, dataBytes);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


/*
 * Get PMA number (absolute)
 */
int ariesGetPmaNumber(
        int absLane)
{
    int pmaNum;
    pmaNum = (absLane/4);
    return pmaNum;
}

/*
 * Get PMA lane number
 */
int ariesGetPmaLane(
        int absLane)
{
    int pmaLane;
    pmaLane = (absLane%4);
    return pmaLane;
}

/*
 * Get Path ID
 */
int ariesGetPathID(
        int lane,
        int direction)
{
    return (((lane/2)*2) + direction);
}

/*
 * Get Path Lane ID
 */
int ariesGetPathLaneID(
        int lane)
{
    return (lane%2);
}

int ariesGetStartLane(
        AriesLinkType* link)
{
    int startLane = link->config.startLane;
    if (link->config.partNumber == ARIES_PTX08)
    {
        startLane += 4;
    }

    return startLane;
}

/*
 * Get QS and relative Path from lane number
 */
void ariesGetQSPathInfo(
        int lane,
        int direction,
        int* qs,
        int* qsPath,
        int* qsPathLane)
{
    int pathID = ariesGetPathID(lane, direction);
    *qs = pathID/4;
    *qsPath = pathID%4;
    *qsPathLane = lane%2;
}

/*
 * Get link RX term
 */
AriesErrorType ariesGetLinkRxTerm(
        AriesLinkType* link,
        int side,
        int absLane,
        int* term)
{
    AriesErrorType rc;
    uint8_t dataWord[2];
    AriesI2CDriverType *i2cDriver;
    i2cDriver = link->device->i2cDriver;

    int pmaNum = ariesGetPmaNumber(absLane);
    int pmaLane = ariesGetPmaLane(absLane);

    // Read from PMAX_LANE0_DIG_ASIC_RX_OVRD_IN_3
    rc = ariesReadWordPmaLaneMainMicroIndirect(i2cDriver, side, pmaNum, pmaLane,
            ARIES_PMA_LANE_DIG_ASIC_RX_OVRD_IN_3, dataWord);
    CHECK_SUCCESS(rc);

    // TERM_EN_OVRD_EN is bit 7
    uint8_t termEnOverrideEn = (dataWord[0] & 0x80) >> 7;
    // TERM_EN is bit 6
    uint8_t termEn = (dataWord[0] & 0x40) >> 6;

    if (termEnOverrideEn == 1)
    {
        *term = termEn;
    }
    else
    {
        // Read from LANE0_DIG_ASIC_RX_ASIC_IN_1
        rc = ariesReadWordPmaLaneMainMicroIndirect(i2cDriver, side, pmaNum,
                pmaLane, ARIES_PMA_LANE_DIG_ASIC_RX_ASIC_IN_1, dataWord);
        CHECK_SUCCESS(rc);
        // RX_TERM_EN is bit 2
        *term = (dataWord[0] & 0x04) >> 2;
    }

    return ARIES_SUCCESS;
}

/*
 * Get link current speed
 */
AriesErrorType ariesGetLinkCurrentSpeed(
        AriesLinkType* link,
        int lane,
        int direction,
        float *speed)
{
    uint8_t dataByte[1];
    AriesErrorType rc;
    int pcieGen;

    // Based on the lane info, determine QS and Path info
    int qs;
    int qsPath;
    int qsPathLane;
    ariesGetQSPathInfo(lane, direction, &qs, &qsPath, &qsPathLane);

    // Compute reg address
    // Calculate QS, Path, Path lane relative offsets first,
    // and then compute actual address
    int qsOff = ARIES_QS_0_CSR_OFFSET + (qs*ARIES_QS_STRIDE);
    int pathOff = ARIES_PATH_WRAPPER_0_CSR_OFFSET +
        (qsPath*ARIES_PATH_WRP_STRIDE);

    // Compute actual address based on offsets above
    uint32_t address = qsOff + pathOff + ARIES_PATH_GLOBAL_CSR_OFFSET +
        ARIES_GBL_CSR_MAC_PHY_RATE_AND_PCLK_RATE;

    rc = ariesReadByteData(link->device->i2cDriver, address, dataByte);
    CHECK_SUCCESS(rc);

    // rate val is last 3 bits of reg
    pcieGen = (dataByte[0] & 0x07) + 1;

    // Get speed from PCIE gen value
    switch(pcieGen)
    {
        case 1:
            *speed = 2.5;
            break;
        case 2:
            *speed = 5.0;
            break;
        case 3:
            *speed = 8.0;
            break;
        case 4:
            *speed = 16.0;
            break;
        case 5:
            *speed = 32.0;
            break;
        default:
            *speed= 0.0;
    }
    return ARIES_SUCCESS;
}


/*
 * Get link logical lane number
 */
AriesErrorType ariesGetLogicalLaneNum(
        AriesLinkType* link,
        int lane,
        int direction,
        int* laneNum)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    // Based on the lane info, determine QS and Path info
    int qs;
    int qsPath;
    int qsPathLane;
    ariesGetQSPathInfo(lane, direction, &qs, &qsPath, &qsPathLane);

    // Compute reg address
    // Calculate QS, Path, Path lane relative offsets first,
    // and then compute actual address
    int qsOff = ARIES_QS_0_CSR_OFFSET + (qs*ARIES_QS_STRIDE);
    int pathOff = ARIES_PATH_WRAPPER_0_CSR_OFFSET +
        (qsPath*ARIES_PATH_WRP_STRIDE);
    int pathLaneOff = ARIES_PATH_LANE_0_CSR_OFFSET + (qsPathLane*ARIES_PATH_LANE_STRIDE);

    uint32_t address = qsOff + pathOff + pathLaneOff +
        ARIES_LN_CAPT_NUM;

    rc = ariesReadByteData(link->device->i2cDriver, address, dataByte);
    CHECK_SUCCESS(rc);
    *laneNum = dataByte[0];
    return ARIES_SUCCESS;
}

/*
 * Get link TX pre-cursor value
 */
AriesErrorType ariesGetTxPre(
        AriesLinkType* link,
        int lane,
        int direction,
        int* txPre)
{
    uint8_t dataBytes[3];
    AriesErrorType rc;
    int tmpVal;

    // Based on the lane info, determine QS and Path info
    int qs;
    int qsPath;
    int qsPathLane;
    ariesGetQSPathInfo(lane, direction, &qs, &qsPath, &qsPathLane);

    // Compute reg address
    // Calculate QS, Path, Path lane relative offsets first,
    // and then compute actual address
    int qsOff = ARIES_QS_0_CSR_OFFSET + (qs*ARIES_QS_STRIDE);
    int pathOff = ARIES_PATH_WRAPPER_0_CSR_OFFSET +
        (qsPath*ARIES_PATH_WRP_STRIDE);
    int pathLaneOff = ARIES_PATH_LANE_0_CSR_OFFSET + (qsPathLane*ARIES_PATH_LANE_STRIDE);

    // Compute Reg Offset
    uint32_t address = qsOff + pathOff + pathLaneOff +
        ARIES_MAC_PHY_TXDEEMPH_OB;

    rc = ariesReadBlockData(link->device->i2cDriver, address, 3, dataBytes);
    CHECK_SUCCESS(rc);
    tmpVal = (dataBytes[0] + (dataBytes[1] << 8) + (dataBytes[2] << 16));
    *txPre = tmpVal & 0x3f;

    return ARIES_SUCCESS;
}


AriesErrorType ariesGetPathHWState(
        AriesLinkType* link,
        int lane,
        int direction,
        int* state)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    // Based on the lane info, determine QS and Path info
    int qs;
    int qsPath;
    int qsPathLane;
    ariesGetQSPathInfo(lane, direction, &qs, &qsPath, &qsPathLane);

    int qsOff = ARIES_QS_0_CSR_OFFSET + (qs*ARIES_QS_STRIDE);
    int pathOff = ARIES_PATH_WRAPPER_0_CSR_OFFSET +
        (qsPath*ARIES_PATH_WRP_STRIDE);

    uint32_t address = qsOff + pathOff + ARIES_RET_PTH_NEXT_STATE_OFFSET;

    rc = ariesReadBlockData(link->device->i2cDriver, address, 1, dataByte);
    CHECK_SUCCESS(rc);
    *state = dataByte[0];

    return ARIES_SUCCESS;
}

/*
 * Get link TX current cursor value
 */
AriesErrorType ariesGetTxCur(
        AriesLinkType* link,
        int lane,
        int direction,
        int* txCur)
{
    uint8_t dataBytes[3];
    AriesErrorType rc;
    int tmpVal;

    // Based on the lane info, determine QS and Path info
    int qs;
    int qsPath;
    int qsPathLane;
    ariesGetQSPathInfo(lane, direction, &qs, &qsPath, &qsPathLane);

    // Compute reg address
    // Calculate QS, Path, Path lane relative offsets first,
    // and then compute actual address
    int qsOff = ARIES_QS_0_CSR_OFFSET + (qs*ARIES_QS_STRIDE);
    int pathOff = ARIES_PATH_WRAPPER_0_CSR_OFFSET +
        (qsPath*ARIES_PATH_WRP_STRIDE);
    int pathLaneOff = ARIES_PATH_LANE_0_CSR_OFFSET + (qsPathLane*ARIES_PATH_LANE_STRIDE);

    // Compute Reg Offset
    uint32_t address = qsOff + pathOff + pathLaneOff +
        ARIES_MAC_PHY_TXDEEMPH_OB;

    rc = ariesReadBlockData(link->device->i2cDriver, address, 3, dataBytes);
    CHECK_SUCCESS(rc);
    tmpVal = (dataBytes[0] + (dataBytes[1] << 8) + (dataBytes[2] << 16));
    *txCur = (tmpVal & 0xfc0) >> 6;

    return ARIES_SUCCESS;
}

/*
 * Get link TX post cursor value
 */
AriesErrorType ariesGetTxPst(
        AriesLinkType* link,
        int lane,
        int direction,
        int* txPst)
{
    uint8_t dataBytes[3];
    AriesErrorType rc;
    int tmpVal;

    // Based on the lane info, determine QS and Path info
    int qs;
    int qsPath;
    int qsPathLane;
    ariesGetQSPathInfo(lane, direction, &qs, &qsPath, &qsPathLane);

    // Compute reg address
    // Calculate QS, Path, Path lane relative offsets first,
    // and then compute actual address
    int qsOff = ARIES_QS_0_CSR_OFFSET + (qs*ARIES_QS_STRIDE);
    int pathOff = ARIES_PATH_WRAPPER_0_CSR_OFFSET +
        (qsPath*ARIES_PATH_WRP_STRIDE);
    int pathLaneOff = ARIES_PATH_LANE_0_CSR_OFFSET + (qsPathLane*ARIES_PATH_LANE_STRIDE);

    // Compute Reg Offset
    uint32_t address = qsOff + pathOff + pathLaneOff +
        ARIES_MAC_PHY_TXDEEMPH_OB;

    rc = ariesReadBlockData(link->device->i2cDriver, address, 3, dataBytes);
    CHECK_SUCCESS(rc);
    tmpVal = (dataBytes[0] + (dataBytes[1] << 8) + (dataBytes[2] << 16));
    *txPst = (tmpVal & 0x3f000) >> 12;

    return ARIES_SUCCESS;
}

/*
 * Get RX polarity code
 */
AriesErrorType ariesGetRxPolarityCode(
        AriesLinkType* link,
        int lane,
        int direction,
        int pinSet,
        int* polarity)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    // Based on the lane info, determine QS and Path info
    int qs;
    int qsPath;
    int qsPathLane;
    ariesGetQSPathInfo(lane, direction, &qs, &qsPath, &qsPathLane);

    // Compute reg address
    // Calculate QS, Path, Path lane relative offsets first,
    // and then compute actual address
    int qsOff = ARIES_QS_0_CSR_OFFSET + (qs*ARIES_QS_STRIDE);
    int pathOff = ARIES_PATH_WRAPPER_0_CSR_OFFSET +
        (qsPath*ARIES_PATH_WRP_STRIDE);
    int pathLaneOff = ARIES_PATH_LANE_0_CSR_OFFSET +
        (qsPathLane*ARIES_PATH_LANE_STRIDE);

    // Compute actual address based on offsets above
    uint32_t address = qsOff + pathOff + pathLaneOff +
        ARIES_MAC_RX_POLARITY;

    rc = ariesReadByteData(link->device->i2cDriver, address, dataByte);
    CHECK_SUCCESS(rc);
    *polarity = dataByte[0] & 0x1;

    // Check inversion value for rx pins on package
    int inversion;
    if (pinSet == 0)
    {
        inversion = link->device->pins[lane].pinSet1.rxPackageInversion;
    }
    else
    {
        inversion = link->device->pins[lane].pinSet2.rxPackageInversion;
    }
    if ((inversion == 1) && (*polarity == 1))
    {
        *polarity = 0;
    }
    else if ((inversion == 1) && (*polarity == 0))
    {
        *polarity = 1;
    }

    return ARIES_SUCCESS;
}

/*
 * Get current RX attr code
 */
AriesErrorType ariesGetRxAttCode(
        AriesLinkType* link,
        int side,
        int absLane,
        int* code)
{
    int pmaNum = ariesGetPmaNumber(absLane);
    int pmaLane = ariesGetPmaLane(absLane);
    AriesErrorType rc;
    uint8_t dataWord[2];

    rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side, pmaNum, pmaLane,
            ARIES_PMA_LANE_DIG_RX_ADPTCTL_ATT_STATUS, dataWord);
    CHECK_SUCCESS(rc);

    // You need bits 7:0
    *code = dataWord[0] >> 5;
    return ARIES_SUCCESS;
}

/*
 * Get RX CTLE boost
 */
AriesErrorType ariesGetRxCtleBoostCode(
        AriesLinkType* link,
        int side,
        int absLane,
        int* boostCode)
{
    AriesErrorType rc;
    int pmaNum = ariesGetPmaNumber(absLane);
    int pmaLane = ariesGetPmaLane(absLane);
    uint8_t dataWord[2];

    rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side, pmaNum,
            pmaLane, ARIES_PMA_LANE_DIG_RX_ADPTCTL_CTLE_STATUS, dataWord);
    CHECK_SUCCESS(rc);

    // Need bits 9:0
    *boostCode = (((dataWord[1] & 0x03) << 8) + dataWord[0]) >> 5;
    return ARIES_SUCCESS;
}

/*
 * Get RX VGA Code
 */
AriesErrorType ariesGetRxVgaCode(
        AriesLinkType* link,
        int side,
        int absLane,
        int* vgaCode)
{
    AriesErrorType rc;
    int pmaNum = ariesGetPmaNumber(absLane);
    int pmaLane = ariesGetPmaLane(absLane);
    uint8_t dataWord[2];

    rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side, pmaNum,
            pmaLane, ARIES_PMA_LANE_DIG_RX_ADPTCTL_VGA_STATUS, dataWord);
    CHECK_SUCCESS(rc);

    // Need bits 9:0
    *vgaCode = (((dataWord[1] & 0x03) << 8) + dataWord[0]) >> 5;
    return ARIES_SUCCESS;
}

/*
 * Get RX Boost value
 */
float ariesGetRxBoostValueDb(
        int boostCode,
        float attValDb,
        int vgaCode)
{
    float AfeHfGain;
    float AfeLfGain;
    float boostValDb;

    float attVal = 1.5 + attValDb;
    float vgaVal = 0.9 * vgaCode;
    float tmpVal1;
    float tmpVal2;
    float boostVal;

    if (vgaCode <= 6)
    {
        tmpVal1 = 0.9 * (6 - vgaCode);
        if (boostCode <= 10)
        {
            tmpVal2 = 0.65 * boostCode;
        }
        else
        {
            tmpVal2 = 0.65 * 10;
        }
        // Find max between 2 values
        boostVal = tmpVal1;
        if (tmpVal1 < tmpVal2)
        {
            boostVal = tmpVal2;
        }
    }
    else
    {
        if (boostCode <= 10)
        {
            tmpVal2 = 0.65 * boostCode;
        }
        else
        {
            tmpVal2 = 0.65 * 10;
        }
        boostVal = tmpVal2;
    }
    AfeHfGain = attVal + vgaVal + boostVal;

    if (boostCode <= 10)
    {
        boostVal = 0;
    }
    else
    {
        boostVal = -0.65 * (boostCode - 10);
    }
    AfeLfGain = attValDb + vgaVal + boostVal;

    boostValDb = AfeHfGain - AfeLfGain;
    return boostValDb;
}

/*
 * Get the current RX CTLE POLE code
 */
AriesErrorType ariesGetRxCtlePoleCode(
        AriesLinkType* link,
        int side,
        int absLane,
        int* poleCode)
{
    AriesErrorType rc;
    int pmaNum = ariesGetPmaNumber(absLane);
    int pmaLane = ariesGetPmaLane(absLane);
    uint8_t dataWord[2];

    rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side, pmaNum,
            pmaLane, ARIES_PMA_LANE_DIG_RX_ADPTCTL_CTLE_STATUS, dataWord);
    CHECK_SUCCESS(rc);

    // Get bits 11:10
    *poleCode = (dataWord[1] & 0x0c) >> 2;
    return ARIES_SUCCESS;
}

/*
 * Get the current RX DFE code
 */
AriesErrorType ariesGetRxDfeCode(
        AriesLinkType* link,
        int side,
        int absLane,
        int tapNum,
        int* dfeCode)
{
    AriesErrorType rc;
    int pmaNum = ariesGetPmaNumber(absLane);
    int pmaLane = ariesGetPmaLane(absLane);
    uint8_t dataWord[2];
    int address;
    int tapVal;

    switch(tapNum)
    {
        case 1:
            // Get PMA Tap address from a util function
            address = ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP1_STATUS;
            rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side,
                    pmaNum, pmaLane, address, dataWord);
            CHECK_SUCCESS(rc);
            // Get Bits 13:0
            tapVal = (((dataWord[1] & 0x3f) << 8) + dataWord[0]) >> 5;
            *dfeCode = tapVal;
            if (tapVal >= 256)
            {
                // dfe_tap1_code is stored in two's complement format
                *dfeCode = tapVal - 512;
            }
            break;

        case 2:
            // Get PMA Tap address from a util function
            address = ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP2_STATUS;
            rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side,
                    pmaNum, pmaLane, address, dataWord);
            CHECK_SUCCESS(rc);
            // Get bits 12:0
            tapVal = (((dataWord[1] & 0x1f) << 8) + dataWord[0]) >> 5;
            *dfeCode = tapVal - 128;
            break;

        case 3:
            // Get PMA Tap address from a util function
            address = ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP3_STATUS;
            rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side,
                    pmaNum, pmaLane, address, dataWord);
            CHECK_SUCCESS(rc);
            // Get bits 11:0
            tapVal = (((dataWord[1] & 0x0f) << 8) + dataWord[0]) >> 5;
            *dfeCode = tapVal - 64;
            break;

        case 4:
            // Get PMA Tap address from a util function
            address = ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP4_STATUS;
            rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side,
                    pmaNum, pmaLane, address, dataWord);
            CHECK_SUCCESS(rc);
            // Get bits 11:0
            tapVal = (((dataWord[1] & 0x0f) << 8) + dataWord[0]) >> 5;
            *dfeCode = tapVal - 64;
            break;

        case 5:
            // Get PMA Tap address from a util function
            address = ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP5_STATUS;
            rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side,
                    pmaNum, pmaLane, address, dataWord);
            CHECK_SUCCESS(rc);
            // Get bits 11:0
            tapVal = (((dataWord[1] & 0x0f) << 8) + dataWord[0]) >> 5;
            *dfeCode = tapVal - 64;
            break;

        case 6:
            // Get PMA Tap address from a util function
            address = ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP6_STATUS;
            rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side,
                    pmaNum, pmaLane, address, dataWord);
            CHECK_SUCCESS(rc);
            // Get bits 11:0
            tapVal = (((dataWord[1] & 0x0f) << 8) + dataWord[0]) >> 5;
            *dfeCode = tapVal - 64;
            break;

        case 7:
            // Get PMA Tap address from a util function
            address = ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP7_STATUS;
            rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side,
                    pmaNum, pmaLane, address, dataWord);
            CHECK_SUCCESS(rc);
            // Get bits 11:0
            tapVal = (((dataWord[1] & 0x0f) << 8) + dataWord[0]) >> 5;
            *dfeCode = tapVal - 64;
            break;

        case 8:
            // Get PMA Tap address from a util function
            address = ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP8_STATUS;
            rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side,
                    pmaNum, pmaLane, address, dataWord);
            CHECK_SUCCESS(rc);
            // Get bits 11:0
            tapVal = (((dataWord[1] & 0x0f) << 8) + dataWord[0]) >> 5;
            *dfeCode = tapVal - 64;
            break;

        default:
            ASTERA_ERROR("Invalid DFE Tag");
            return ARIES_INVALID_ARGUMENT;
            break;
    }
    return ARIES_SUCCESS;
}

/*
 * Get the last speed at which EQ was run (e.g. 3 for Gen-3)
 */
AriesErrorType ariesGetLastEqSpeed(
        AriesLinkType* link,
        int lane,
        int direction,
        int* speed)
{
    AriesErrorType rc;
    int pathID = ((lane/2)*2) + direction;
    uint8_t dataByte[1];

    // Reading gp_ctrl_sts_struct.last_eq_pcie_gen
    int address = link->device->pm_gp_ctrl_sts_struct_addr +
        ARIES_CTRL_STS_STRUCT_LAST_EQ_PCIE_GEN;

    rc = ariesReadByteDataPathMicroIndirect(link->device->i2cDriver, pathID,
            address, dataByte);
    CHECK_SUCCESS(rc);

    *speed = dataByte[0];
    return ARIES_SUCCESS;
}

/*
 * Get the deskew status string for the given lane
 */
AriesErrorType ariesGetDeskewStatus(
        AriesLinkType* link,
        int lane,
        int direction,
        int* status)
{
    uint8_t dataByte[1];
    AriesErrorType rc;

    // Based on the lane info, determine QS and Path info
    int qs;
    int qsPath;
    int qsPathLane;
    ariesGetQSPathInfo(lane, direction, &qs, &qsPath, &qsPathLane);

    // Compute reg address
    // Calculate QS, Path, Path lane relative offsets first,
    // and then compute actual address
    int qsOff = ARIES_QS_0_CSR_OFFSET + (qs*ARIES_QS_STRIDE);
    int pathOff = ARIES_PATH_WRAPPER_0_CSR_OFFSET +
        (qsPath*ARIES_PATH_WRP_STRIDE);
    int pathLaneOff = ARIES_PATH_LANE_0_CSR_OFFSET +
        (qsPathLane*ARIES_PATH_LANE_STRIDE);

    uint32_t address = qsOff + pathOff + pathLaneOff +
        ARIES_DESKEW_STATUS;

    rc = ariesReadByteData(link->device->i2cDriver, address, dataByte);
    CHECK_SUCCESS(rc);
    *status = dataByte[0];
    return ARIES_SUCCESS;
}

/*
 * Get the number of clocks of deskew applied for the given lane
 */
AriesErrorType ariesGetDeskewClks(
        AriesLinkType* link,
        int lane,
        int direction,
        int* val)
{
    uint8_t dataByte[1];
    AriesErrorType rc;

    // Based on the lane info, determine QS and Path info
    int qs;
    int qsPath;
    int qsPathLane;
    ariesGetQSPathInfo(lane, direction, &qs, &qsPath, &qsPathLane);

    // Compute reg address
    // Calculate QS, Path, Path lane relative offsets first,
    // and then compute actual address
    int qsOff = ARIES_QS_0_CSR_OFFSET + (qs*ARIES_QS_STRIDE);
    int pathOff = ARIES_PATH_WRAPPER_0_CSR_OFFSET +
        (qsPath*ARIES_PATH_WRP_STRIDE);
    int pathLaneOff = ARIES_PATH_LANE_0_CSR_OFFSET +
        (qsPathLane*ARIES_PATH_LANE_STRIDE);

    uint32_t address = qsOff + pathOff + pathLaneOff +
        ARIES_DSK_CC_DELTA;

    rc = ariesReadByteData(link->device->i2cDriver, address, dataByte);
    CHECK_SUCCESS(rc);
    *val = dataByte[0];
    return ARIES_SUCCESS;
}

/*
 * For the last round of Equalization, get the final pre-cursor request the
 * link partner made. If the last request was a Perest request, this value
 * will be 0.
 */
AriesErrorType ariesGetLastEqReqPre(
        AriesLinkType* link,
        int lane,
        int direction,
        int* val)
{
    AriesErrorType rc;
    int pathID = ((lane/2)*2) + direction;
    int pathLane = lane%2;
    uint8_t dataByte[1];
    int offset;

    if (pathLane == 0)
    {
        offset = ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_PRE_LN0;
    }
    else
    {
        offset = ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_PRE_LN1;
    }

    // Reading gp_ctrl_sts_struct.last_eq_final_req_pre_lnX
    int address = link->device->pm_gp_ctrl_sts_struct_addr + offset;
    rc = ariesReadByteDataPathMicroIndirect(link->device->i2cDriver, pathID,
            address, dataByte);
    CHECK_SUCCESS(rc);

    *val = dataByte[0];
    return ARIES_SUCCESS;
}


/*
 * Capture FW state
 */
AriesErrorType ariesGetPathFWState(
        AriesLinkType* link,
        int lane,
        int direction,
        int* state)
{
    AriesErrorType rc;
    int pathID = ((lane/2)*2) + direction;
    /*int pathLane = lane%2;*/
    uint8_t dataByte[1];
    int offset = ARIES_CTRL_STS_STRUCT_FW_STATE;

    int address = link->device->pm_gp_ctrl_sts_struct_addr + offset;
    rc = ariesReadByteDataPathMicroIndirect(link->device->i2cDriver, pathID,
            address, dataByte);
    CHECK_SUCCESS(rc);

    *state = dataByte[0];
    return ARIES_SUCCESS;
}

/*
 * For the last round of Equalization, get the final cursor request the
 * link partner made. A value of 0xf will indicate the final request was a
 * coefficient request.
 */
AriesErrorType ariesGetLastEqReqCur(
        AriesLinkType* link,
        int lane,
        int direction,
        int* val)
{
    AriesErrorType rc;
    int pathID = ((lane/2)*2) + direction;
    int pathLane = lane%2;
    uint8_t dataByte[1];
    int offset;

    if (pathLane == 0)
    {
        offset = ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_CUR_LN0;
    }
    else
    {
        offset = ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_CUR_LN1;
    }

    // Reading gp_ctrl_sts_struct.last_eq_final_req_cur_lnX
    int address = link->device->pm_gp_ctrl_sts_struct_addr + offset;
    rc = ariesReadByteDataPathMicroIndirect(link->device->i2cDriver, pathID,
            address, dataByte);
    CHECK_SUCCESS(rc);

    *val = dataByte[0];
    return ARIES_SUCCESS;
}

/*
 * For the last round of Equalization, get the final post-cursor request the
 * link partner made. A value of 0xf will indicate the final request was a
 * coefficient request.
 */
AriesErrorType ariesGetLastEqReqPst(
        AriesLinkType* link,
        int lane,
        int direction,
        int* val)
{
    AriesErrorType rc;
    int pathID = ((lane/2)*2) + direction;
    int pathLane = lane%2;
    uint8_t dataByte[1];
    int offset;

    if (pathLane == 0)
    {
        offset = ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_PST_LN0;
    }
    else
    {
        offset = ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_PST_LN1;
    }

    // Reading gp_ctrl_sts_struct.last_eq_final_req_pst_lnX
    int address = link->device->pm_gp_ctrl_sts_struct_addr + offset;
    rc = ariesReadByteDataPathMicroIndirect(link->device->i2cDriver, pathID,
            address, dataByte);
    CHECK_SUCCESS(rc);

    *val = dataByte[0];
    return ARIES_SUCCESS;
}

/*
 * For the last round of Equalization, get the final preset request the link
 * partner made. A value of 0xf will indicate the final request was a
 * coefficient request.
 */
AriesErrorType ariesGetLastEqReqPreset(
        AriesLinkType* link,
        int lane,
        int direction,
        int* val)
{
    AriesErrorType rc;
    int pathID = ((lane/2)*2) + direction;
    int pathLane = lane%2;
    uint8_t dataByte[1];
    int offset;

    if (pathLane == 0)
    {
        offset  = ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_PRESET_LN0;
    }
    else
    {
        offset = ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_PRESET_LN1;
    }

    // Reading gp_ctrl_sts_struct.last_eq_final_req_preset_lnX
    int address = link->device->pm_gp_ctrl_sts_struct_addr + offset;
    rc = ariesReadByteDataPathMicroIndirect(link->device->i2cDriver, pathID,
            address, dataByte);
    CHECK_SUCCESS(rc);

    *val = dataByte[0];
    return ARIES_SUCCESS;
}

/*
 * Get final preset request
 */
AriesErrorType ariesGetLastEqPresetReq(
        AriesLinkType* link,
        int lane,
        int direction,
        int reqNum,
        int* val)
{
    AriesErrorType rc;
    int pathID = ((lane/2)*2) + direction;
    int pathLane = lane%2;
    uint8_t dataBytes[8];
    int offset;

    if (pathLane == 0)
    {
        offset  = ARIES_CTRL_STS_STRUCT_LAST_EQ_PRESET_REQS_LN0;
    }
    else
    {
        offset = ARIES_CTRL_STS_STRUCT_LAST_EQ_PRESET_REQS_LN1;
    }

    // Reading gp_ctrl_sts_struct.last_eq_final_req_preset_ln
    int address = link->device->pm_gp_ctrl_sts_struct_addr + offset;
    rc = ariesReadBlockDataPathMicroIndirect(link->device->i2cDriver, pathID,
            address, 8, dataBytes);
    CHECK_SUCCESS(rc);

    *val = dataBytes[reqNum];
    return ARIES_SUCCESS;
}

/*
 * Get final preset request FOM
 */
AriesErrorType ariesGetLastEqPresetReqFOM(
        AriesLinkType* link,
        int lane,
        int direction,
        int reqNum,
        int* val)
{
    AriesErrorType rc;
    int pathID = ((lane/2)*2) + direction;
    int pathLane = lane%2;
    uint8_t dataBytes[8];
    int offset;

    if (pathLane == 0)
    {
        offset  = ARIES_CTRL_STS_STRUCT_LAST_EQ_FOMS_LN0;
    }
    else
    {
        offset = ARIES_CTRL_STS_STRUCT_LAST_EQ_FOMS_LN1;
    }

    // Reading gp_ctrl_sts_struct.last_eq_final_req_preset_ln
    int address = link->device->pm_gp_ctrl_sts_struct_addr + offset;
    rc = ariesReadBlockDataPathMicroIndirect(link->device->i2cDriver, pathID,
            address, 8, dataBytes);
    CHECK_SUCCESS(rc);

    *val = dataBytes[reqNum];
    return ARIES_SUCCESS;
}

/*
 * Get format ID from current location
 */
AriesErrorType ariesGetLoggerFmtID(
        AriesLinkType* link,
        AriesLTSSMLoggerEnumType loggerType,
        int offset,
        int* fmtID)
{
    int address;
    uint8_t dataByte[1];
    AriesErrorType rc;

    // Main Micro
    if (loggerType == ARIES_LTSSM_LINK_LOGGER)
    {
        address = link->device->mm_print_info_struct_addr +
            ARIES_PRINT_INFO_STRUCT_PRINT_BUFFER_OFFSET +
            offset;
        rc = ariesReadByteDataMainMicroIndirect(link->device->i2cDriver,
                address, dataByte);
        CHECK_SUCCESS(rc);
        *fmtID = dataByte[0];
    }
    else
    {
        address = link->device->pm_print_info_struct_addr +
            ARIES_PRINT_INFO_STRUCT_PRINT_BUFFER_OFFSET +
            offset;
        rc = ariesReadByteDataPathMicroIndirect(link->device->i2cDriver,
                loggerType, address, dataByte);
        CHECK_SUCCESS(rc);
        *fmtID = dataByte[0];
    }

    return ARIES_SUCCESS;
}

/*
 * Get LTSSM Write Offset
 */
AriesErrorType ariesGetLoggerWriteOffset(
        AriesLinkType* link,
        AriesLTSSMLoggerEnumType loggerType,
        int* writeOffset)
{
    int address;
    int data0;
    int data1;
    uint8_t dataByte[1];
    AriesErrorType rc;

    // Main Micro
    if (loggerType == ARIES_LTSSM_LINK_LOGGER)
    {
        address = link->device->mm_print_info_struct_addr +
            ARIES_PRINT_INFO_STRUCT_WR_PTR_OFFSET;
        rc = ariesReadByteDataMainMicroIndirect(link->device->i2cDriver,
                address, dataByte);
        CHECK_SUCCESS(rc);
        data0 = dataByte[0];
        rc = ariesReadByteDataMainMicroIndirect(link->device->i2cDriver,
                (address+1), dataByte);
        CHECK_SUCCESS(rc);
        data1 = dataByte[0];

        *writeOffset = (data1 << 8) | data0;
    }
    else
    {
        address = link->device->pm_print_info_struct_addr +
            ARIES_PRINT_INFO_STRUCT_WR_PTR_OFFSET;
        rc = ariesReadByteDataPathMicroIndirect(link->device->i2cDriver,
                loggerType, address, dataByte);
        CHECK_SUCCESS(rc);
        data0 = dataByte[0];
        rc = ariesReadByteDataPathMicroIndirect(link->device->i2cDriver,
                loggerType, (address+1), dataByte);
        CHECK_SUCCESS(rc);
        data1 = dataByte[0];

        *writeOffset = (data1 << 8) | data0;
    }

    return ARIES_SUCCESS;
}


/*
 * Get One Batch Mode En from current location
 */
AriesErrorType ariesGetLoggerOneBatchModeEn(
        AriesLinkType* link,
        AriesLTSSMLoggerEnumType loggerType,
        int* oneBatchModeEn)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    int address;

    if (loggerType == ARIES_LTSSM_LINK_LOGGER)
    {
        address = link->device->mm_print_info_struct_addr +
            ARIES_PRINT_INFO_STRUCT_ONE_BATCH_MODE_EN_OFFSET;
        rc = ariesReadByteDataMainMicroIndirect(link->device->i2cDriver,
                address, dataByte);
        CHECK_SUCCESS(rc);
        *oneBatchModeEn = dataByte[0];
    }
    else
    {
        address = link->device->pm_print_info_struct_addr +
            ARIES_PRINT_INFO_STRUCT_ONE_BATCH_MODE_EN_OFFSET;
        rc = ariesReadByteDataPathMicroIndirect(link->device->i2cDriver,
                loggerType, address, dataByte);
        CHECK_SUCCESS(rc);
        *oneBatchModeEn = dataByte[0];
    }

    return ARIES_SUCCESS;
}


/*
 * Get One Batch Write En from current location
 */
AriesErrorType ariesGetLoggerOneBatchWrEn(
        AriesLinkType* link,
        AriesLTSSMLoggerEnumType loggerType,
        int* oneBatchWrEn)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    int address;

    if (loggerType == ARIES_LTSSM_LINK_LOGGER)
    {
        address = link->device->mm_print_info_struct_addr +
            ARIES_PRINT_INFO_STRUCT_ONE_BATCH_WR_EN_OFFSET;
        rc = ariesReadByteDataMainMicroIndirect(link->device->i2cDriver,
                address, dataByte);
        CHECK_SUCCESS(rc);
        *oneBatchWrEn = dataByte[0];
    }
    else
    {
        address = link->device->pm_print_info_struct_addr +
            ARIES_PRINT_INFO_STRUCT_ONE_BATCH_WR_EN_OFFSET;
        rc = ariesReadByteDataPathMicroIndirect(link->device->i2cDriver,
                loggerType, address, dataByte);
        CHECK_SUCCESS(rc);
        *oneBatchWrEn = dataByte[0];
    }

    return ARIES_SUCCESS;
}


uint8_t ariesGetPecByte(
        uint8_t* polynomial,
        uint8_t length)
{
    uint8_t crc;
    int byteIndex;
    int bitIndex;

    // Shift polynomial by 1 so that it is 8 bit
    // Take this extra bit into account later
    uint8_t poly = ARIES_CRC8_POLYNOMIAL >> 1;
    crc = polynomial[0];

    for (byteIndex = 1; byteIndex < length; byteIndex++)
    {
        uint8_t nextByte = polynomial[byteIndex];
        for (bitIndex = 7; bitIndex >= 0; bitIndex--)
        {
            // Check if MSB in CRC is 1
            if (crc & 0x80)
            {
                // Perform subtraction of first 8 bits
                crc = (crc ^ poly) << 1;
                // Add final bit of mod2 subtraction and place in pos 0 of CRC
                crc = crc + (((nextByte >> bitIndex) & 1) ^ 1);
            }
            else
            {
                // Shift out 0
                crc = crc << 1;
                // Shift in next bit
                crc = crc + ((nextByte >> bitIndex) & 1);
            }
        }
    }
    return crc;
}


AriesErrorType ariesGetMinFoMVal(
        AriesDeviceType* device,
        int side,
        int pathID,
        int lane,
        int regOffset,
        uint8_t* data)
{
    AriesErrorType rc;

    uint8_t dataByte[1];
    uint8_t dataWord[2];

    int address = regOffset;
    if (lane < 4)
    {
        address = regOffset + (lane * ARIES_PMA_LANE_STRIDE);
    }

    // Set up reg address
    dataWord[0] = address & 0xff;
    dataWord[1] = (address >> 8) & 0xff;
    rc = ariesWriteBlockData(device->i2cDriver,
            ARIES_PMA_MM_ASSIST_REG_ADDR_OFFSET, 2, dataWord);
    CHECK_SUCCESS(rc);

    // Path ID is represented in upper 4 bits of this byte
    dataByte[0] = pathID << 4;
    rc = ariesWriteByteData(device->i2cDriver,
            ARIES_PMA_MM_ASSIST_PATH_ID_OFFSET, dataByte);
    CHECK_SUCCESS(rc);

    // Set up PMA side
    dataByte[0] = ARIES_PMA_MM_ASSIST_SIDE0_RD + side;
    rc = ariesWriteByteData(device->i2cDriver,
            ARIES_PMA_MM_ASSIST_CMD_OFFSET, dataByte);
    CHECK_SUCCESS(rc);

    // Get data (LSB and MSB)
    rc = ariesReadByteData(device->i2cDriver,
            ARIES_PMA_MM_ASSIST_DATA0_OFFSET, dataByte);
    CHECK_SUCCESS(rc);
    data[0] = dataByte[0];
    rc = ariesReadByteData(device->i2cDriver,
            ARIES_PMA_MM_ASSIST_DATA1_OFFSET, dataByte);
    CHECK_SUCCESS(rc);
    data[1] = dataByte[0];

    return ARIES_SUCCESS;
}


AriesErrorType ariesGetPinMap(
        AriesDeviceType* device)
{
    if (device->partNumber == ARIES_PTX16)
    {
        device->pins[0].lane = 0;
        strcpy(device->pins[0].pinSet1.rxPin, "B_PER0");
        strcpy(device->pins[0].pinSet1.txPin, "A_PET0");
        device->pins[0].pinSet1.rxPackageInversion = 1;
        device->pins[0].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[0].pinSet2.rxPin, "A_PER0");
        strcpy(device->pins[0].pinSet2.txPin, "B_PET0");
        device->pins[0].pinSet2.rxPackageInversion = 1;
        device->pins[0].pinSet2.txPackageInsersion = 1;

        device->pins[1].lane = 1;
        strcpy(device->pins[1].pinSet1.rxPin, "B_PER1");
        strcpy(device->pins[1].pinSet1.txPin, "A_PET1");
        device->pins[1].pinSet1.rxPackageInversion = 1;
        device->pins[1].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[1].pinSet2.rxPin, "A_PER1");
        strcpy(device->pins[1].pinSet2.txPin, "B_PET1");
        device->pins[1].pinSet2.rxPackageInversion = 0;
        device->pins[1].pinSet2.txPackageInsersion = 0;

        device->pins[2].lane = 2;
        strcpy(device->pins[2].pinSet1.rxPin, "B_PER2");
        strcpy(device->pins[2].pinSet1.txPin, "A_PET2");
        device->pins[2].pinSet1.rxPackageInversion = 0;
        device->pins[2].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[2].pinSet2.rxPin, "A_PER2");
        strcpy(device->pins[2].pinSet2.txPin, "B_PET2");
        device->pins[2].pinSet2.rxPackageInversion = 1;
        device->pins[2].pinSet2.txPackageInsersion = 0;

        device->pins[3].lane = 3;
        strcpy(device->pins[3].pinSet1.rxPin, "B_PER3");
        strcpy(device->pins[3].pinSet1.txPin, "A_PET3");
        device->pins[3].pinSet1.rxPackageInversion = 0;
        device->pins[3].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[3].pinSet2.rxPin, "A_PER3");
        strcpy(device->pins[3].pinSet2.txPin, "B_PET3");
        device->pins[3].pinSet2.rxPackageInversion = 1;
        device->pins[3].pinSet2.txPackageInsersion = 1;

        device->pins[4].lane = 4;
        strcpy(device->pins[4].pinSet1.rxPin, "B_PER4");
        strcpy(device->pins[4].pinSet1.txPin, "A_PET4");
        device->pins[4].pinSet1.rxPackageInversion = 1;
        device->pins[4].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[4].pinSet2.rxPin, "A_PER4");
        strcpy(device->pins[4].pinSet2.txPin, "B_PET4");
        device->pins[4].pinSet2.rxPackageInversion = 0;
        device->pins[4].pinSet2.txPackageInsersion = 1;

        device->pins[5].lane = 5;
        strcpy(device->pins[5].pinSet1.rxPin, "B_PER5");
        strcpy(device->pins[5].pinSet1.txPin, "A_PET5");
        device->pins[5].pinSet1.rxPackageInversion = 1;
        device->pins[5].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[5].pinSet2.rxPin, "A_PER5");
        strcpy(device->pins[5].pinSet2.txPin, "B_PET5");
        device->pins[5].pinSet2.rxPackageInversion = 0;
        device->pins[5].pinSet2.txPackageInsersion = 0;

        device->pins[6].lane = 6;
        strcpy(device->pins[6].pinSet1.rxPin, "B_PER6");
        strcpy(device->pins[6].pinSet1.txPin, "A_PET6");
        device->pins[6].pinSet1.rxPackageInversion = 0;
        device->pins[6].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[6].pinSet2.rxPin, "A_PER6");
        strcpy(device->pins[6].pinSet2.txPin, "B_PET6");
        device->pins[6].pinSet2.rxPackageInversion = 1;
        device->pins[6].pinSet2.txPackageInsersion = 1;

        device->pins[7].lane = 7;
        strcpy(device->pins[7].pinSet1.rxPin, "B_PER7");
        strcpy(device->pins[7].pinSet1.txPin, "A_PET7");
        device->pins[7].pinSet1.rxPackageInversion = 0;
        device->pins[7].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[7].pinSet2.rxPin, "A_PER7");
        strcpy(device->pins[7].pinSet2.txPin, "B_PET7");
        device->pins[7].pinSet2.rxPackageInversion = 1;
        device->pins[7].pinSet2.txPackageInsersion = 1;

        device->pins[8].lane = 8;
        strcpy(device->pins[8].pinSet1.rxPin, "B_PER8");
        strcpy(device->pins[8].pinSet1.txPin, "A_PET8");
        device->pins[8].pinSet1.rxPackageInversion = 1;
        device->pins[8].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[8].pinSet2.rxPin, "A_PER8");
        strcpy(device->pins[8].pinSet2.txPin, "B_PET8");
        device->pins[8].pinSet2.rxPackageInversion = 1;
        device->pins[8].pinSet2.txPackageInsersion = 0;

        device->pins[9].lane = 9;
        strcpy(device->pins[9].pinSet1.rxPin, "B_PER9");
        strcpy(device->pins[9].pinSet1.txPin, "A_PET9");
        device->pins[9].pinSet1.rxPackageInversion = 1;
        device->pins[9].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[9].pinSet2.rxPin, "A_PER9");
        strcpy(device->pins[9].pinSet2.txPin, "B_PET9");
        device->pins[9].pinSet2.rxPackageInversion = 1;
        device->pins[9].pinSet2.txPackageInsersion = 0;

        device->pins[10].lane = 10;
        strcpy(device->pins[10].pinSet1.rxPin, "B_PER10");
        strcpy(device->pins[10].pinSet1.txPin, "A_PET10");
        device->pins[10].pinSet1.rxPackageInversion = 0;
        device->pins[10].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[10].pinSet2.rxPin, "A_PER10");
        strcpy(device->pins[10].pinSet2.txPin, "B_PET10");
        device->pins[10].pinSet2.rxPackageInversion = 0;
        device->pins[10].pinSet2.txPackageInsersion = 0;

        device->pins[11].lane = 11;
        strcpy(device->pins[11].pinSet1.rxPin, "B_PER11");
        strcpy(device->pins[11].pinSet1.txPin, "A_PET11");
        device->pins[11].pinSet1.rxPackageInversion = 0;
        device->pins[11].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[11].pinSet2.rxPin, "A_PER11");
        strcpy(device->pins[11].pinSet2.txPin, "B_PET11");
        device->pins[11].pinSet2.rxPackageInversion = 0;
        device->pins[11].pinSet2.txPackageInsersion = 1;

        device->pins[12].lane = 12;
        strcpy(device->pins[12].pinSet1.rxPin, "B_PER12");
        strcpy(device->pins[12].pinSet1.txPin, "A_PET12");
        device->pins[12].pinSet1.rxPackageInversion = 1;
        device->pins[12].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[12].pinSet2.rxPin, "A_PER12");
        strcpy(device->pins[12].pinSet2.txPin, "B_PET12");
        device->pins[12].pinSet2.rxPackageInversion = 1;
        device->pins[12].pinSet2.txPackageInsersion = 1;

        device->pins[13].lane = 13;
        strcpy(device->pins[13].pinSet1.rxPin, "B_PER13");
        strcpy(device->pins[13].pinSet1.txPin, "A_PET13");
        device->pins[13].pinSet1.rxPackageInversion = 1;
        device->pins[13].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[13].pinSet2.rxPin, "A_PER13");
        strcpy(device->pins[13].pinSet2.txPin, "B_PET13");
        device->pins[13].pinSet2.rxPackageInversion = 1;
        device->pins[13].pinSet2.txPackageInsersion = 1;

        device->pins[14].lane = 14;
        strcpy(device->pins[14].pinSet1.rxPin, "B_PER14");
        strcpy(device->pins[14].pinSet1.txPin, "A_PET14");
        device->pins[14].pinSet1.rxPackageInversion = 0;
        device->pins[14].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[14].pinSet2.rxPin, "A_PER14");
        strcpy(device->pins[14].pinSet2.txPin, "B_PET14");
        device->pins[14].pinSet2.rxPackageInversion = 0;
        device->pins[14].pinSet2.txPackageInsersion = 0;

        device->pins[15].lane = 15;
        strcpy(device->pins[15].pinSet1.rxPin, "B_PER15");
        strcpy(device->pins[15].pinSet1.txPin, "A_PET15");
        device->pins[15].pinSet1.rxPackageInversion = 0;
        device->pins[15].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[15].pinSet2.rxPin, "A_PER15");
        strcpy(device->pins[15].pinSet2.txPin, "B_PET15");
        device->pins[15].pinSet2.rxPackageInversion = 1;
        device->pins[15].pinSet2.txPackageInsersion = 0;
    }
    else if (device->partNumber == ARIES_PTX08)
    {
        device->pins[0].lane = 0;
        strcpy(device->pins[0].pinSet1.rxPin, "");
        strcpy(device->pins[0].pinSet1.txPin, "");
        device->pins[0].pinSet1.rxPackageInversion = 1;
        device->pins[0].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[0].pinSet2.rxPin, "");
        strcpy(device->pins[0].pinSet2.txPin, "");
        device->pins[0].pinSet2.rxPackageInversion = 1;
        device->pins[0].pinSet2.txPackageInsersion = 1;

        device->pins[1].lane = 1;
        strcpy(device->pins[1].pinSet1.rxPin, "");
        strcpy(device->pins[1].pinSet1.txPin, "");
        device->pins[1].pinSet1.rxPackageInversion = 1;
        device->pins[1].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[1].pinSet2.rxPin, "");
        strcpy(device->pins[1].pinSet2.txPin, "");
        device->pins[1].pinSet2.rxPackageInversion = 0;
        device->pins[1].pinSet2.txPackageInsersion = 0;

        device->pins[2].lane = 2;
        strcpy(device->pins[2].pinSet1.rxPin, "");
        strcpy(device->pins[2].pinSet1.txPin, "");
        device->pins[2].pinSet1.rxPackageInversion = 0;
        device->pins[2].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[2].pinSet2.rxPin, "");
        strcpy(device->pins[2].pinSet2.txPin, "");
        device->pins[2].pinSet2.rxPackageInversion = 1;
        device->pins[2].pinSet2.txPackageInsersion = 0;

        device->pins[3].lane = 3;
        strcpy(device->pins[3].pinSet1.rxPin, "");
        strcpy(device->pins[3].pinSet1.txPin, "");
        device->pins[3].pinSet1.rxPackageInversion = 0;
        device->pins[3].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[3].pinSet2.rxPin, "");
        strcpy(device->pins[3].pinSet2.txPin, "");
        device->pins[3].pinSet2.rxPackageInversion = 1;
        device->pins[3].pinSet2.txPackageInsersion = 1;

        device->pins[4].lane = 4;
        strcpy(device->pins[4].pinSet1.rxPin, "A_PER0");
        strcpy(device->pins[4].pinSet1.txPin, "B_PET0");
        device->pins[4].pinSet1.rxPackageInversion = 1;
        device->pins[4].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[4].pinSet2.rxPin, "B_PER0");
        strcpy(device->pins[4].pinSet2.txPin, "A_PET0");
        device->pins[4].pinSet2.rxPackageInversion = 0;
        device->pins[4].pinSet2.txPackageInsersion = 1;

        device->pins[5].lane = 5;
        strcpy(device->pins[5].pinSet1.rxPin, "A_PER1");
        strcpy(device->pins[5].pinSet1.txPin, "B_PET1");
        device->pins[5].pinSet1.rxPackageInversion = 1;
        device->pins[5].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[5].pinSet2.rxPin, "B_PER1");
        strcpy(device->pins[5].pinSet2.txPin, "A_PET1");
        device->pins[5].pinSet2.rxPackageInversion = 0;
        device->pins[5].pinSet2.txPackageInsersion = 0;

        device->pins[6].lane = 6;
        strcpy(device->pins[6].pinSet1.rxPin, "A_PER2");
        strcpy(device->pins[6].pinSet1.txPin, "B_PET2");
        device->pins[6].pinSet1.rxPackageInversion = 0;
        device->pins[6].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[6].pinSet2.rxPin, "B_PER2");
        strcpy(device->pins[6].pinSet2.txPin, "A_PET2");
        device->pins[6].pinSet2.rxPackageInversion = 1;
        device->pins[6].pinSet2.txPackageInsersion = 1;

        device->pins[7].lane = 7;
        strcpy(device->pins[7].pinSet1.rxPin, "A_PER3");
        strcpy(device->pins[7].pinSet1.txPin, "B_PET3");
        device->pins[7].pinSet1.rxPackageInversion = 0;
        device->pins[7].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[7].pinSet2.rxPin, "B_PER3");
        strcpy(device->pins[7].pinSet2.txPin, "A_PET3");
        device->pins[7].pinSet2.rxPackageInversion = 1;
        device->pins[7].pinSet2.txPackageInsersion = 1;

        device->pins[8].lane = 8;
        strcpy(device->pins[8].pinSet1.rxPin, "A_PER4");
        strcpy(device->pins[8].pinSet1.txPin, "B_PET4");
        device->pins[8].pinSet1.rxPackageInversion = 1;
        device->pins[8].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[8].pinSet2.rxPin, "B_PER4");
        strcpy(device->pins[8].pinSet2.txPin, "A_PET4");
        device->pins[8].pinSet2.rxPackageInversion = 1;
        device->pins[8].pinSet2.txPackageInsersion = 0;

        device->pins[9].lane = 9;
        strcpy(device->pins[9].pinSet1.rxPin, "A_PER5");
        strcpy(device->pins[9].pinSet1.txPin, "B_PET5");
        device->pins[9].pinSet1.rxPackageInversion = 1;
        device->pins[9].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[9].pinSet2.rxPin, "B_PER5");
        strcpy(device->pins[9].pinSet2.txPin, "A_PET5");
        device->pins[9].pinSet2.rxPackageInversion = 1;
        device->pins[9].pinSet2.txPackageInsersion = 0;

        device->pins[10].lane = 10;
        strcpy(device->pins[10].pinSet1.rxPin, "A_PER6");
        strcpy(device->pins[10].pinSet1.txPin, "B_PET6");
        device->pins[10].pinSet1.rxPackageInversion = 0;
        device->pins[10].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[10].pinSet2.rxPin, "B_PER6");
        strcpy(device->pins[10].pinSet2.txPin, "A_PET6");
        device->pins[10].pinSet2.rxPackageInversion = 0;
        device->pins[10].pinSet2.txPackageInsersion = 0;

        device->pins[11].lane = 11;
        strcpy(device->pins[11].pinSet1.rxPin, "A_PER7");
        strcpy(device->pins[11].pinSet1.txPin, "B_PET7");
        device->pins[11].pinSet1.rxPackageInversion = 0;
        device->pins[11].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[11].pinSet2.rxPin, "B_PER7");
        strcpy(device->pins[11].pinSet2.txPin, "A_PET7");
        device->pins[11].pinSet2.rxPackageInversion = 0;
        device->pins[11].pinSet2.txPackageInsersion = 1;

        device->pins[12].lane = 12;
        strcpy(device->pins[12].pinSet1.rxPin, "");
        strcpy(device->pins[12].pinSet1.txPin, "");
        device->pins[12].pinSet1.rxPackageInversion = 1;
        device->pins[12].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[12].pinSet2.rxPin, "");
        strcpy(device->pins[12].pinSet2.txPin, "");
        device->pins[12].pinSet2.rxPackageInversion = 1;
        device->pins[12].pinSet2.txPackageInsersion = 1;

        device->pins[13].lane = 13;
        strcpy(device->pins[13].pinSet1.rxPin, "");
        strcpy(device->pins[13].pinSet1.txPin, "");
        device->pins[13].pinSet1.rxPackageInversion = 1;
        device->pins[13].pinSet1.txPackageInsersion = 0;
        strcpy(device->pins[13].pinSet2.rxPin, "");
        strcpy(device->pins[13].pinSet2.txPin, "");
        device->pins[13].pinSet2.rxPackageInversion = 1;
        device->pins[13].pinSet2.txPackageInsersion = 1;

        device->pins[14].lane = 14;
        strcpy(device->pins[14].pinSet1.rxPin, "");
        strcpy(device->pins[14].pinSet1.txPin, "");
        device->pins[14].pinSet1.rxPackageInversion = 0;
        device->pins[14].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[14].pinSet2.rxPin, "");
        strcpy(device->pins[14].pinSet2.txPin, "");
        device->pins[14].pinSet2.rxPackageInversion = 0;
        device->pins[14].pinSet2.txPackageInsersion = 0;

        device->pins[15].lane = 15;
        strcpy(device->pins[15].pinSet1.rxPin, "");
        strcpy(device->pins[15].pinSet1.txPin, "");
        device->pins[15].pinSet1.rxPackageInversion = 0;
        device->pins[15].pinSet1.txPackageInsersion = 1;
        strcpy(device->pins[15].pinSet2.rxPin, "");
        strcpy(device->pins[15].pinSet2.txPin, "");
        device->pins[15].pinSet2.rxPackageInversion = 1;
        device->pins[15].pinSet2.txPackageInsersion = 0;
    }
    else
    {
        return ARIES_INVALID_ARGUMENT;
    }

    return ARIES_SUCCESS;
}


AriesErrorType ariesEepromReadBlockData(
        AriesDeviceType *device,
        uint8_t* values,
        int startAddr,
        uint8_t numBytes)
{
    AriesErrorType rc;
    int addrMSB;
    int addr;
    int currentPage = 0;
    bool firstRead = true;
    uint8_t dataByte[1];
    int valIndex = 0;

    if ((numBytes <= 0) || (startAddr < 0) || (startAddr >= 262144) ||
        ((startAddr+numBytes-1) >= 262144))
    {
        return ARIES_INVALID_ARGUMENT;
    }

    // Perform legacy mode reads here
    for (addr = startAddr; addr < (startAddr+numBytes); addr++)
    {
        addrMSB = floor(addr/65536);
        // Set page and address on the bus with first read
        // Or whenever there is a page update
        if (firstRead || (addrMSB != currentPage))
        {
            // Set updated page address
            rc = ariesI2CMasterSetPage(device->i2cDriver, addrMSB);
            CHECK_SUCCESS(rc);
            currentPage = addrMSB;

            // Send EEPROM address 0 after page update
            dataByte[0] = (addr >> 8) & 0xff;
            rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 2);
            CHECK_SUCCESS(rc);
            dataByte[0] = addr & 0xff;
            rc = ariesI2CMasterSendByte(device->i2cDriver, dataByte, 1);
            CHECK_SUCCESS(rc);
            firstRead = false;
        }

        // Perform byte read and store in values array
        rc = ariesI2CMasterReceiveByte(device->i2cDriver, dataByte);
        CHECK_SUCCESS(rc);

        values[valIndex] = dataByte[0];
        valIndex++;
    }

    return ARIES_SUCCESS;
}


AriesErrorType ariesEepromCalcChecksum(
        AriesDeviceType* device,
        int startAddr,
        int numBytes,
        uint8_t* checksum)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    uint8_t runningChecksum = 0;
    int readVal;

    if ((numBytes <= 0) || (startAddr < 0) || (startAddr >= 262144) ||
        ((startAddr+numBytes-1) >= 262144))
    {
        return ARIES_INVALID_ARGUMENT;
    }

    // Calculate expected checksum
    int addr;
    for (addr = startAddr; addr < (startAddr+numBytes); addr++)
    {
        rc = ariesEepromReadBlockData(device, dataByte, addr, 1);
        CHECK_SUCCESS(rc);
        readVal = dataByte[0];
        runningChecksum = (runningChecksum + readVal) & 0xff;
    }
    *checksum = runningChecksum;
    return ARIES_SUCCESS;
}


AriesErrorType ariesGetDPLLFreq(
        AriesLinkType* link,
        int side,
        int absLane,
        uint16_t* dpllFreq)
{
    AriesErrorType rc;
    int pmaNum = ariesGetPmaNumber(absLane);
    int pmaLane = ariesGetPmaLane(absLane);

    uint8_t dataWord[2];
    rc = ariesReadWordPmaLaneMainMicroIndirect(link->device->i2cDriver, side,
            pmaNum, pmaLane, ARIES_PMA_LANE_DIG_RX_DPLL_FREQ_ADDRESS,
            dataWord);
    CHECK_SUCCESS(rc);

    *dpllFreq = dataWord[0] + (dataWord[1] << 8);
    return ARIES_SUCCESS;
}


void ariesSortArray(
        uint16_t* arr,
        int size)
{
   int i,j,temp;
   for (i = 0; i < (size-1); i++)
   {
        for (j = 0; j < (size-i-1); j++)
        {
            if (arr[j] > arr[j+1])
            {
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}


uint16_t ariesGetMedian(
        uint16_t* arr,
        int size)
{
    ariesSortArray(arr, size);
    // Capture middle element
    int ind = ((size+1)/2) - 1;
    return arr[ind];
}

/* loads a bin file into the global memory[] array */
/* filename is a string of the file to be opened */
AriesErrorType ariesLoadBinFile(
        char* filename,
        uint8_t* mem)
{
    FILE *fin;
    int r;

    // Check if file is valid
    if (strlen(filename) == 0)
    {
        ASTERA_ERROR("Can't load a file without the filename");
        return ARIES_INVALID_ARGUMENT;
    }
    fin = fopen(filename, "r");
    if (fin == NULL)
    {
        ASTERA_ERROR("Can't open file '%s' for reading", filename);
        return ARIES_FAILURE;
    }

    r = fread(mem, 1, ARIES_EEPROM_NUM_BYTES, fin);

    ASTERA_INFO("Read %d bytes from binary file", r);

    fclose(fin);

    if (r != ARIES_EEPROM_NUM_BYTES) {
        ASTERA_ERROR("Expected %d bytes from binary file", ARIES_EEPROM_NUM_BYTES);
        return ARIES_FAILURE;
    }
    return ARIES_SUCCESS;
}


/* loads an intel hex file into the global memory[] array */
/* filename is a string of the file to be opened */
AriesErrorType ariesLoadIhxFile(
        char* filename,
        uint8_t* mem)
{
    AriesErrorType rc;
    char line[1000];
    FILE *fin;
    uint addr;
    uint n;
    uint status;
    uint8_t bytes[256];
    uint byte;
    int total = 0;
    int lineno = 1;
    uint minAddr = 0xffff;
    uint maxAddr = 0;

    // Check if file is valid
    if (strlen(filename) == 0)
    {
        ASTERA_ERROR("Can't load a file without the filename");
        return ARIES_INVALID_ARGUMENT;
    }
    fin = fopen(filename, "r");
    if (fin == NULL)
    {
        ASTERA_ERROR("Can't open file '%s' for reading", filename);
        return ARIES_FAILURE;
    }

    int indx = 0;
    while (!feof(fin) && !ferror(fin))
    {
        char *r;
        line[0] = '\0';
        r = fgets(line, 1000, fin);
        if (r == NULL) {
            ASTERA_WARN("fgets failure");
        }
        if (line[strlen(line)-1] == '\n')
        {
            line[strlen(line)-1] = '\0';
        }
        if (line[strlen(line)-1] == '\r')
        {
            line[strlen(line)-1] = '\0';
        }

        rc = ariesParseIhxLine(line, bytes, &addr, &n, &status);
        if (rc != ARIES_SUCCESS)
        {
            ASTERA_ERROR("Error: '%s', line: %d", filename, lineno);
            break;
        }
        else
        {
            if (status == 0)
            {  /* data */
                for (byte = 0; byte <= (n-1); byte++)
                {
                    mem[indx] = bytes[byte] & 0xff;
                    total++;
                    if (addr < minAddr) minAddr = addr;
                    if (addr > maxAddr) maxAddr = addr;
                    addr++;
                    indx++;
                }
            }
            if (status == 1)
            {  /* end of file */
                fclose(fin);
                return ARIES_SUCCESS;
            }
        }
        lineno++;
    }
    return ARIES_FAILURE;
}

/* parses a line of intel hex code, stores the data in bytes[] */
/* and the beginning address in addr, and returns a ARIES_SUCCESS if the */
/* line was valid, or a ARIES_FAILURE if an error occured.  The variable */
/* num gets the number of bytes that were stored into bytes[] */
AriesErrorType ariesParseIhxLine(
        char* line,
        uint8_t* bytes,
        uint* addr,
        uint* num,
        uint* code)
{
    uint sum;
    uint cksum;
    uint len;
    char *ptr;

    *num = 0;
    // First char
    if (line[0] != ':')
    {
        return ARIES_FAILURE;
    }

    if (strlen(line) < 11)
    {
        return ARIES_FAILURE;
    }
    ptr = line+1;

    if (!sscanf(ptr, "%02x", &len))
    {
        return ARIES_FAILURE;
    }
    ptr += 2;

    if ( strlen(line) < (11 + (len * 2)) )
    {
        return ARIES_FAILURE;
    }
    if (!sscanf(ptr, "%04x", addr))
    {
        return ARIES_FAILURE;
    }
    ptr += 4;

    if (!sscanf(ptr, "%02x", code))
    {
        return ARIES_FAILURE;
    }
    ptr += 2;
    sum = (len & 255) + ((*addr >> 8) & 255) + (*addr & 255) + (*code & 255);
    uint val;
    while (*num != len)
    {
        int status = sscanf(ptr, "%02x", &val);
        bytes[*num] = (uint8_t) val;
        if (!status)
        {
            return ARIES_FAILURE;
        }
        ptr += 2;
        sum += bytes[*num] & 255;
        (*num)++;
        if (*num >= 256)
        {
            return ARIES_FAILURE;
        }
    }
    if (!sscanf(ptr, "%02x", &cksum))
    {
        return ARIES_FAILURE;
    }
    if ( ((sum & 255) + (cksum & 255)) & 255 )
    {
        return ARIES_FAILURE; /* checksum error */
    }
    return ARIES_SUCCESS;
}


AriesErrorType ariesI2cMasterSoftReset(
        AriesI2CDriverType* i2cDriver)
{
    AriesErrorType rc;

    uint8_t i2cInitCtrl;
    /*uint16_t loopCount;*/
    uint16_t indx;
    uint8_t dataByte[1];

    /*loopCount = 0xff;*/

    // Enable Bit bang mode
    dataByte[0] = 3;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_BB_OUTPUT_ADDRESS, dataByte);
    CHECK_SUCCESS(rc);

    rc = ariesReadByteData(i2cDriver, ARIES_I2C_MST_INIT_CTRL_ADDRESS, dataByte);
    CHECK_SUCCESS(rc);
    i2cInitCtrl = dataByte[0];

    i2cInitCtrl = ARIES_MAIN_MICRO_EXT_CSR_I2C_MST_INIT_CTRL_BIT_BANG_MODE_EN_MODIFY(i2cInitCtrl, 1);
    dataByte[0] = i2cInitCtrl;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_INIT_CTRL_ADDRESS, dataByte);
    CHECK_SUCCESS(rc);

    // Start Sequence
    // SDA = 1, SCL = 1
    dataByte[0] = 3;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_BB_OUTPUT_ADDRESS, dataByte);
    CHECK_SUCCESS(rc);
    // SDA = 0, SCL = 1
    dataByte[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_BB_OUTPUT_ADDRESS, dataByte);
    CHECK_SUCCESS(rc);
    // SDA = 0, SCL = 0
    dataByte[0] = 0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_BB_OUTPUT_ADDRESS, dataByte);
    CHECK_SUCCESS(rc);
    // SDA = 1, SCL = 0
    dataByte[0] = 2;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_BB_OUTPUT_ADDRESS, dataByte);
    CHECK_SUCCESS(rc);


    for (indx = 0; indx < 9; indx++)
    {
        // SDA = 1, SCL = 1
        dataByte[0] = 3;
        rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_BB_OUTPUT_ADDRESS, dataByte);
        CHECK_SUCCESS(rc);

        // SDA = 1 SCL = 0
        dataByte[0] = 2;
        rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_BB_OUTPUT_ADDRESS, dataByte);
        CHECK_SUCCESS(rc);
    }

    // Stop Sequence
    // SDA = 0, SCL = 0
    dataByte[0] = 0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_BB_OUTPUT_ADDRESS, dataByte);
    CHECK_SUCCESS(rc);
    // SDA = 0, SCL = 1
    dataByte[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_BB_OUTPUT_ADDRESS, dataByte);
    CHECK_SUCCESS(rc);
    // SDA = 1, SCL = 1
    dataByte[0] = 3;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_BB_OUTPUT_ADDRESS, dataByte);
    CHECK_SUCCESS(rc);

    // Disable BB mode
    i2cInitCtrl = ARIES_MAIN_MICRO_EXT_CSR_I2C_MST_INIT_CTRL_BIT_BANG_MODE_EN_MODIFY(i2cInitCtrl, 0);
    dataByte[0] = i2cInitCtrl;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_INIT_CTRL_ADDRESS, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


AriesErrorType ariesGetEEPROMFirstBlock(
        AriesI2CDriverType* i2cDriver,
        int* blockStartAddr)
{
    AriesErrorType rc;
    uint8_t dataByte[1];

    // Set Page = 0
    rc = ariesI2CMasterSetPage(i2cDriver, 0);
    CHECK_SUCCESS(rc);

    // Set address = 0
    dataByte[0] = 0;
    rc = ariesI2CMasterSendByte(i2cDriver, dataByte, 2);
    CHECK_SUCCESS(rc);
    dataByte[0] = 0;
    rc = ariesI2CMasterSendByte(i2cDriver, dataByte, 1);
    CHECK_SUCCESS(rc);

    // The first block SHOULD start at address zero, but check up to
    // maxAddrToCheck bytes to find it.
    int addr = 0;
    // int blockStartAddr = 0;
    int maxAddrToCheck = 50;

    uint8_t byte0;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;

    /*bool addrCheck = false;*/

    while (addr < maxAddrToCheck)
    {
        rc = ariesI2CMasterReceiveByte(i2cDriver, dataByte);
        CHECK_SUCCESS(rc);
        byte0 = dataByte[0];
        if (byte0 == 0xa5)
        {
            rc = ariesI2CMasterReceiveByte(i2cDriver, dataByte);
            CHECK_SUCCESS(rc);
            byte1 = dataByte[0];
            if (byte1 == 0x5a)
            {
                rc = ariesI2CMasterReceiveByte(i2cDriver, dataByte);
                CHECK_SUCCESS(rc);
                byte2 = dataByte[0];
                if (byte2 == 0xa5)
                {
                    rc = ariesI2CMasterReceiveByte(i2cDriver, dataByte);
                    CHECK_SUCCESS(rc);
                    byte3 = dataByte[0];
                    if (byte3 == 0x5a)
                    {
                        /*addrCheck = true;*/
                        *blockStartAddr = addr;
                        break;
                    }
                }
            }
        }
        addr++;
    }

    return ARIES_SUCCESS;
}


AriesErrorType ariesGetEEPROMBlockType(
        AriesI2CDriverType* i2cDriver,
        int blockStartAddr,
        uint8_t* blockType)
{
    AriesErrorType rc;
    int addr;
    addr = blockStartAddr+4;

    rc = ariesEEPROMGetRandomByte(i2cDriver, addr, blockType);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


AriesErrorType ariesGetEEPROMBlockCrcByte(
        AriesI2CDriverType* i2cDriver,
        int blockStartAddr,
        int blockLength,
        uint8_t* crcByte)
{
    AriesErrorType rc;
    int addr;
    addr = blockStartAddr+blockLength+11;

    rc = ariesEEPROMGetRandomByte(i2cDriver, addr, crcByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


AriesErrorType ariesEEPROMGetRandomByte(
        AriesI2CDriverType* i2cDriver,
        int addr,
        uint8_t* value)
{
    AriesErrorType rc;
    int addrMSB = floor(addr/65536);
    uint8_t dataByte[1];

    rc = ariesI2CMasterSetPage(i2cDriver, addrMSB);
    CHECK_SUCCESS(rc);

    // Send address
    // Write command
    dataByte[0] = 0x10;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_IC_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    // Prepare Flag Byte
    dataByte[0] = 0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA1_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    uint8_t addr15To8;
    uint8_t addr7To0;
    addr15To8 = (addr >> 8) & 0xff;
    addr7To0 = addr & 0xff;
    dataByte[0] = addr15To8;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = addr7To0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    dataByte[0] = 1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    dataByte[0] = 3;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_DATA1_ADDR, dataByte);
    CHECK_SUCCESS(rc);

    dataByte[0] = 0x1;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    usleep(ARIES_I2C_MASTER_CMD_RST);
    dataByte[0] = 0x0;
    rc = ariesWriteByteData(i2cDriver, ARIES_I2C_MST_CMD_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    rc = ariesReadByteData(i2cDriver, ARIES_I2C_MST_DATA0_ADDR, dataByte);
    CHECK_SUCCESS(rc);
    *value = dataByte[0];

    return ARIES_SUCCESS;
}


AriesErrorType ariesEEPROMGetBlockLength(
        AriesI2CDriverType* i2cDriver,
        int blockStartAddr,
        int* blockLength)
{
    AriesErrorType rc;
    int addrMSB = floor(blockStartAddr/65536);

    rc = ariesI2CMasterSetPage(i2cDriver, addrMSB);
    CHECK_SUCCESS(rc);

    int addrBlockLengthLSB = blockStartAddr + 5;
    uint8_t blockLengthLSB;
    rc = ariesEEPROMGetRandomByte(i2cDriver, addrBlockLengthLSB,
        &blockLengthLSB);
    CHECK_SUCCESS(rc);

    int addrBlockLengthMSB = blockStartAddr + 6;
    uint8_t blockLengthMSB;
    rc = ariesEEPROMGetRandomByte(i2cDriver, addrBlockLengthMSB,
        &blockLengthMSB);
    CHECK_SUCCESS(rc);

    *blockLength = (blockLengthMSB << 8) + blockLengthLSB;
    return ARIES_SUCCESS;
}


void ariesGetCrcBytesImage(
        uint8_t* image,
        uint8_t* crcBytes,
        uint8_t* numCrcBytes)
{

    int numBlocks = 0;

    // Get First Block
    int addr = 0;
    int blockStartAddr = 0;
    int maxAddrToCheck = 50;
    uint8_t byte0;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
    while (addr < maxAddrToCheck)
    {
        byte0 = image[addr];
        if (byte0 == 0xa5)
        {
            addr++;
            byte1 = image[addr];
            if (byte1 == 0x5a)
            {
                addr++;
                byte2 = image[addr];
                if (byte2 == 0xa5)
                {
                    addr++;
                    byte3 = image[addr];
                    if (byte3 == 0x5a)
                    {
                        blockStartAddr = addr-3;
                        break;
                    }
                }
            }
        }
        addr++;
    }

    while (numBlocks < ARIES_EEPROM_MAX_NUM_CRC_BLOCKS)
    {
        // Get Block Type
        uint8_t blockType = image[(blockStartAddr+4)];
        if (blockType != 0xff)
        {
            // Get Block Length
            uint8_t blockLengthLSB = image[(blockStartAddr+5)];
            uint8_t blockLengthMSB = image[(blockStartAddr+6)];
            uint16_t blockLength = (blockLengthMSB << 8) + blockLengthLSB;

            uint8_t crcByte = image[(blockStartAddr+blockLength+11)];
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
}


AriesErrorType ariesSetMMReset(
        AriesDeviceType* device,
        bool value)
{
    AriesErrorType rc;
    uint8_t dataWord[2];

    /*
    rc = ariesReadBlockData(device->i2cDriver, 0x602, 2, dataWord);
    CHECK_SUCCESS(rc);
    if (value)
    {
        dataWord[1] |= (1 << 2);
    }
    else
    {
        dataWord[1] &= ~(1 << 2);
    }
    */
    if (value)
    {
      dataWord[1] = 0x04;
      dataWord[0] = 0x00;
    }
    else
    {
      dataWord[1] = 0x00;
      dataWord[0] = 0x00;
    }
    rc = ariesWriteBlockData(device->i2cDriver, 0x602, 2, dataWord);
    CHECK_SUCCESS(rc);
    // if (reset == True):
    //         self.csr.misc.sw_rst = (self.csr.misc.sw_rst.value | (1 << 10))
    //     else:
    //         self.csr.misc.sw_rst = (self.csr.misc.sw_rst.value & ~(1 << 10))
    return ARIES_SUCCESS;
}


AriesErrorType ariesPipeRxAdapt(
        AriesDeviceType* device,
        int side,
        int lane)
{
    AriesErrorType rc;
    uint8_t dataByte[2];
    uint8_t sigdet;
    uint8_t qs = floor(lane/4);
    uint8_t pma_ln = (lane % 4);

    //# First confirm there is signal detect
    //sigdet = ipid.__dict__['LANE'+str(pma_ln)+'_DIG_ASIC_RX_ASIC_OUT_0'].SIGDET_LF.value
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, pma_ln,
                ARIES_PMA_LANE_DIG_ASIC_RX_ASIC_OUT_0, dataByte);
    CHECK_SUCCESS(rc);
    sigdet = (dataByte[0] >> 2) & 1;
    if (sigdet == 0)
    {
        //logger.warning("Side:%s, Lane:%02d, Signal not detected!" % (side, lane))
        ASTERA_INFO("Side:%1d, Lane:%02d, Signal not detected! Confirm link partner transmitter is enabled.", side, lane);
    }

    //ipid.__dict__['RAWLANE'+str(pma_ln)+'_DIG_PCS_XF_RX_OVRD_IN_1'].REQ_OVRD_VAL=0
    //ipid.__dict__['RAWLANE'+str(pma_ln)+'_DIG_PCS_XF_RX_OVRD_IN_1'].REQ_OVRD_EN=0
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, pma_ln,
                ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataByte);
    dataByte[0] &= ~(1 << 3); // REQ_OVRD_EN is bit 3
    dataByte[0] &= ~(1 << 2); // REQ_OVRD_VAL is bit 2
    rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, pma_ln,
                ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataByte);
    CHECK_SUCCESS(rc);

    //rx_data_en_set(ipid=ipid, lane=pma_ln, en=0)
    rc = ariesPMARxDataEnSet(device, side, lane, false);
    CHECK_SUCCESS(rc);

    //self.csr.__dict__['qs_'+str(qs)].__dict__['pth_wrap_'+str(pth_wrap)].__dict__['ret_pth_ln'+str(ret_ln)].mac_phy_rxstandby=2
    rc = ariesPipeRxStandbySet(device, side, lane, false);
    CHECK_SUCCESS(rc);

    //self.csr.__dict__['qs_'+str(qs)].__dict__['pth_wrap_'+str(pth_wrap)].__dict__['ret_pth_ln'+str(ret_ln)].mac_phy_rxeqeval=3
    rc = ariesPipeRxEqEval(device, side, lane, true);
    CHECK_SUCCESS(rc);

    // time.sleep(0.1)
    usleep(ARIES_PIPE_RXEQEVAL_TIME_US);

    //self.csr.__dict__['qs_'+str(qs)].__dict__['pth_wrap_'+str(pth_wrap)].__dict__['ret_pth_ln'+str(ret_ln)].mac_phy_rxeqeval=2
    rc = ariesPipeRxEqEval(device, side, lane, false);
    CHECK_SUCCESS(rc);

    //rx_data_en_set(ipid=ipid, lane=pma_ln, en=1)
    rc = ariesPMARxDataEnSet(device, side, lane, true);
    CHECK_SUCCESS(rc);

    int i;
    for (i = 0; i < 20; i++)
    {
        //ipid.__dict__['RAWLANE'+str(pma_ln)+'_DIG_PCS_XF_RX_OVRD_IN_1'].REQ_OVRD_VAL=0
        //ipid.__dict__['RAWLANE'+str(pma_ln)+'_DIG_PCS_XF_RX_OVRD_IN_1'].REQ_OVRD_EN=1
        rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, pma_ln,
                  ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataByte);
        dataByte[0] |=  (1 << 3); // REQ_OVRD_EN is bit 3
        dataByte[0] &= ~(1 << 2); // REQ_OVRD_VAL is bit 2
        rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, pma_ln,
                  ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataByte);
        CHECK_SUCCESS(rc);

        //rxvalid = ipid.__dict__['LANE'+str(pma_ln)+'_DIG_ASIC_RX_ASIC_OUT_0'].VALID.value
        //#logger.info("Side:%s, Lane:%02d, PHY rx_valid = %d" % (side, lane, rxvalid))
        rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, pma_ln,
                  ARIES_PMA_LANE_DIG_ASIC_RX_ASIC_OUT_0, dataByte);
        CHECK_SUCCESS(rc);
        uint8_t rxvalid = (dataByte[0] >> 1) & 1; // VALID is bit 1

        if (rxvalid == 0)
        {
            //ipid.__dict__['RAWLANE'+str(pma_ln)+'_DIG_PCS_XF_RX_OVRD_IN_1'].REQ_OVRD_EN=0
            rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, pma_ln,
                      ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataByte);
            dataByte[0] &= ~(1 << 3); // REQ_OVRD_EN is bit 3
            rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, pma_ln,
                      ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataByte);
            CHECK_SUCCESS(rc);
        }
        else
        {
            break;
        }
    }
    // Check for no-adapt case
    if (i == 20)
    {
        ASTERA_INFO("Side:%1d, Lane:%02d, RxValid=0! Confirm link partner transmitter is enabled at the correct data rate.", side, lane);
    }
    //ipid.__dict__['RAWLANE'+str(pma_ln)+'_DIG_PCS_XF_RX_OVRD_IN_1'].REQ_OVRD_EN=1
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, pma_ln,
              ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataByte);
    dataByte[0] |= (1 << 3); // REQ_OVRD_EN is bit 3
    dataByte[0] &= ~(1 << 2); // REQ_OVRD_VAL is bit 2
    rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, pma_ln,
              ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataByte);
    CHECK_SUCCESS(rc);

    //self.csr.__dict__['qs_'+str(qs)].__dict__['pth_wrap_'+str(pth_wrap)].__dict__['ret_pth_ln'+str(ret_ln)].mac_phy_rxstandby=3
    rc = ariesPipeRxStandbySet(device, side, lane, true);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

AriesErrorType ariesPipeFomGet(
        AriesDeviceType* device,
        int side,
        int lane,
        int* fom)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    //fom = self.csr.__dict__['qs_'+str(qs)].__dict__['pth_wrap_'+str(pth_wrap)].__dict__['ret_pth_ln'+str(ret_ln)].phy_mac_fomfeedback.value
    rc = ariesReadRetimerRegister(device->i2cDriver, side, lane,
        ARIES_RET_PTH_LN_PHY_MAC_FOMFEEDBACK_ADDR, 1, dataByte);
    CHECK_SUCCESS(rc);
    fom[0] = dataByte[0];
    return ARIES_SUCCESS;
}

AriesErrorType ariesPipeRxStandbySet(
        AriesDeviceType* device,
        int side,
        int lane,
        bool value)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    if (value == true)
    {
        dataByte[0] = 3; // en=1, val=1
    }
    else
    {
        dataByte[0] = 2; // en=1, val=0
    }
    rc = ariesWriteRetimerRegister(device->i2cDriver, side, lane,
        ARIES_RET_PTH_LN_MAC_PHY_RXSTANDBY_ADDR, 1, dataByte);
    CHECK_SUCCESS(rc);
    return ARIES_SUCCESS;
}

AriesErrorType ariesPipeRxEqEval(
        AriesDeviceType* device,
        int side,
        int lane,
        bool value)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    if (value == true)
    {
        dataByte[0] = 3; // en=1, val=1
    }
    else
    {
        dataByte[0] = 2; // en=1, val=0
    }
    rc = ariesWriteRetimerRegister(device->i2cDriver, side, lane,
        ARIES_RET_PTH_LN_MAC_PHY_RXEQEVAL_ADDR, 1, dataByte);
    CHECK_SUCCESS(rc);
    return ARIES_SUCCESS;
}

AriesErrorType ariesPipePhyStatusClear(
        AriesDeviceType* device,
        int side,
        int lane)
{
    (void)device; (void)side; (void)lane;
    return ARIES_SUCCESS;
}


AriesErrorType ariesPipePhyStatusGet(
        AriesDeviceType* device,
        int side,
        int lane,
        bool* value)
{
    (void)device; (void)side; (void)lane; (void)value;
    return ARIES_SUCCESS;
}


AriesErrorType ariesPipePhyStatusToggle(
        AriesDeviceType* device,
        int side,
        int lane)
{
    (void)device; (void)side; (void)lane;
    return ARIES_SUCCESS;
}

AriesErrorType ariesPipePowerdownSet(
        AriesDeviceType* device,
        int side,
        int lane,
        int value)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    uint8_t lane_base = lane & 0xfe; // for access to ret_pth_gbl register, use a ret_pth_ln offset of 0
    dataByte[0] = (1 << 4) | (value & 0xf); // bit 4 is en
    rc = ariesWriteRetimerRegister(device->i2cDriver, side, lane_base,
        ARIES_RET_PTH_GBL_MAC_PHY_POWERDOWN_ADDR, 1, dataByte);
    CHECK_SUCCESS(rc);

    // Verify
    //rc = ariesReadRetimerRegister(device->i2cDriver, side, lane_base,
    //    ARIES_RET_PTH_GBL_MAC_PHY_POWERDOWN_ADDR, 1, dataByte);
    //CHECK_SUCCESS(rc);
    //ASTERA_INFO("Side: %d, Lane: %d, mac_phy_powerdown = 0x%x", side, lane, dataByte[0]);

    return ARIES_SUCCESS;
}

AriesErrorType ariesPipePowerdownCheck(
        AriesDeviceType* device,
        int side,
        int lane,
        int value)
{
    AriesErrorType rc;
    uint8_t dataByte[2];
    uint8_t qs = floor(lane/4);
    uint8_t qsLane = lane % 4;

    // Read the txX_pstate register
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
            ARIES_PMA_LANE_DIG_ASIC_TX_ASIC_IN_0, dataByte);
    CHECK_SUCCESS(rc);
    uint8_t tx_pstate = (dataByte[0] >> 6) & 0x3; // PSTATE is bits 7:6
    uint8_t tx_pstate_expect = 0;
    if (value == 0) // P0
    {
        tx_pstate_expect = 0;
    }
    else if (value == 2) // P1
    {
        tx_pstate_expect = 2;
    }
    else
    {
        ASTERA_ERROR("Side: %d, Lane: %d, unsupported POWERDOWN value %d!", side, lane, value);
    }
    // check if value matches expectation
    if (tx_pstate != tx_pstate_expect)
    {
        ASTERA_ERROR("Side: %d, Lane: %d, txX_pstate (%d) does not match expected value (%d)!", side, lane, tx_pstate, tx_pstate_expect);
    }

    // Read the rxX_pstate register
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
            ARIES_PMA_LANE_DIG_ASIC_RX_ASIC_IN_0, dataByte);
    CHECK_SUCCESS(rc);
    uint8_t rx_pstate = (dataByte[0] >> 5) & 0x3; // PSTATE is bits 6:5
    uint8_t rx_pstate_expect = 0;
    if (value == 0) // P0
    {
        rx_pstate_expect = 0;
    }
    else if (value == 2) // P1
    {
        rx_pstate_expect = 2;
    }
    else
    {
        ASTERA_ERROR("Side: %d, Lane: %d, unsupported POWERDOWN value %d!", side, lane, value);
    }
    // check if value matches expectation
    if (rx_pstate != rx_pstate_expect)
    {
        ASTERA_ERROR("Side: %d, Lane: %d, rxX_pstate (%d) does not match expected value (%d)!", side, lane, rx_pstate, rx_pstate_expect);
    }

    return ARIES_SUCCESS;
}

AriesErrorType ariesPipeRateChange(
        AriesDeviceType* device,
        int side,
        int lane,
        int rate)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    uint8_t pipeRate;
    uint8_t curPipeRate;
    uint8_t lane_base = lane & 0xfe; // for access to ret_pth_gbl register, use a ret_pth_ln offset of 0
    if ((rate > 5) || (rate < 1))
    {
        ASTERA_ERROR("rate argument must be 1, 2, ..., or 5.");
        return ARIES_INVALID_ARGUMENT;
    }
    pipeRate = rate - 1;
    // Check if rate needs to be updated
    rc = ariesReadRetimerRegister(device->i2cDriver, side, lane_base,
        ARIES_RET_PTH_GBL_MAC_PHY_RATE_AND_PCLK_RATE_ADDR, 1, dataByte);
    CHECK_SUCCESS(rc);
    curPipeRate = dataByte[0] & 0x7; // PIPE rate is 0, 1, .., 4 for Gen1, Gen2, ..., Gen5
    if (curPipeRate != pipeRate)
    {
        //# Put receveiers into standby (both receivers in the path)
        ariesPipeRxStandbySet(device, side, lane_base, true);
        ariesPipeRxStandbySet(device, side, lane_base+1, true);
        usleep(10000); // 10 ms

        //self.pipe_phystatus_clear(pid);

        //# change rate in the controller logic (Note: this is a wide register read)
        //rmlh = self.csr.__dict__['qs_' + str(qs)].__dict__['pth_wrap_' + str(pth_wrap)].ret_pth_gbl.pt_snps_rmlh.value
        //# rate is bits 22:20
        //rmlh = (rmlh & 0x18fffff) | (rate << 20)
        //self.csr.__dict__['qs_' + str(qs)].__dict__['pth_wrap_' + str(pth_wrap)].ret_pth_gbl.pt_snps_rmlh = rmlh

        //# change rate on the PIPE interface
        //self.csr.__dict__['qs_' + str(qs)].__dict__['pth_wrap_' + str(pth_wrap)].ret_pth_gbl.mac_phy_rate_and_pclk_rate = 0x88 | (rate << 4) | rate # Bit 7 and bit 3 are override enable for pclk_rate and rate, respectively
        dataByte[0] = 0x88 | (pipeRate << 4) | (pipeRate & 0xf);
        rc = ariesWriteRetimerRegister(device->i2cDriver, side, lane_base,
            ARIES_RET_PTH_GBL_MAC_PHY_RATE_AND_PCLK_RATE_ADDR, 1, dataByte);
        CHECK_SUCCESS(rc);

        //# Wait for PhyStatus == 1
        //#self.wait_phystatus_toggle([pid])
    }
    else
    {
        ASTERA_INFO("Current rate is Gen%d. Skipping rate change to Gen%d", (curPipeRate+1), rate);
    }
    return ARIES_SUCCESS;
}

AriesErrorType ariesPipeRateCheck(
        AriesDeviceType* device,
        int side,
        int lane,
        int rate)
{
    AriesErrorType rc;
    uint8_t dataByte[2];
    uint8_t qs = floor(lane/4);
    uint8_t qsLane = lane % 4;
    if ((rate > 5) || (rate < 1))
    {
        ASTERA_ERROR("rate argument must be 1, 2, ..., or 5.");
        return ARIES_INVALID_ARGUMENT;
    }

    // Read the txX_rate register
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
            ARIES_PMA_LANE_DIG_ASIC_TX_ASIC_IN_0, dataByte);
    CHECK_SUCCESS(rc);
    uint8_t tx_rate = dataByte[1] & 0x7; // RATE is bits 10:8
    uint8_t tx_rate_expect = 0;
    if (rate == 1) // Gen1
    {
        tx_rate_expect = 3;
    }
    else if (rate == 2) // Gen2
    {
        tx_rate_expect = 2;
    }
    else if (rate == 3) // Gen3
    {
        tx_rate_expect = 2;
    }
    else if (rate == 4) // Gen4
    {
        tx_rate_expect = 1;
    }
    else if (rate == 5) // Gen5
    {
        tx_rate_expect = 0;
    }
    // check if value matches expectation
    if (tx_rate != tx_rate_expect)
    {
        ASTERA_ERROR("Side: %d, Lane: %d, txX_rate (%d) does not match expected value (%d) for Gen%d!", side, lane, tx_rate, tx_rate_expect, rate);
    }

    // Read the rxX_rate register
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
            ARIES_PMA_LANE_DIG_ASIC_RX_ASIC_IN_0, dataByte);
    CHECK_SUCCESS(rc);
    uint8_t rx_rate = (dataByte[0] & 0x1) >> 7;
    rx_rate |= (dataByte[1] & 0x3) << 1; // RATE is bits 9:7
    uint8_t rx_rate_expect = 0;
    if (rate == 1) // Gen1
    {
        rx_rate_expect = 3;
    }
    else if (rate == 2) // Gen2
    {
        rx_rate_expect = 2;
    }
    else if (rate == 3) // Gen3
    {
        rx_rate_expect = 1;
    }
    else if (rate == 4) // Gen4
    {
        rx_rate_expect = 0;
    }
    else if (rate == 5) // Gen5
    {
        rx_rate_expect = 0;
    }
    // check if value matches expectation
    if (rx_rate != rx_rate_expect)
    {
        if (rate != 3) // Suppress this check for Gen3
        {
          ASTERA_ERROR("Side: %d, Lane: %d, rxX_rate (%d) does not match expected value (%d) for Gen%d!", side, lane, rx_rate, rx_rate_expect, rate);
        }
    }

    return ARIES_SUCCESS;
}

AriesErrorType ariesPipeDeepmhasisSet(
        AriesDeviceType* device,
        int side,
        int lane,
        int de,
        int preset,
        int pre,
        int main,
        int pst)
{
    AriesErrorType rc;
    uint8_t dataByte[3];
    uint32_t deemph;
    if (de != ARIES_PIPE_DEEMPHASIS_DE_NONE)
    {
        deemph = de;
        //ASTERA_INFO("Side: %d, Lane: %d, Setting de-emphasis value to 0x%x for Gen1/2.", side, lane, deemph);
    }
    else if (preset != ARIES_PIPE_DEEMPHASIS_PRESET_NONE)
    {
        switch (preset)
        {
            case 4:
                deemph = (48 << 6) | (0 << 0) | (0 << 12);
                break;
            case 1:
                deemph = (40 << 6) | (0 << 0) | (8 << 12);
                break;
            case 0:
                deemph = (36 << 6) | (0 << 0) | (12 << 12);
                break;
            case 9:
                deemph = (40 << 6) | (8 << 0) | (0 << 12);
                break;
            case 8:
                deemph = (36 << 6) | (6 << 0) | (6 << 12);
                break;
            case 7:
                deemph = (34 << 6) | (5 << 0) | (9 << 12);
                break;
            case 5:
                deemph = (44 << 6) | (4 << 0) | (0 << 12);
                break;
            case 6:
                deemph = (42 << 6) | (6 << 0) | (0 << 12);
                break;
            case 3:
                deemph = (42 << 6) | (0 << 0) | (6 << 12);
                break;
            case 2:
                deemph = (38 << 6) | (0 << 0) | (10 << 12);
                break;
            case 10:
                deemph = (32 << 6) | (0 << 0) | (16 << 12);
                break;
            default:
                deemph = (40 << 6) | (0 << 0) | (8 << 12);
        }
        //ASTERA_INFO("Side: %d, Lane: %d, Setting de-emphasis value to 0x%x (Preset %d) for Gen3/4/5.", side, lane, deemph, preset);
    }
    else
    {
        deemph = (main << 6) | (pre << 0) | (pst << 12);
        //ASTERA_INFO("Side: %d, Lane: %d, Setting de-emphasis value to 0x%x (custom).", side, lane, deemph);
    }
    //self.csr.__dict__['qs_' + str(qs)].__dict__['pth_wrap_' + str(pth_wrap)].__dict__['ret_pth_ln'+str(ret_ln)].mac_phy_txdeemph = ((1 << 18) | deemph) # en is bit 18
    dataByte[2] = (1 << 2) | ((deemph >> 16) & 0xff);
    dataByte[1] = ((deemph >> 8) & 0xff);
    dataByte[0] = (deemph & 0xff);
    rc = ariesWriteRetimerRegister(device->i2cDriver, side, lane,
        ARIES_RET_PTH_LN_MAC_PHY_TXDEEMPH_ADDR, 3, dataByte);
    CHECK_SUCCESS(rc);

    // Read back
    //rc = ariesReadRetimerRegister(device->i2cDriver, side, lane,
    //    ARIES_RET_PTH_LN_MAC_PHY_TXDEEMPH_OB_ADDR, 3, dataByte);
    //CHECK_SUCCESS(rc);
    //uint32_t value = (dataByte[2]<<16) | (dataByte[1]<<8) | (dataByte[0]);
    //ASTERA_INFO("Side: %d, Lane: %d, de-emphasis read-back: 0x%x.", side, lane, value);

    return ARIES_SUCCESS;
}

AriesErrorType ariesPipeRxPolaritySet(
        AriesDeviceType* device,
        int side,
        int lane,
        int value)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    //self.csr.__dict__['qs_' + str(qs)].__dict__['pth_wrap_' + str(pth_wrap)].__dict__['ret_pth_ln' + str(ret_ln)].mac_phy_rxpolarity = (1 << 1) | polarity # en is bit 1
    dataByte[0] = (1 << 1) | (value & 1); // bit 1 is enable
    rc = ariesWriteRetimerRegister(device->i2cDriver, side, lane,
        ARIES_RET_PTH_LN_MAC_PHY_RXPOLARITY_ADDR, 1, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

AriesErrorType ariesPipeTxElecIdleSet(
        AriesDeviceType* device,
        int side,
        int lane,
        bool value)
{
    (void)device; (void)side; (void)lane; (void)value;
    return ARIES_SUCCESS;
}

AriesErrorType ariesPipeRxTermSet(
        AriesDeviceType* device,
        int side,
        int lane,
        bool value)
{
    AriesErrorType rc;
    uint8_t dataByte[1];
    //self.csr.__dict__['qs_' + str(qs)].__dict__['pth_wrap_' + str(pth_wrap)].__dict__['ret_pth_ln' + str(ret_ln)].pcs_rx_termination = rx_termination
    dataByte[0] = value ? 1 : 0;
    rc = ariesWriteRetimerRegister(device->i2cDriver, side, lane,
        ARIES_RET_PTH_LN_PCS_RX_TERMINATION_ADDR, 1, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

AriesErrorType ariesPipeBlkAlgnCtrlSet(
        AriesDeviceType* device,
        int side,
        int lane,
        bool value)
{
    AriesErrorType rc;
    uint8_t lane_base = lane & 0xfe; // for access to ret_pth_gbl register, use a ret_pth_ln offset of 0
    uint8_t dataByte[1];
    //self.csr.__dict__['qs_' + str(qs)].__dict__['pth_wrap_' + str(pth_wrap)].ret_pth_gbl.mac_phy_blockaligncontrol = (1 << 1) | blockaligncontrol # en is bit 1
    dataByte[0] = value ? 3 : 2; // bit 1 is enable
    rc = ariesWriteRetimerRegister(device->i2cDriver, side, lane_base,
        ARIES_RET_PTH_GBL_MAC_PHY_BLOCKALIGNCONTROL_ADDR, 1, dataByte);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

AriesErrorType ariesPMABertPatChkSts(
        AriesDeviceType* device,
        int side,
        int lane,
        int* ecount)
{
    AriesErrorType rc;
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;
    int ecountVal = 0;

    qs = lane / 4;
    qsLane = lane % 4;
    // Double-read required
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane, 0x108d, dataWord);
    CHECK_SUCCESS(rc);
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane, 0x108d, dataWord);
    CHECK_SUCCESS(rc);
    ecountVal = (dataWord[1] << 8) | dataWord[0];
    if (ecountVal >= 32768)
    {
        ecountVal = ecountVal >> 1;
        if (ecountVal == (32768 - 1))
        {
            ASTERA_INFO("Side: %d, Lane: %02d, Error Count saturated!", side, lane);
        }
        ecountVal = ecountVal * 128;
    }
    ecount[0] = ecountVal;

    return ARIES_SUCCESS;
}


AriesErrorType ariesPMABertPatChkToggleSync(
        AriesDeviceType* device,
        int side,
        int lane)
{
    AriesErrorType rc;
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;

    qs = lane / 4;
    qsLane = lane % 4;
    // bert_pat_chk_toggle_sync(ipid, lane=pma_ln)
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_RX_LBERT_CTL, dataWord);
    CHECK_SUCCESS(rc);
    // ipid.__dict__['LANE'+lane_str+'_DIG_RX_LBERT_CTL'].SYNC = 0
    dataWord[0] &= ~(1 << 4);
    rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_RX_LBERT_CTL, dataWord);
    CHECK_SUCCESS(rc);
    // ipid.__dict__['LANE'+lane_str+'_DIG_RX_LBERT_CTL'].SYNC = 1
    dataWord[0] |= (1 << 4);
    rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_RX_LBERT_CTL, dataWord);
    CHECK_SUCCESS(rc);
    // ipid.__dict__['LANE'+lane_str+'_DIG_RX_LBERT_CTL'].SYNC = 0
    dataWord[0] &= ~(1 << 4);
    rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_RX_LBERT_CTL, dataWord);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

AriesErrorType ariesPMABertPatChkDetectCorrectPolarity(
        AriesDeviceType* device,
        int side,
        int lane)
{
    AriesErrorType rc;
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;
    int ecount[1];
    int invert_ovrd_en = 0;
    int invert = 0;
    int cur_polarity = 0;
    int new_polarity = 0;

    qs = lane / 4;
    qsLane = lane % 4;

    // Clear the error counter
    rc = ariesPMABertPatChkToggleSync(device, side, lane);
    CHECK_SUCCESS(rc);
    rc = ariesPMABertPatChkToggleSync(device, side, lane);
    CHECK_SUCCESS(rc);

    // Read the error counter
    rc = ariesPMABertPatChkSts(device, side, lane, ecount);
    CHECK_SUCCESS(rc);

    if (ecount[0] == 4194176)
    {
        ASTERA_INFO("Side: %d, Lane: %02d, Invert polarity", side, lane);
        rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
            ARIES_PMA_LANE_DIG_ASIC_RX_OVRD_IN_0, dataWord);
        CHECK_SUCCESS(rc);
        invert_ovrd_en = (dataWord[0] >> 3) & 0x1;
        invert = (dataWord[0] >> 2) & 0x1;
        if (invert_ovrd_en)
        {
            cur_polarity = invert;
        }
        else
        {
            cur_polarity = 0;
        }
        new_polarity = 1 - cur_polarity;
        // Set new polarity
        rc = ariesPMARxInvertSet(device, side, lane, new_polarity, 1);
        CHECK_SUCCESS(rc);
        // CLear error counter
        rc = ariesPMABertPatChkToggleSync(device, side, lane);
        CHECK_SUCCESS(rc);
        rc = ariesPMABertPatChkToggleSync(device, side, lane);
        CHECK_SUCCESS(rc);
    }

    return ARIES_SUCCESS;
}

AriesErrorType ariesPMARxInvertSet(
        AriesDeviceType* device,
        int side,
        int lane,
        bool invert,
        bool override)
{
    AriesErrorType rc;
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;

    qs = lane / 4;
    qsLane = lane % 4;
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
            ARIES_PMA_LANE_DIG_ASIC_RX_OVRD_IN_0, dataWord);
        CHECK_SUCCESS(rc);
    if (invert)
    {
        dataWord[0] |= (1 << 2);
    }
    else
    {
        dataWord[0] &= ~(1 << 2);
    }
    if (override)
    {
        dataWord[0] |= (1 << 3);
    }
    else
    {
        dataWord[0] &= ~(1 << 3);
    }
    rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
            ARIES_PMA_LANE_DIG_ASIC_RX_OVRD_IN_0, dataWord);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

AriesErrorType ariesPMABertPatChkConfig(
        AriesDeviceType* device,
        int side,
        int lane,
        AriesPRBSPatternType mode)
{
    AriesErrorType rc;
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;

    qs = lane / 4;
    qsLane = lane % 4;
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_RX_LBERT_CTL, dataWord);
    CHECK_SUCCESS(rc);
    dataWord[0] &= ~(0xF);
    dataWord[0] |= (mode & 0xF);
    rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_RX_LBERT_CTL, dataWord);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

AriesErrorType ariesPMABertPatGenConfig(
        AriesDeviceType* device,
        int side,
        int lane,
        AriesPRBSPatternType mode)
{
    AriesErrorType rc;
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;

    //ASTERA_INFO("Side: %d, Lane: %d, PRBS generator mode: %d", side, lane, mode);

    qs = lane / 4;
    qsLane = lane % 4;
    // As per datasheet Table 11-340: when changing modes, you must change to disabled (0) first
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_TX_LBERT_CTL, dataWord);
    CHECK_SUCCESS(rc);
    dataWord[0] &= ~(0xF);
    rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_TX_LBERT_CTL, dataWord);
    CHECK_SUCCESS(rc);

    // Skip this redundant read
    //rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
    //    ARIES_PMA_LANE_DIG_TX_LBERT_CTL, dataWord);
    //CHECK_SUCCESS(rc);
    dataWord[0] |= (mode & 0xF);
    rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_TX_LBERT_CTL, dataWord);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

AriesErrorType ariesPMARxDataEnSet(
        AriesDeviceType* device,
        int side,
        int lane,
        bool value)
{
    AriesErrorType rc;
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;

    qs = floor(lane / 4);
    qsLane = lane % 4;
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_ASIC_RX_OVRD_IN_0, dataWord);
    CHECK_SUCCESS(rc);
    if (value == false)
    {
        dataWord[0] &= ~(1 << 4);
        dataWord[0] &= ~(1 << 5);
    }
    else
    {
        dataWord[0] |= (1 << 4);
        dataWord[0] |= (1 << 5);
    }
    rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_ASIC_RX_OVRD_IN_0, dataWord);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

AriesErrorType ariesPMATxDataEnSet(
        AriesDeviceType* device,
        int side,
        int lane,
        bool enable)
{
    AriesErrorType rc;
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;

    qs = lane / 4;
    qsLane = lane % 4;
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_ASIC_TX_OVRD_IN_0, dataWord);
    CHECK_SUCCESS(rc);
    if (enable == false)
    {
        dataWord[0] &= ~(1 << 6); // DATA_EN_OVRD_VAL = 0
        dataWord[0] &= ~(1 << 7); // DATA_EN_OVRD_EN = 0
    }
    else
    {
        dataWord[0] |= (1 << 6); // DATA_EN_OVRD_VAL = 1
        dataWord[0] |= (1 << 7); // DATA_EN_OVRD_EN = 1
    }
    rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_LANE_DIG_ASIC_TX_OVRD_IN_0, dataWord);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}


AriesErrorType ariesPMAPCSRxReqBlock(
        AriesDeviceType* device,
        int side,
        int lane)
{
    AriesErrorType rc;
    uint8_t dataWord[2];
    int qs = 0;
    int qsLane = 0;

    qs = floor(lane / 4);
    qsLane = lane % 4;
    rc = ariesReadWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataWord);
    CHECK_SUCCESS(rc);
    dataWord[0] &= ~(1 << 2);
    dataWord[0] |= (1 << 3);
    rc = ariesWriteWordPmaLaneIndirect(device->i2cDriver, side, qs, qsLane,
        ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1, dataWord);
    CHECK_SUCCESS(rc);

    return ARIES_SUCCESS;
}

#ifdef __cplusplus
}
#endif
