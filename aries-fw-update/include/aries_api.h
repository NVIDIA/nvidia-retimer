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
 * @file aries_api.h
 * @brief Definition of public functions for the SDK.
 */

#ifndef ASTERA_ARIES_SDK_API_H_
#define ASTERA_ARIES_SDK_API_H_

#include "aries_globals.h"
#include "aries_error.h"
#include "aries_i2c.h"
#include "aries_api_types.h"
#include "aries_misc.h"
#include "aries_bifurcation_params.h"

#ifdef ARIES_MPW
#include "aries_mpw_reg_defines.h"
#else
#include "aries_a0_reg_defines.h"
#endif

#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ARIES_SDK_VERSION "2.5"

/**
 * @brief Return the SDK version
 *
 * @return     uint8_t* - SDK version number
 */
const uint8_t* ariesGetSDKVersion(void);

/**
 * @brief Check status of FW loaded
 *
 * Check the code load register and the Main Micro heartbeat. If all
 * is good, then read the firmware version
 *
 * @param[in,out]  device  Aries device struct
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesFWStatusCheck(AriesDeviceType* device);

/**
 * @brief Initialize Aries device
 *
 * Capture the FW version, device id, vendor id and revision id and store
 * the values in the device struct
 *
 * @param[in,out]  device  Aries device struct
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesInitDevice(AriesDeviceType* device);

/**
 * @brief Set the bifurcation mode
 *
 * Bifurcation mode is written to global parameter register 0. Returns a
 * negative error code, else zero on success.
 *
 * @param[in]  i2cDriver  I2C driver responsible for the transaction(s)
 * @param[in]  bifur      Bifurcation mode
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesSetBifurcationMode(AriesDeviceType* device,
                                       AriesBifurcationType bifur);

/**
 * @brief Get the bifurcation mode
 *
 * Bifurcation mode is read from the global parameter register 0. Returns a
 * negative error code, else zero on success.
 *
 * @param[in]  i2cDriver  I2C driver responsible for the transaction(s)
 * @param[in]  bifur      Pointer to bifurcation mode variable
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesGetBifurcationMode(AriesDeviceType* device,
                                       AriesBifurcationType* bifur);

/**
 * @brief Set the PCIe Protocol Reset.
 *
 * PCIe Protocol (fundamental) Reset may be triggered from the PERST_N
 * hardware pin or from a register. This function sets the register to assert
 * PCIe reset (1) or de-assert PCIe reset (0). User must wait 1 ms between
 * an assertion and de-assertion.
 *
 * @param[in]  link  Link object (containing link id)
 * @param[in]  reset      Reset assert (1) or de-assert (0)
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesSetPcieReset(AriesLinkType* link, uint8_t reset);

/**
 * @brief Set the PCIe HW Reset. This function sets the register to assert
 * reset (1) or de-assert reset (0). Asserting reset would set the retimer in
 * reset, and de-asserting it will bring the retimer out of reset and cause
 * a firmware reload. User must wait 1ms between assertion and de-assertion
 *
 * @param[in]  i2cDriver  I2C driver responsible for the transaction(s)
 * @param[in]  reset      Reset assert (1) or de-assert (0)
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesSetPcieHwReset(AriesDeviceType* device, uint8_t reset);

/**
 * @brief Update the FW image in the EEPROM connected to the Retimer.
 *
 * numBytes from the values[] buffer will be written to the EEPROM attached
 * to the Retimer then verified using the optimal method.
 * Returns a negative error code, else zero on success.
 *
 * @param[in]  device   Struct containing device information
 * @param[in]  values   Pointer to byte array containing the data to be
 *                      written to the EEPROM
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesUpdateFirmware(AriesDeviceType* device, char* filename);

/**
 * @brief Load a FW image into the EEPROM connected to the Retimer.
 *
 * numBytes from the values[] buffer will be written to the EEPROM attached
 * to the Retimer. Returns a negative error code, else zero on success.
 *
 * @param[in]  device   Struct containing device information
 * @param[in]  values   Pointer to byte array containing the data to be
 *                      written to the EEPROM
 * @param[in]  legacyMode   If true, write EEPROM in slower legacy mode
                            (set this flag in error scenarios, when you wish to
 write EEPROM without using faster Main Micro assisted writes). Please set to
 false otherwise.
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesWriteEEPROMImage(AriesDeviceType* device, uint8_t* values,
                                     bool legacyMode);

/**
 * @brief Verify the FW image in the EEPROM connected to the Retimer.
 *
 * numBytes from the values[] buffer will be compared against the contents
 * of the EEPROM attached to the Retimer. Returns a negative error code, else
 * zero on success.
 *
 * @param[in]  device  Struct containing device information
 * @param[in]  values     Pointer to byte array containing the expected
 *                        firmware image. The actual contents of the EEPROM
 *                        will be compared against this.
 * @param[in]  numBytes  Number of bytes to compare (<= 256k)
 * @param[in]  legacyMode  If true, write EEPROM in slower legacy mode
                            (set this flag in error scenarios, when you wish to
 read EEPROM without using faster Main Micro assisted read). Please set to false
 otherwise.
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesVerifyEEPROMImage(AriesDeviceType* device, uint8_t* values,
                                      bool legacyMode);

/**
 * @brief Verify the FW image in the EEPROM connected to the Retimer via
 * checksum.
 *
 * In this case, no re-writes happen in case of a failure.
 * It is recommended to attempt the rewrite the FW into the EEPROM again
 * upon failure
 *
 * @param[in]  device  Struct containing device information
 * @param[in]  image     Pointer to byte array containing the expected
 *                        firmware image. The actual contents of the EEPROM
 *                        will be compared against this.
 * @param[in]  numBytes size of FW image (in bytes)
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesVerifyEEPROMImageViaChecksum(AriesDeviceType* device,
                                                 uint8_t* image);

/**
 * @brief Calculate block CRCs from data in EEPROM
 *
 * @param[in]  device  Struct containing device information
 * @param[in, out]  image  Array containing FW info (in bytes)
 * @param[in, out]  numBytes size of image (in bytes)
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesCheckEEPROMCrc(AriesDeviceType* device, uint8_t* image);

/**
 * @brief Calculate block CRCs from data in EEPROM
 *
 * @param[in]  device  Struct containing device information
 * @param[in, out]  crcBytes  Array containing block crc bytes
 * @param[in, out]  numCrcBytes size of crcBytes (in bytes)
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesCheckEEPROMImageCrcBytes(AriesDeviceType* device,
                                             uint8_t* crcBytes,
                                             uint8_t* numCrcBytes);

/**
 * @brief Load a FW image into the EEPROM connected to the Retimer.
 *
 * This method will only write the bytes different between the current loaded
 * image and the new image
 *
 * @param[in]  device  Struct containing device information
 * @param[in]  imageCurrent     Pointer to byte array containing the data in the
 * EEPROM currently
 * @param[in]  sizeCurrent  Size of imageCurrent
 * @param[in]  imageNew     Pointer to byte array containing the data to be
 * written to the EEPROM
 * @param[in]  sizeNew  Size of imageNew
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesWriteEEPROMImageDelta(AriesDeviceType* device,
                                          uint8_t* imageCurrent,
                                          int sizeCurrent, uint8_t* imageNew,
                                          int sizeNew);

/**
 * @brief Read a byte from the EEPROM.
 *
 * Executes the sequence of SMBus transactions to read a single byte from the
 * EEPROM connected to the retimer.
 *
 * @param[in]  i2cDriver    I2C driver responsible for the transaction(s)
 * @param[in]  addr     EEPROM address to read a byte from
 * @param[in,out]  value    Data read from EEPROM (1 byte)
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesReadEEPROMByte(AriesDeviceType* device, int addr,
                                   uint8_t* value);

/**
 * @brief Write a byte to the EEPROM.
 *
 * Executes the sequence of SMBus transactions to write a single byte to the
 * EEPROM connected to the retimer.
 *
 * @param[in]  i2cDriver    I2C driver responsible for the transaction(s)
 * @param[in]  addr     EEPROM address to write a byte to
 * @param[in]  value    Data to write to EEPROM (1 byte)
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesWriteEEPROMByte(AriesDeviceType* device, int addr,
                                    uint8_t* value);

/**
 * @brief Enable self checking in the Sram memory
 *
 * @param[in] device   Pointer to Aries Device struct object
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesMMSRAMCheckStart(AriesDeviceType* device);

/**
 * @brief Get status of self check in the Sram memory
 *
 * @param[in] device   Pointer to Aries Device struct object
 * @param[in, out] status   Pointer to AriesSramMemoryCheck enum
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesMMSRAMCheckStatus(AriesDeviceType* device,
                                      AriesSramMemoryCheckType* status);

/**
 * @brief Check connection health
 *
 * @param[in] device   Pointer to Aries Device struct object
 * @param[in] slaveAddress   Desired Retimer I2C (7-bit) address in case ARP
 * needs to be run
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesCheckConnectionHealth(AriesDeviceType* device,
                                          uint8_t slaveAddress);

/**
 * @brief Perform an all-in-one health check on the device.
 *
 * Check if regular accesses to the EEPROM are working, and if the EEPROM has
 * loaded correctly (i.e. Main Mircocode, Path Microcode and PMA code have
 * loaded correctly).
 *
 * @param[in]  device    Pointer to Aries Device struct object
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesCheckDeviceHealth(AriesDeviceType* device);

/**
 * @brief Perform an all-in-one health check on a given Link.
 *
 * Check critical parameters like junction temperature, Link LTSSM state,
 * and per-lane eye height/width against certain alert thresholds. Update
 * link.linkOkay member (bool).
 *
 * @param[in]  link    Pointer to Aries Link struct object
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesCheckLinkHealth(AriesLinkType* link);

/**
 * @brief Get link recovery counter value
 *
 * @param[in]  link   Link struct created by user
 * @param[out] recoveryCount   Recovery count returned
 * @return     AriesErrorType - Aries error code
 *
 */
