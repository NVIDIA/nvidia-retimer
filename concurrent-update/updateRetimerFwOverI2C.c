/**
 * Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <systemd/sd-bus.h>
#include "updateRetimerFwOverI2C.h"

char *versionStr = NULL;
const uint8_t mask_retimer[] = { RETIMER0, RETIMER1, RETIMER2,
				 RETIMER3, RETIMER4, RETIMER5,
				 RETIMER6, RETIMER7, RETIMERALL };

uint8_t verbosity = 0;

char *arrRetimer[] = { "PCIeRetimer0Firmware", "PCIeRetimer1Firmware",
		       "PCIeRetimer2Firmware", "PCIeRetimer3Firmware",
		       "PCIeRetimer4Firmware", "PCIeRetimer5Firmware",
		       "PCIeRetimer6Firmware", "PCIeRetimer7Firmware",
		       };

uint8_t retimerNum = INIT_UINT8;

void debug_print(char *fmt, ...)
{
	if (verbosity) {
		va_list args;
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	}
}

/***********************************************************************
 * 
 * prepareMessageRegistry()
 * 
 * Wrappper on top of emitLogMessage.
 * Message registry is updated based on passed Retimer,successfully updated retimer and failed Retimer
 *
 * RETURN: void 
 **********************************************************************/
void prepareMessageRegistry(uint8_t retimer, char *message,
			    bool VerBeforeDevice, char *severity,
			    char *resolution)
{
	if (retimer) {
		for (uint8_t index = 0; index < 8; index++) {
			if (retimer & 1) {
				if (VerBeforeDevice) {
					emitLogMessage(message, versionStr,
						       arrRetimer[index],
						       severity, resolution);
				} else {
					emitLogMessage(message,
						       arrRetimer[index],
						       versionStr, severity,
						       resolution);
				}
			}
			retimer = retimer >> 1;
		}
	}
}

/***********************************************
 *
 *	CRC order: 32
 *	CRC polynom (hex) : 0x04c11db7
 *	Initial value (hex) : 0xFFFFFFFF
 *	Final XOR value (hex) : 0
 *
 ***********************************************/
unsigned int crc32_table[] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

/************************************************
 * crc32()
 *
 * CRC32 checksum command function
 *
 * buf: FW image
 * length: image length
 *
 * RETURN: crc 32 checksum
 ************************************************/
unsigned int crc32(const unsigned char *buf, int length)
{
	const unsigned char *cp = buf;
	unsigned int crc = 0xFFFFFFFF;
	if (cp == NULL) {
		fprintf(stderr, "In CRC32 computation, buf is empty \n");
		return 0xFFFFFFFF;
	}

	while (length--)
		crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *cp++) & 255];
	fprintf(stdout, "In CRC32 computation, crc32: 0x%x \n", crc);
	return crc;
}

/**************************************************************
 * send_i2c_cmd()
 *
 * I2C command function
 *
 * fd: file describe
 * isRead: 1 for FPGA_READ behavior; 0 for FPGA_WRITE
 * write_data: write data
 * read_data: read data
 * write_count: write count
 * read_count: read count
 *
 * RETURN: 0 if success
 *****************************************************************/
int send_i2c_cmd(int fd, int isRead, unsigned char slaveId,
		 unsigned char *write_data, unsigned char *read_data,
		 unsigned int write_count, unsigned int read_count)
{
	struct i2c_rdwr_ioctl_data rdwr_msg;
	struct i2c_msg msg[2];
	int ret;

	memset(&rdwr_msg, 0, sizeof(rdwr_msg));
	memset(&msg, 0, sizeof(msg));

	debug_print("%d %x %d \n", write_count, slaveId, slaveId);
	if (isRead) {
		debug_print("R[0x%x] \n\n", slaveId);
		if (!write_data || !read_data) {
			fprintf(stderr,
				"In send_i2c_cmd read command, read_data,write_data empty \n");
			return -1;
		}
		msg[0].addr = (slaveId);
		msg[0].flags = 0;
		msg[0].len = write_count;
		msg[0].buf = write_data;

		msg[1].addr = (slaveId);
		msg[1].flags = I2C_M_RD;
		msg[1].len = read_count;
		msg[1].buf = read_data;

		rdwr_msg.msgs = msg;
		rdwr_msg.nmsgs = 2;
	} else {
		//-	Write 3 bytes at DPram location 0x02_ABCD
		// Start -> 0x62 (7bits) + w -> 0x02 -> 0xAB -> 0xCD -> wdata1 -> wdata2 -> wdata3-> Stop

		debug_print("W[0x%x]  write_count 0x%x \n\n", slaveId,
			    write_count);
		if (!write_data) {
			fprintf(stderr,
				"In send_i2c_cmd write command, write_data is empty \n");
			return -1;
		}
		msg[0].addr = (slaveId);
		msg[0].flags = 0;
		msg[0].len = write_count;
		msg[0].buf = write_data;

		rdwr_msg.msgs = msg;
		rdwr_msg.nmsgs = 1;
	}
	if ((ret = ioctl(fd, I2C_RDWR, &rdwr_msg)) < 0) {
		fprintf(stderr, "ret:%d  error %s \n", ret, strerror(errno));
		return -ERROR_IOCTL_I2C_RDWR_FAILURE;
	}

	return 0;
}

