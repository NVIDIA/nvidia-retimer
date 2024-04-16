/**
 * Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#pragma once

#include "config.h"
#include "updateRetimerFw_dbus_log_event.h"
#include <stddef.h>
#include <stdbool.h>

#define UPDATE_STATUS 0x0F
#define FPGA_READ 0x1
#define FPGA_WRITE 0x0
#define RETIMER_MAX_NUM 8
#define RETIMER_EEPROM_WRITE 0
#define RETIMER_EEPROM_READ 1
#define BYTE_PER_PAGE 256
#define MAX_NAME_SIZE 255
#define INIT_UINT8 0xFF
#define MAX_RETRY_WRITE_FLOCK 100
#define UPDATE_PROGRESS 0
#define UPDATE_NACKERR 0
#define VERIFICATION_NACKERR 0
#define MAX_UPDATE_RETRYCOUNT 2
#define MAX_TIMEOUT_SEC 60
#define DELAY_1SEC 1000000
#define DELAY_1MS 1000
#define FW_UPDATE_COMPLETE_FLAG 0x00

#define GPU_BASE1_PRSNT_N_MASK 0x1
#define GPU_BASE1_CPLD_READY_MASK 0x4

// WRITE_BUF_SIZE consist of WRITE_ADDRESS+FW_IMAGE_SIZE
#define WRITE_BUF_SIZE (MAX_FW_IMAGE_SIZE + 4)
#define READ_BUF_SIZE 4
#define HOST_BMC_FPGA_I2C_BUS_NUM 12
#define HMC_FPGA_I2C_BUS_NUM 3

// FPGA Register
#define FPGA_IMG_SIZE_REG 0x040000
#define FPGA_CHKSUM_REG 0x040004
#define FPGA_UPDATE_STATUS_REG 0x040008
#define FPGA_READ_STATUS_REG 0x04000C

// BYTE MASK for FPGA Register
#define BYTE0 0x000000FF
#define BYTE1 0x0000FF00
#define BYTE2 0x00FF0000
#define BYTE3 0xFF000000

// NIBBLE for mask
#define NIBBLE 0xF

// MASK for FW READ operation
#define FW_READ_STATUS_MASK 0x1
#define FW_READ_NACK_MASK 0x1
#define SET_RETIMER_FW_READ 0x1

// READ and WRITE BYTE Count
#define W_BYTE_COUNT_WITHPAYLOAD 7
#define W_BYTE_COUNT 3
#define R_BYTE_COUNT 4

// CPLD related MACRO
#define CPLD_I2C_BUS 2
#define CPLD_SLAVE_ID 0x3c
#define CPLD_GB_OFFSET 0x2b

// FPGA Secondary RegTBL slave address for extended error reporting
#define FPGA_SECONDARY_REGTBL 0x31
#define FPGA_SEC_REGTBL_FWCONTROLLER_OFFSET 0x4B
#define HMC_I2CBUS_FPGA_SEC_REGTBL 0x2
#define EXTENDED_ERR_MAX_PAGE_SZ 256
#define NO_ERR 0x0
#define GLOBAL_WP_L_MASK 0x10
#define RET_MUX_SEL_MASK 0x0F
#define UNKNOWN_ERROR "Unknown Error"

#define VERSION_LEN 10
#define INVALID -1

// Default Version
#define DEFAULT_VERSION "Unknown"
#define MSG_REG_DEV_FOLLOWED_BY_VER 0
#define MSG_REG_VER_FOLLOWED_BY_DEV 1

// ERROR DEFINITION
enum { ERROR_INPUT_ARGUMENTS = 100,
       ERROR_INPUT_I2C_ARGUMENT,
       ERROR_INPUT_CKS_ARGUMENT,
       ERROR_OPEN_FIRMWARE = 105,
       ERROR_WRONG_FIRMWARE,
       ERROR_WRONG_CRC32_CHKSM,
       ERROR_MALLOC_FAILURE,
       ERROR_OPEN_I2C_DEVICE,
       ERROR_IOCTL_I2C_RDWR_FAILURE = 110,
       ERROR_PROG_BUF_CHECKSUM_ERROR,
       ERROR_PROG_READ_CHECKSUM_ERROR,
       ERROR_PROG_OVER_THREE_TIMES,
       ERROR_CHECKERR_OVER_THREE_TIMES,
       ERROR_TRANS_BLOCK,
       ERROR_TRANS_PAGE,
       ERROR_FPGA_NOT_READY,
       ERROR_RETIMER_NOT_READY,
       // FW UPDATE ERROR
       ERROR_WRITE_NACK = 0x200,
       ERROR_UPG_NACK_RETIMER0 = 0x200,
       ERROR_UPG_NACK_RETIMER1 = 0x201,
       ERROR_UPG_NACK_RETIMER2 = 0x202,
       ERROR_UPG_NACK_RETIMER3 = 0x203,
       ERROR_UPG_NACK_RETIMER4 = 0x204,
       ERROR_UPG_NACK_RETIMER5 = 0x205,
       ERROR_UPG_NACK_RETIMER6 = 0x206,
       ERROR_UPG_NACK_RETIMER7 = 0x207,
       ERROR_UPG_NACK_RETIMER_ALL = 0x208,
       // FW READ ERROR
       ERROR_READ_NACK = 0x300,
       ERROR_READ_NACK_RETIMER0 = 0x300,
       ERROR_READ_NACK_RETIMER1 = 0x301,
       ERROR_READ_NACK_RETIMER2 = 0x302,
       ERROR_READ_NACK_RETIMER3 = 0x303,
       ERROR_READ_NACK_RETIMER4 = 0x304,
       ERROR_READ_NACK_RETIMER5 = 0x305,
       ERROR_READ_NACK_RETIMER6 = 0x306,
       ERROR_READ_NACK_RETIMER7 = 0x307,
       ERROR_CHECKSUM = 0x400,
       ERROR_UPG_CRC_RETIMER0 = 0x400,
       ERROR_UPG_CRC_RETIMER1 = 0x401,
       ERROR_UPG_CRC_RETIMER2 = 0x402,
       ERROR_UPG_CRC_RETIMER3 = 0x403,
       ERROR_UPG_CRC_RETIMER4 = 0x404,
       ERROR_UPG_CRC_RETIMER5 = 0x405,
       ERROR_UPG_CRC_RETIMER6 = 0x406,
       ERROR_UPG_CRC_RETIMER7 = 0x407,
       ERROR_UPG_CRC_RETIMER_ALL = 0x408,
       ERROR_UNKNOWN = 0xff,
};

enum RETIMER_MASK {
	RETIMER0 = 0x01,
	RETIMER1 = 0x02,
	RETIMER2 = 0x04,
	RETIMER3 = 0x08,
	RETIMER4 = 0x10,
	RETIMER5 = 0x20,
	RETIMER6 = 0x40,
	RETIMER7 = 0x80,
	RETIMERALL = 0xFF,
};

/**
 * @brief *
 * Enumeration of command for FPGA update and read operation
 */