AriesErrorType ariesGetLinkRecoveryCount(AriesLinkType* link,
                                         int* recoveryCount);

/**
 * @brief Clear link recovery counter value
 *
 * @param[in]  link   Link struct created by user
 * @return     AriesErrorType - Aries error code
 *
 */
AriesErrorType ariesClearLinkRecoveryCount(AriesLinkType* link);

/**
 * @brief Get the max recorded junction temperature.
 *
 * Read the maximum junction temperature and return in units of degrees
 * Celsius. This value represents the maximum value from all temperature
 * sensors on the Retimer. Returns a negative error code, else zero on
 * success.
 *
 * @param[in]  device  Struct containing device information
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesGetMaxTemp(AriesDeviceType* device);

/**
 * @brief Get the current junction temperature.
 *
 * Read the current junction temperature and return in units of degrees
 * Celsius.
 *
 * @param[in]  device  Struct containing device information
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesGetCurrentTemp(AriesDeviceType* device);

/**
 * @brief Get the current detailed Link state, including electrical parameters.
 *
 * Read the current Link state and return the parameters for the current links
 * Returns a negative error code, else zero on success.
 *
 * @param[in,out]  link   Pointer to Aries Link struct object
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesGetLinkState(AriesLinkType* link);

/**
 * @brief Get the current detailed Link state, including electrical parameters.
 *
 * Read the current Link state and return the parameters for the current links
 * Returns a negative error code, else zero on success.
 *
 * @param[in,out]  link   Pointer to Aries Link struct object
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesGetLinkStateDetailed(AriesLinkType* link);

/**
 * @brief Initialize LTSSM logger.
 *
 * Configure the LTSSM logger for one-batch or continuous mode and set the
 * print classes (and per-Link enables for main microcontroller log).
 * Returns a negative error code, else zero on success.
 *
 * @param[in]  link          Pointer to Aries Link struct object
 * @param[in]  oneBatchMode  Enable one-batch mode (1) or continuous mode (0)
 * @param[in]  verbosity     Logger verbosity control
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesLTSSMLoggerInit(AriesLinkType* link, uint8_t oneBatchMode,
                                    AriesLTSSMVerbosityType verbosity);

/**
 * @brief Enable or disable LTSSM logger.
 *
 * @param[in]  link     Pointer to Aries Link struct object
 * @param[in]  printEn  Enable (1) or disable (0) printing for this Link
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesLTSSMLoggerPrintEn(AriesLinkType* link, uint8_t printEn);

/**
 * @brief Read an entry from the LTSSM logger.
 *
 * The LTSSM log entry starting at offset is returned, and offset
 * is updated to point to the next entry. Returns a negative error code,
 * including ARIES_LTSSM_INVALID_ENTRY if the the end of the log is reached,
 * else zero on success.
 *
 * @param[in]      link    Pointer to Aries Link struct object
 * @param[in]      log     The specific log to read from
 * @param[in,out]  offset  Pointer to the log offset value
 * @param[out]     entry   Pointer to Aries LTSSM Logger Entry struct returned
 *                         by this function
 * @return         AriesErrorType - Aries error code
 */