/******************************************************
 * checkDigit_i2c()
 *
 * confirm if input i2c device is number between 1 ~ 12
 *
 * str: input string
 *
 *
 * RETURN: 0 if success
 *****************************************************/
int checkDigit_i2c(char *str)
{
	uint8_t i;
	if ((atoi(str) >= 1) && (atoi(str) <= 12)) {
		for (i = 0; i < strlen(str); i++) {
			if (isdigit(str[i]) == 0) {
				fprintf(stderr, "I2C Bus: Out of Range\n");
				return 1;
			}
		}
		return 0;
	} else {
		fprintf(stderr, "I2C Bus: Out of Range\n");
		return 1;
	}
}

/*****************************************************
 * checkDigit_retimer()
 *
 * confirm if input retimer is number between 0 ~ 8
 *
 * str: input string
 *
 *
 * RETURN: 0 if success
 *****************************************************/
int checkDigit_retimer(char *str)
{
	uint8_t i;
	if (atoi(str) > 8) {
		fprintf(stderr, "Retimer number Out of Range\n");
		return 1;
	}

	for (i = 0; i < strlen(str); i++) {
		if (isdigit(str[i]) == 0) {
			fprintf(stderr, "Retimer number Out of Range\n");
			return 1;
		}
	}

	return 0;
}

/********************************************************************
 * copyImageToFpga()
 *
 * Load FW image file
 * Calculate CRC32 and update CRC32 value in FPGA control register
 * Copy FW image from FILE to DPRAM
 *
 * imageFilename: FW image name
 * fd: file describe
 * slaveId: FPGA I2C controller slave id
 *
 * RETURN: 0 if success
 ********************************************************************/

