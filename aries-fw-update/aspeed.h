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
 * @file aspeed.h
 * @brief Implementation of helper functions used by A-Speed
 */

#ifndef ASTERA_ARIES_SDK_ASPEED_H_
#define ASTERA_ARIES_SDK_ASPEED_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/i2c-dev.h>   // linux library included on A-Speed
#include <i2c/smbus.h>

/**
 * @brief: Set I2C slave address
 *
 * @param[in] file      I2C handle
 * @param[in] address   Slave address
 * @param[in] force     Override user provied slave address with default
                         I2C_SLAVE address
 * @return int           Zero if success, else a negative value
 */
int setSlaveAddress(int file, int address, int force);

/**
 * @brief: Close I2C connection
 *
 * @param[in] file      I2C handle
 */
void closeI2CConnection(int file);

#endif /* ASTERA_ARIES_SDK_ASPEED_H_ */
