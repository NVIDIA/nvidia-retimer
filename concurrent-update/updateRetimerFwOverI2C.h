/*****************************************************************
 *****************************************************************
 ***                                                            **
 ***    (C)Copyright 2022 NVDIA             				    **
 ***                                                            **
 ***            All Rights Reserved.                            **
 ***                                                            **
 *****************************************************************
 *****************************************************************
 ******************************************************************
 *
 * updateRetimerFwOverI2C.h

 * Execute the remote PSU update.
 *
 ******************************************************************/
 
#ifndef _UPDATERETIMERFWOVERI2C_H_
#define _UPDATERETIMERFWOVERI2C_H_

#define UPDATE_STATUS	0x0F
#define FPGA_READ       0x1
#define FPGA_WRITE      0x0
#define RETIMER_MAX_NUM	8
#define FPGA_I2C_CNTRL_ADDR	0x62
#define RETIMER_EEPROM_WRITE 	0
#define RETIMER_EEPROM_READ  	1
#define BYTE_PER_PAGE   256
#define BUSY_ERR	0x10
#define MAX_NAME_SIZE 255
#define INIT_UINT8 0xFF
#define MAX_RETRY_WRITE_FLOCK 100
#define UPDATE_PROGRESS 0
#define UPDATE_NACKERR  0
#define VERIFICATION_NACKERR 0
#define MAX_UPDATE_RETRYCOUNT 2
#define MAX_TIMEOUT     60
#define DELAY_1SEC      1000000
#define DELAY_1MS       1000
#define BUSY_FLAG       0xFF
#define FW_UPDATE_COMPLETE_FLAG 0x00

#define GPU_BASE1_PRSNT_N_MASK 0x1
#define GPU_BASE1_CPLD_READY_MASK 0x4

#define FW_IMAGE_SIZE 256*1024
#define WRITE_BUF_SIZE FW_IMAGE_SIZE+4
#define READ_BUF_SIZE 4
#define ERROR_WRITE_NACK 0x200
#define ERROR_READ_NACK 0x300
#define ERROR_CHECKSUM 0x400

#define HOST_BMC_FPGA_I2C_BUS_NUM 12
#define HMC_FPGA_I2C_BUS_NUM 3

// FPGA Register 
#define FPGA_IMG_SIZE_REG 		0x040000
#define FPGA_CHKSUM_REG 		0x040004
#define FPGA_UPDATE_STATUS_REG 	0x040008
#define FPGA_READ_STATUS_REG 	0x04000C

// NIBBLE MASK for FPGA Register
#define NIBBLE0 0x000000FF
#define NIBBLE1 0x0000FF00
#define NIBBLE2 0x00FF0000
#define NIBBLE3 0xFF000000

// MASK for FW READ operation
#define FW_READ_STATUS_MASK 0x1
#define FW_READ_NACK_MASK 0x1
#define SET_RETIMER_FW_READ 0x1

// CPLD related MACRO
#define CPLD_I2C_BUS 2
#define CPLD_SLAVE_ID 0x3c
#define CPLD_GB_OFFSET 0x2b

#define VERSION_LEN 10
#define INVALID -1



// ERROR DEFINITION
enum {
	ERROR_INPUT_ARGUMENTS = 100,
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
	ERROR_UPG_NACK_RETIMER0 = 200,
	ERROR_UPG_NACK_RETIMER1 = 201,
	ERROR_UPG_NACK_RETIMER2 = 202,
	ERROR_UPG_NACK_RETIMER3 = 203,
	ERROR_UPG_NACK_RETIMER4 = 204,
	ERROR_UPG_NACK_RETIMER5 = 205,
	ERROR_UPG_NACK_RETIMER6 = 206,
	ERROR_UPG_NACK_RETIMER7 = 207,
	ERROR_UPG_NACK_RETIMER_ALL = 208,
	ERROR_UPG_CRC_RETIMER0 = 300,
	ERROR_UPG_CRC_RETIMER1 = 301,
	ERROR_UPG_CRC_RETIMER2 = 302,
	ERROR_UPG_CRC_RETIMER3 = 303,
	ERROR_UPG_CRC_RETIMER4 = 304,
	ERROR_UPG_CRC_RETIMER5 = 305,
	ERROR_UPG_CRC_RETIMER6 = 306,
	ERROR_UPG_CRC_RETIMER7 = 307,
	ERROR_UPG_CRC_RETIMER_ALL = 308,
    // FW READ ERROR
	ERROR_READ_NACK_RETIMER0 = 400,
	ERROR_READ_NACK_RETIMER1 = 401,
	ERROR_READ_NACK_RETIMER2 = 402,
	ERROR_READ_NACK_RETIMER3 = 403,
	ERROR_READ_NACK_RETIMER4 = 404,
	ERROR_READ_NACK_RETIMER5 = 405,
	ERROR_READ_NACK_RETIMER6 = 406,
	ERROR_READ_NACK_RETIMER7 = 407,
	ERROR_UNKNOW = 0xff,
};

enum RETIMER_MASK 
{
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
typedef enum command
{
    RETIMER_FW_UPDATE = 0x0,   /**< To update FW */
    RETIMER_FW_READ = 0x1,   /**< To read Retimer FW */
} RetimerFWCommand;

#endif /* _UPDATERETIMERFWOVERI2C_H_ */