int copyImageToFpga(unsigned int fw_fd, unsigned int fd, unsigned int slaveId)
{
	struct stat st;
	unsigned char *fw_buf = NULL;
	unsigned int fw_crc32 = 0;
	int ret = -1;
	unsigned char write_buffer[WRITE_BUF_SIZE] = { 0 };
	unsigned char read_buffer[READ_BUF_SIZE] = { 0 };
	unsigned int pageCount = 0;

	if (fstat(fw_fd, &st)) {
		fprintf(stderr, "\nfstat error: [%s]\n", strerror(errno));
		close(fw_fd);
		return -1;
	}
	/* Check FW image size */
	/* fw_size must be mutiple of BYTE_PER_PAGE */
	if ((st.st_size <= 0) || (st.st_size % BYTE_PER_PAGE)) {
		fprintf(stderr, "\nNot a valid size: [%s]\n", strerror(errno));
		close(fw_fd);
		return -ERROR_WRONG_FIRMWARE;
	}

	fw_buf = (unsigned char *)malloc(st.st_size);

	if (fw_buf == NULL) {
		return -ERROR_MALLOC_FAILURE;
	}

	ret = read(fw_fd, fw_buf, st.st_size);
	if (ret < 0) {
		fprintf(stderr, "ret:%d  unable to read FW file error %s \n",
			ret, strerror(errno));
		close(fw_fd);
		return -ERROR_OPEN_FIRMWARE;
	}

	//close(fw_fd);

	// calculate CRC32
	fw_crc32 = crc32(fw_buf, st.st_size);

	// 5. Copy FW image to FPGA DP RAM 0x0_0000
	fprintf(stdout, "Initiate Copy to FPGA RAM...\n");
	fprintf(stdout, "RETIMER FW Image size: 0x%lx \n",
		(long int)st.st_size);

	pageCount = ((unsigned int)st.st_size / BYTE_PER_PAGE);
	//Copy FW image to FPGA DP RAM 0x0_0000
	//Write to DPRAM address to write_buffer0, write_buffer1, write_buffer 2 and then 256 bytes of payloadtill pageCount
	for (uint32_t i = 0; i < pageCount; i++) {
		memset(write_buffer, 0x00, sizeof(write_buffer));
		memset(read_buffer, 0x00, sizeof(read_buffer));
		write_buffer[0] = (0x00 | (i & 0xFF00) >> 8);
		write_buffer[1] = (0x00 | (i & 0x00FF));
		write_buffer[2] = 0x00;
		memcpy(&write_buffer[3], fw_buf + (i * BYTE_PER_PAGE),
		       BYTE_PER_PAGE);
		ret = send_i2c_cmd(fd, FPGA_WRITE, slaveId, write_buffer,
				   read_buffer, BYTE_PER_PAGE + 3, 1);
		if (ret) {
			fprintf(stderr,
				"FW update FPGA_WRITE failed write_buffer: 0x%x 0x%x 0x%x\n",
				write_buffer[0], write_buffer[1],
				write_buffer[2]);
			free(fw_buf);
			return ret;
		}
	}
	fprintf(stdout, "Image copy to FPGA completed 0x%x \n", read_buffer[0]);
	free(fw_buf);

	// 6. Copy Image size to 0x04_0000
	fprintf(stdout, " Copy Image size...\n");
	memset(write_buffer, 0x00, sizeof(write_buffer));
	memset(read_buffer, 0x00, sizeof(read_buffer));
	//Write to FPGA register @ 0x02_ABCD-> 0x02(BYTE2) -> 0xAB(BYTE1) -> 0xCD(BYTE0)
	write_buffer[0] = ((FPGA_IMG_SIZE_REG & BYTE2) >> 16); //0x04;
	write_buffer[1] = ((FPGA_IMG_SIZE_REG & BYTE1) >> 8); //0x00;
	write_buffer[2] = ((FPGA_IMG_SIZE_REG & BYTE0) >> 0); //0x00;

	//payload -> wdata1(LSB) -> wdata2 -> wdata3 -> wdata4 (MSB)
	write_buffer[3] = ((st.st_size & BYTE0) >> 0);
	write_buffer[4] = ((st.st_size & BYTE1) >> 8);
	write_buffer[5] = ((st.st_size & BYTE2) >> 16);
	write_buffer[6] = ((st.st_size & BYTE3) >> 24);

	for (int index = 3; index < 7; index++) {
		debug_print("# Retimer %d 0x%lx write_buffer: 0x%x\n", index,
			    (long int)st.st_size, write_buffer[index]);
	}

	ret = send_i2c_cmd(fd, FPGA_WRITE, slaveId, write_buffer, 0, 7, 0);
	if (ret) {
		fprintf(stderr,
			"FW update FPGA_WRITE failed write_buffer: 0x%x 0x%x 0x%x\n",
			write_buffer[0], write_buffer[1], write_buffer[2]);
		return ret;
	}

	// Verify Write
	fprintf(stdout, "Read Image Size.\n");
	memset(write_buffer, 0x00, sizeof(write_buffer));
	memset(read_buffer, 0x00, sizeof(read_buffer));
	write_buffer[0] = ((FPGA_IMG_SIZE_REG & BYTE2) >> 16); //0x04;
	write_buffer[1] = ((FPGA_IMG_SIZE_REG & BYTE1) >> 8); //0x00;
	write_buffer[2] = ((FPGA_IMG_SIZE_REG & BYTE0) >> 0); //0x00;

	ret = send_i2c_cmd(fd, FPGA_READ, slaveId, write_buffer, read_buffer, 3,
			   4);
	if (ret) {
		fprintf(stderr,
			"FW update FPGA_READ failed write_buffer: 0x%x 0x%x 0x%x\n",
			write_buffer[0], write_buffer[1], write_buffer[2]);
		return ret;
	}

	for (int index = 0; index < 4; index++) {
		fprintf(stdout, "Retimer %d read_buffer: 0x%x\n", index,
			read_buffer[index]);
	}

	//Copy CheckSum size to 0x04_0004
	fprintf(stdout, "Copy CheckSum ...\n");
	memset(write_buffer, 0x00, sizeof(write_buffer));
	memset(read_buffer, 0x00, sizeof(read_buffer));
	//Write to FPGA register @ 0x02_ABCD-> 0x02(BYTE2) -> 0xAB(BYTE1) -> 0xCD(BYTE0)
	write_buffer[0] = ((FPGA_CHKSUM_REG & BYTE2) >> 16); //0x04;
	write_buffer[1] = ((FPGA_CHKSUM_REG & BYTE1) >> 8); //0x00;
	write_buffer[2] = ((FPGA_CHKSUM_REG & BYTE0) >> 0); //0x04;

	//crc 32
	//payload -> wdata1(LSB) -> wdata2 -> wdata3 -> wdata4 (MSB)
	write_buffer[3] = ((fw_crc32 & BYTE0) >> 0);
	write_buffer[4] = ((fw_crc32 & BYTE1) >> 8);
	write_buffer[5] = ((fw_crc32 & BYTE2) >> 16);
	write_buffer[6] = ((fw_crc32 & BYTE3) >> 24);

	ret = send_i2c_cmd(fd, FPGA_WRITE, slaveId, write_buffer, 0, 7, 0);
	if (ret) {
		fprintf(stderr,
			"FW update FPGA_WRITE failed write_buffer: 0x%x 0x%x 0x%x\n",
			write_buffer[0], write_buffer[1], write_buffer[2]);
		return ret;
	}

	fprintf(stdout, "Read Checksum .\n");
	memset(write_buffer, 0x00, sizeof(write_buffer));
	memset(read_buffer, 0x00, sizeof(read_buffer));
	//Write to FPGA register @ 0x02_ABCD-> 0x02(BYTE2) -> 0xAB(BYTE1) -> 0xCD(BYTE0)
	write_buffer[0] = ((FPGA_CHKSUM_REG & BYTE2) >> 16); //0x04;
	write_buffer[1] = ((FPGA_CHKSUM_REG & BYTE1) >> 8); //0x00;
	write_buffer[2] = ((FPGA_CHKSUM_REG & BYTE0) >> 0); //0x04;

	ret = send_i2c_cmd(fd, FPGA_READ, slaveId, write_buffer, read_buffer,
			   W_BYTE_COUNT, R_BYTE_COUNT);
	if (ret) {
		fprintf(stderr,
			"FW update FPGA_READ failed write_buffer: 0x%x 0x%x 0x%x\n",
			write_buffer[0], write_buffer[1], write_buffer[2]);
		return ret;
	}

	for (int index = 0; index < 4; index++) {
		debug_print("Retimer %d read_buffer: 0x%x\n", index,
			    read_buffer[index]);
	}
	return 0;
}