typedef enum command {
	RETIMER_FW_UPDATE = 0x0, /**< To update FW */
	RETIMER_FW_READ = 0x1, /**< To read Retimer FW */
} RetimerFWCommand;

/**
* @brief *
* structure for I2C Transaction based Error Codes for Retimer FW update 
* from FPGA INTERNAL ARCHITECTURE SPECIFICATION
* RETx_EEPROM_UPDATE_I2C_ERROR_ADDR & RETx_EEPROM_UPDATE_I2C_ERROR_CODE
* 
**/
typedef struct {
	uint8_t RET_EEPROM_I2C_ERROR_ADDR;
	uint8_t RET_EEPROM_I2C_ERROR_CODE;
} RetimerAddrErrorCode;

/**
* @brief *
* structure to map RegTBL register for Retimer FW update
* refer Vulcan I2C Request Form - Google Sheets for details
**/
typedef struct {
	RetimerAddrErrorCode AddrErrorCode[8];
	uint8_t globalWp;
	uint8_t retimerEEPROMmuxSel;
} extendedErrorCode;

typedef struct pair {
	uint8_t errorCode;
	char *errorString;
} ErrorCodeMapTable;

void debug_print(char *fmt, ...);
void prepareMessageRegistry(uint8_t retimer, char *message,
			    bool verBeforeDevice, char *severity,
			    char *resolution, bool genericMessage);
unsigned int crc32(const unsigned char *buf, int length);
int send_i2c_cmd(int fd, int isRead, unsigned char slaveId,
		 unsigned char *write_data, unsigned char *read_data,
		 unsigned int write_count, unsigned int read_count);
int checkExtenedErrorReg();
void genericMessageRegistry(char *message, char *arg0, char *arg1,
			    char *severity, char *resolution);
int maperrnoToI2CError(int errnoval, unsigned char slaveId, char **msg,
		       char **resolution);
int checkDigit_i2c(char *str);
int checkDigit_retimer(char *str);
int parseStr(const char *in, int startid, int endid, char *op);
int readFwVersion(char *str, char **ver);
int copyImageToFpga(int fw_fd, int fd, unsigned int slaveId);
int copyImageFromFpga(int fw_fd, int fd, unsigned int slaveId);
int checkReadNackError(uint8_t status, const uint8_t mask[], uint8_t *retimer);
int checkWriteNackError(uint8_t status, const uint8_t mask[], uint8_t *retimer);
int checkChecksumError(uint8_t status, const uint8_t mask[], uint8_t *retimer);
int startRetimerFwUpdate(int fd, uint8_t retimerNumber,
			 uint8_t *retimerNotUpdated);
int readRetimerfw(int fd, uint8_t retimerNumber);