AriesErrorType ariesLTSSMLoggerReadEntry(AriesLinkType* link,
                                         AriesLTSSMLoggerEnumType log,
                                         int* offset,
                                         AriesLTSSMEntryType* entry);

/**
 * @brief Set max data rate
 *
 * @param[in]  device  Struct containing device information
 * @param[in]  rate  Max data rate to set
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesSetMaxDataRate(AriesDeviceType* device,
                                   AriesMaxDataRateType rate);

/**
 * @brief Set the GPIO value
 *
 * @param[in]  device  Struct containing device information
 * @param[in]  gpioNum  GPIO number [0:3]
 * @param[in]  value  GPIO value (0) or (1)
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesSetGPIO(AriesDeviceType* device, int gpioNum, bool value);

/**
 * @brief Get the GPIO value
 *
 * @param[in]  device  Struct containing device information
 * @param[in]  gpioNum  GPIO number [0:3]
 * @param[in, out]  value  Pointer to GPIO value (0) or (1) variable
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesGetGPIO(AriesDeviceType* device, int gpioNum, bool* value);

/**
 * @brief Toggle the GPIO value
 *
 * @param[in]  device  Struct containing device information
 * @param[in]  gpioNum  GPIO number [0:3]
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesToggleGPIO(AriesDeviceType* device, int gpioNum);

/**
 * @brief Set the GPIO direction
 *
 * @param[in]  device  Struct containing device information
 * @param[in]  gpioNum  GPIO number [0:3]
 * @param[in]  value  GPIO direction (0 = output) or (1 = input)
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesSetGPIODirection(AriesDeviceType* device, int gpioNum,
                                     bool value);

/**
 * @brief Get the GPIO direction
 *
 * @param[in]  device  Struct containing device information
 * @param[in]  gpioNum  GPIO number [0:3]
 * @param[in, out]  value  Pointer to GPIO direction (0 = output) or (1 = input)
 * variable
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesGetGPIODirection(AriesDeviceType* device, int gpioNum,
                                     bool* value);

/**
 * @brief Enable Aries Test Mode for PRBS generation and checking
 *
 * @param[in]  device  Struct containing device information
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesTestModeEnable(AriesDeviceType* device);

/**
 * @brief Disable Aries Test Mode for PRBS generation and checking
 *
 * @param[in]  device  Struct containing device information
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesTestModeDisable(AriesDeviceType* device);

/**
 * @brief Set the desired Aries Test Mode data rate
 *
 * @param[in]  device  Struct containing device information
 * @param[in]  rate  Desired rate (1, 2, ... 5)
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesTestModeRateChange(AriesDeviceType* device,
                                       AriesMaxDataRateType rate);

/**
 * @brief Aries Test Mode transmitter configuration
 *
 * @param[in]  device  Struct containing device information
 * @param[in]  pattern  Desired PRBS data pattern
 * @param[in]  preset  Desired Tx preset setting
 * @param[in]  enable  Enable (1) or disable (0) flag
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesTestModeTxConfig(AriesDeviceType* device,
                                     AriesPRBSPatternType pattern, int preset,
                                     bool enable);

/**
 * @brief Aries Test Mode receiver configuration
 *
 * @param[in]  device  Struct containing device information
 * @param[in]  pattern  Desired PRBS data pattern
 * @param[in]  enable  Enable (1) or disable (0) flag
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesTestModeRxConfig(AriesDeviceType* device,
                                     AriesPRBSPatternType pattern, bool enable);

/**
 * @brief Aries Test Mode read error count
 *
 * @param[in]  device  Struct containing device information
 * @param[in, out]  ecount  Array containing error count data for each side and
 * lane
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesTestModeRxEcountRead(AriesDeviceType* device, int* ecount);

/**
 * @brief Aries Test Mode clear error count
 *
 * Reads ecount values for each side and lane and populates an array
 *
 * @param[in]  device  Struct containing device information
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesTestModeRxEcountClear(AriesDeviceType* device);

/**
 * @brief Aries Test Mode read FoM
 *
 * Reads FoM values for each side and lane and populates an array
 *
 * @param[in]  device  Struct containing device information
 * @param[in, out]  fom  Array containing FoM data for each side and lane
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesTestModeRxFomRead(AriesDeviceType* device, int* fom);

/**
 * @brief Aries Test Mode read Rx valid
 *
 * @param[in]  device  Struct containing device information
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesTestModeRxValidRead(AriesDeviceType* device);

/**
 * @brief Aries Test Mode inject error
 *
 * @param[in]  device  Struct containing device information
 * @return     AriesErrorType - Aries error code
 */
AriesErrorType ariesTestModeTxErrorInject(AriesDeviceType* device);

#ifdef __cplusplus
}
#endif

#endif /* ASTERA_ARIES_SDK_API_H_ */