/********************************************************************
 * copyImageFromFpga()
 *
 * Copy FW from DPRAM to FILE
 *
 * fw_fd to read DPRAM content
 * fd: file descrriptor
 * slaveId: FPGA I2C controller slave id
 *
 * RETURN: 0 if success
 ********************************************************************/

int copyImageFromFpga(unsigned int fw_fd, unsigned int fd, unsigned int slaveId)
{
	struct stat st;
	unsigned char *fw_buf = NULL;
	int ret = -1;
	unsigned char write_buffer[WRITE_BUF_SIZE] = { 0 };
	unsigned char read_buffer[BYTE_PER_PAGE] = { 0 };
	unsigned int pageCount = 0;

	if (fstat(fw_fd, &st)) {
		fprintf(stderr, "\nfstat error: [%s]\n", strerror(errno));
		close(fw_fd);
		return -1;
	}
	/* Check FW image size */
	/* fw_size must be mutiple of BYTE_PER_PAGE */
	if ((st.st_size <= 0) || (st.st_size % BYTE_PER_PAGE)) {
		fprintf(stderr, "\nNot a valid size: [%s]\n", strerror(errno));
		close(fw_fd);
		return -ERROR_WRONG_FIRMWARE;
	}

	fw_buf = (unsigned char *)malloc(st.st_size);

	if (fw_buf == NULL) {
		return -ERROR_MALLOC_FAILURE;
	}

	pageCount = ((unsigned int)st.st_size / BYTE_PER_PAGE);
	//Copy FW image to FPGA DP RAM 0x0_0000
	//Write to DPRAM address to write_buffer0, write_buffer1, write_buffer 2 and then 256 bytes of payload till pageCount
	for (uint32_t i = 0; i < pageCount; i++) {
		memset(write_buffer, 0x00, sizeof(write_buffer));
		memset(read_buffer, 0x00, sizeof(read_buffer));
		write_buffer[0] = (0x00 | (i & 0xFF00) >> 8);
		write_buffer[1] = (0x00 | (i & 0x00FF));
		write_buffer[2] = 0x00;
		memcpy(&write_buffer[3], fw_buf + (i * BYTE_PER_PAGE),
		       BYTE_PER_PAGE);
		ret = send_i2c_cmd(fd, FPGA_READ, slaveId, write_buffer,
				   read_buffer, W_BYTE_COUNT, BYTE_PER_PAGE);
		if (ret) {
			fprintf(stderr,
				"FW update FPGA_WRITE failed write_buffer: 0x%x 0x%x 0x%x\n",
				write_buffer[0], write_buffer[1],
				write_buffer[2]);
			free(fw_buf);
			return ret;
		}

		memcpy(fw_buf + (i * BYTE_PER_PAGE), &read_buffer[0],
		       BYTE_PER_PAGE);
	}
	fprintf(stdout, "Image copy from FPGA completed 0x%x \n",
		read_buffer[0]);
	ret = write(fw_fd, fw_buf, st.st_size);
	if (ret < 0) {
		fprintf(stderr, "ret:%d  unable to read FW file error %s \n",
			ret, strerror(errno));
		close(fw_fd);
		return -ERROR_OPEN_FIRMWARE;
	}

	close(fw_fd);

	free(fw_buf);
	return 0;
}
//payload ->
/*******************************************************************************
 * checkWriteNackError()
 *
 * status: Write NACK error bit 15-8 of "EEprom Update/Verify Control Status Register"
 * mask:
 * retimer: Failure seen for retimer number 0-7 & 8 for all
 *
 * RETURN: ERROR code formed as (ERROR_WRITE_NACK | (Retimer Number))
 *
 ******************************************************************************/

int checkWriteNackError(uint8_t status, const uint8_t mask[], uint8_t *retimer)
{
	int ret = -ERROR_WRITE_NACK;
	// read NACK error
	for (int i = 8; i >= 0; i--) {
		if ((i == 8) && ((status & mask[i]) == 0xFF)) {
			*retimer = *retimer | mask[i];
			fprintf(stderr,
				"Retimer WRITE NACK error...%d retimer 0x%x\n",
				i, *retimer);
			break;
		}
		if (((status & mask[i]) >> i) == 1) {
			*retimer = *retimer | mask[i];
			fprintf(stderr,
				"Retimer WRITE NACK error...%d retimer 0x%x\n",
				i, *retimer);
		}
	}

	ret = (ret | (*retimer & 0xFF));

	return ret;
}

/*******************************************************************************
 * checkReadNackError()
 *
 * status: Read NACK error bit 23-16 of "EEprom Update/Verify Control Status Register
 * mask: retimer number
 * retimer: Failure seen for retimer number 0-7 & 8 for all
 *
 * RETURN: ERROR code formed as (ERROR_READ_NACK | (Retimer Number))
 *
 ******************************************************************************/

int checkReadNackError(uint8_t status, const uint8_t mask[], uint8_t *retimer)
{
	int ret = -ERROR_READ_NACK;
	// read NACK error
	for (int i = 8; i >= 0; i--) {
		if ((i == 8) && ((status & mask[i]) == 0xFF)) {
			*retimer = *retimer | mask[i];
			fprintf(stderr,
				"Retimer READ NACK error...%d retimer 0x%x\n",
				i, *retimer);
			break;
		}
		if (((status & mask[i]) >> i) == 1) {
			*retimer = *retimer | mask[i];
			fprintf(stderr,
				"Retimer READ NACK error...%d retimer 0x%x\n",
				i, *retimer);
		}
	}

	ret = (ret | (*retimer & 0xFF));

	return ret;
}

/******************************************************************************
 * checkChecksumError()
 *
 * status: Retimer EEprom verify checksum status
 *         bit 31-24 of "EEprom Update/Verify Control Status Register"
 * mask: retimer number
 * retimer: Failure seen for retimer number 0-7 & 8 for all
 *
 * RETURN: ERROR code formed as (ERROR_CHECKSUM | (Retimer Number))
 *
 ******************************************************************************/
int checkChecksumError(uint8_t status, const uint8_t mask[], uint8_t *retimer)
{
	int ret = -ERROR_CHECKSUM;
	// read CheckSum error
	for (int i = 8; i >= 0; i--) {
		if ((i == 8) && ((status & mask[i]) == 0xFF)) {
			*retimer = *retimer | mask[i];
			fprintf(stderr,
				"Retimer CheckSum error...%d retimer 0x%x\n", i,
				*retimer);
			break;
		}
		if (((status & mask[i]) >> i) == 1) {
			*retimer = *retimer | mask[i];
			fprintf(stderr,
				"Retimer CheckSum error...%d retimer 0x%x\n", i,
				*retimer);
		}
	}

	ret = (ret | (*retimer & 0xFF));

	return ret;
}

/********************************************************************
 * startRetimerFwUpdate()
 * 
 * Trigger Retimer FW update for passed RetimerNumber
 *
 * fd: file descriptor
 * retimerNumber: update retimer 0-7 or all
 *
 * RETURN: 0 if success
 ********************************************************************/
int startRetimerFwUpdate(int fd, uint8_t retimerNumber)
{
	unsigned char write_buffer[WRITE_BUF_SIZE] = { 0 };
	unsigned char read_buffer[READ_BUF_SIZE] = { 0 };
	int ret = 0;

	// 8. Trigger update to 0x04_0008
	fprintf(stdout, "Trigger FW update...retimerNumber %d \n",
		retimerNumber);
	// Trigger FW update
	for (uint8_t updateRetryCount = 0;
	     updateRetryCount < MAX_UPDATE_RETRYCOUNT; updateRetryCount++) {
		fprintf(stdout, "Trigger FW update...\n");
		memset(write_buffer, 0x00, sizeof(write_buffer));
		memset(read_buffer, 0x00, sizeof(read_buffer));
		//Write to FPGA register @ 0x02_ABCD-> 0x02(BYTE2) -> 0xAB(BYTE1) -> 0xCD(BYTE0)
		write_buffer[0] =
			((FPGA_UPDATE_STATUS_REG & BYTE2) >> 16); //0x04;
		write_buffer[1] =
			((FPGA_UPDATE_STATUS_REG & BYTE1) >> 8); //0x00;
		write_buffer[2] =
			((FPGA_UPDATE_STATUS_REG & BYTE0) >> 0); //0x08;

		write_buffer[3] =
			retimerNumber; // Initiate FW update use Update4Retimer
		write_buffer[4] = 0x00;
		write_buffer[5] = 0x00;
		write_buffer[6] = 0x00;
		// Trigger update, writing 3 bytes address followed by 4 bytes 4 bytes 4 bytes 4 bytes value in FPGA Update control register to trigger update for retimerNumber and reading back
		// value from 0x04_0008 AKA FPGA_Control and udpate status register
		ret = send_i2c_cmd(fd, FPGA_WRITE, FPGA_I2C_CNTRL_ADDR,
				   write_buffer, read_buffer,
				   W_BYTE_COUNT_WITHPAYLOAD, R_BYTE_COUNT);
		if (ret) {
			fprintf(stderr,
				"Retimer Fw Update failed!!,send_i2c_cmd command failed with  %d errno %s ...\n",
				ret, strerror(errno));
			return ret;
		}

		// 9. Monitor update progress by reading to 0x04_0008
		fprintf(stdout, "Monitor FW update...updateRetryCount %d \n",
			updateRetryCount);

		uint8_t timeout = 0;
		do {
			if (timeout >= MAX_TIMEOUT_SEC) {
				fprintf(stderr,
					"Retimer FW update : Timeout!!, update still not completed for retimer %d...\n",
					retimerNumber);
				break;
			}
			usleep(DELAY_1SEC); // sleep for 1 second

			memset(write_buffer, 0x00, sizeof(write_buffer));
			memset(read_buffer, 0x00, sizeof(read_buffer));
			//Write to FPGA register @ 0x02_ABCD-> 0x02(BYTE2) -> 0xAB(BYTE1) -> 0xCD(BYTE0)
			write_buffer[0] = ((FPGA_UPDATE_STATUS_REG & BYTE2) >>
					   16); //0x04;
			write_buffer[1] =
				((FPGA_UPDATE_STATUS_REG & BYTE1) >> 8); //0x00;
			write_buffer[2] =
				((FPGA_UPDATE_STATUS_REG & BYTE0) >> 0); //0x08;

			ret = send_i2c_cmd(fd, FPGA_READ, FPGA_I2C_CNTRL_ADDR,
					   write_buffer, read_buffer,
					   W_BYTE_COUNT, R_BYTE_COUNT);
			if (ret) {
				fprintf(stderr,
					"Retimer FW update failed!!,send_i2c_cmd command failed with  %d errno %s ...\n",
					ret, strerror(errno));
				return ret;
			}

			fprintf(stdout,
				"FW update out: 0x%x 0x%x 0x%x 0x%x 0x%x\n",
				read_buffer[0], read_buffer[1], read_buffer[2],
				read_buffer[3], retimerNumber);
			timeout++;

		} while (read_buffer[0] != FW_UPDATE_COMPLETE_FLAG);

		// read status register and check for any error in read, write and verify stage
		uint8_t status_verfication = read_buffer[0];
		uint8_t status_writeNack = read_buffer[1];
		uint8_t status_readNack = read_buffer[2];
		uint8_t status_checksum = read_buffer[3];
		uint8_t retryUpdate4Retimer = 0;

		if (status_verfication) {
			fprintf(stdout,
				"FW update...completed, checking status !!! \n");
			if (status_writeNack) {
				ret = checkWriteNackError(status_writeNack,
							  mask_retimer,
							  &retryUpdate4Retimer);
				prepareMessageRegistry(
					retryUpdate4Retimer,
					"VerificationFailed",
					MSG_REG_DEV_FOLLOWED_BY_VER,
					"xyz.openbmc_project.Logging.Entry.Level.Critical",
					NULL);
			}
			if (status_readNack) {
				ret |= checkReadNackError(status_readNack,
							  mask_retimer,
							  &retryUpdate4Retimer);
				prepareMessageRegistry(
					retryUpdate4Retimer,
					"VerificationFailed",
					MSG_REG_DEV_FOLLOWED_BY_VER,
					"xyz.openbmc_project.Logging.Entry.Level.Critical",
					NULL);
			}
			if (status_checksum) {
				ret |= checkChecksumError(status_checksum,
							  mask_retimer,
							  &retryUpdate4Retimer);
				prepareMessageRegistry(
					retryUpdate4Retimer,
					"VerificationFailed",
					MSG_REG_DEV_FOLLOWED_BY_VER,
					"xyz.openbmc_project.Logging.Entry.Level.Critical",
					NULL);
			}
			retimerNumber = retryUpdate4Retimer;
			fprintf(stderr,
				"FW update...not completed, its timeout !!! \n");
		} else {
			fprintf(stdout,
				"FW update...completed, No Retry !!! \n");
			break;
		}
		// End of retry loop for FW update
	}
	return ret;
}

/********************************************************************
 * readRetimerfw()
 *
 * Trigger Retimer Read for passed RetimerNumber
 *
 * fd: file descriptor
 * retimerNumber: read one of the retimer out of 0 to 7
 *
 * RETURN: 0 if success
 ********************************************************************/
int readRetimerfw(int fd, uint8_t retimerNumber)
{
	unsigned char write_buffer[WRITE_BUF_SIZE] = { 0 };
	unsigned char read_buffer[READ_BUF_SIZE] = { 0 };
	int ret = 0;

	// Trigger Retimer Read
	for (uint8_t update4retimerCount = 0;
	     update4retimerCount < MAX_UPDATE_RETRYCOUNT;
	     update4retimerCount++) {
		fprintf(stdout,
			"Retimer FW Read : Initiate retimer read ...\n");
		memset(write_buffer, 0x00, sizeof(write_buffer));
		memset(read_buffer, 0x00, sizeof(read_buffer));
		//Write to FPGA register @ 0x02_ABCD-> 0x02(BYTE2) -> 0xAB(BYTE1) -> 0xCD(BYTE0)
		write_buffer[0] =
			((FPGA_READ_STATUS_REG & BYTE2) >> 16); //0x04;
		write_buffer[1] = ((FPGA_READ_STATUS_REG & BYTE1) >> 8); //0x00;
		write_buffer[2] = ((FPGA_READ_STATUS_REG & BYTE0) >> 0); //0x0C;

		//write number of byte to be read from retimer
		write_buffer[3] =
			((((retimerNumber)&NIBBLE) << 4) |
			 (SET_RETIMER_FW_READ)); // trigger read for specified retimer
		write_buffer[4] = 0x0;
		write_buffer[5] = 0x0;
		write_buffer[6] = 0x0;

		// trigger retimer read
		ret = send_i2c_cmd(fd, FPGA_WRITE, FPGA_I2C_CNTRL_ADDR,
				   write_buffer, read_buffer,
				   W_BYTE_COUNT_WITHPAYLOAD, R_BYTE_COUNT);
		if (ret) {
			fprintf(stderr,
				"Retimer FW Read : failed!, send_i2c_cmd not completed for retimer %d...errno %s\n",
				retimerNumber, strerror(errno));
			return ret;
		}

		fprintf(stdout, "out: 0x%x 0x%x 0x%x 0x%x 0x%x \n",
			read_buffer[0], read_buffer[1], read_buffer[2],
			read_buffer[3], retimerNumber);

		// 9. Monitor FW read progress by reading to 0x04_000C
		fprintf(stdout,
			"Retimer FW Read : Monitor Read progress update...\n");

		uint8_t timeout = 0;
		do {
			if (timeout >= MAX_TIMEOUT_SEC) {
				fprintf(stderr,
					"Retimer FW Read : Timeout!!, read still not completed for retimer %d...\n",
					retimerNumber);
				break;
			}
			usleep(DELAY_1SEC); // sleep for 1 second
			fprintf(stdout,
				"Retimer FW Read : Monitor Read progress update...\n");
			memset(write_buffer, 0x00, sizeof(write_buffer));
			memset(read_buffer, 0x00, sizeof(read_buffer));
			//Write to FPGA register @ 0x02_ABCD-> 0x02(BYTE2) -> 0xAB(BYTE1) -> 0xCD(BYTE0)
			write_buffer[0] =
				((FPGA_READ_STATUS_REG & BYTE2) >> 16); //0x04;
			write_buffer[1] =
				((FPGA_READ_STATUS_REG & BYTE1) >> 8); //0x00;
			write_buffer[2] =
				((FPGA_READ_STATUS_REG & BYTE0) >> 0); //0x0C;

			ret = send_i2c_cmd(fd, FPGA_READ, FPGA_I2C_CNTRL_ADDR,
					   write_buffer, read_buffer,
					   W_BYTE_COUNT, R_BYTE_COUNT);
			if (ret) {
				fprintf(stderr,
					"Retimer FW Read : failed!, send_i2c_cmd not completed for retimer %d...errno %s\n",
					retimerNumber, strerror(errno));
				return ret;
			}
			debug_print(
				"Retimer FW Read : out: 0x%x 0x%x 0x%x 0x%x %d\n",
				read_buffer[0], read_buffer[1], read_buffer[2],
				read_buffer[3], retimerNumber);
			timeout++;
		} while ((read_buffer[0] & FW_READ_STATUS_MASK) != 0x0);

		uint8_t status_verification =
			read_buffer[0] & FW_READ_STATUS_MASK;
		uint8_t status_nackverification =
			read_buffer[1] & FW_READ_NACK_MASK;

		// read NACK error
		if (status_verification == 0x0) {
			if (status_nackverification) {
				// set retimerNumber bit to 1 for retimer.
				fprintf(stderr,
					"Retimer FW Read : failed for Retimer %d : \n",
					retimerNumber);
				continue;
			} else {
				fprintf(stdout,
					"Retimer FW Read : Retimer Read completed for Retimer %d \n",
					retimerNumber);
				break;
			}
		} else {
			fprintf(stdout,
				"Retimer FW Read : Timeout !!! read still not completed for retimer %d...\n",
				retimerNumber);
		}
	}
	return ret;
}