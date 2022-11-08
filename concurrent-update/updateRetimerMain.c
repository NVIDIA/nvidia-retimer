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
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include "updateRetimerFwOverI2C.h"

extern uint8_t verbosity;
extern const uint8_t mask_retimer[];
extern char *versionStr;
extern char *arrRetimer[];
extern uint8_t retimerNum;
/************************************************
 * show_usage()
 *
 * instruction of command
 *
 * exec: program name
 *
 *
 * No RETURN.
 ***********************************************/
void show_usage(char *exec)
{
	printf("\nUsage: %s <i2c bus number> <retimer number> <firmware filename> <update/read> <versionStr> <verbosity>\n",
	       exec);
	printf("        i2c bus number	: must be digits [3-12]\n");
	printf("        retimer number		: must be digits [0-7]\n");
	printf("        update/read/write	: 0=Update, 1=Read \n");
	printf("        versionStr(optional): versionStr for message registry \n");
	printf("        verbosity(debug)	: 1=enabled, 0=disable \n");
	printf("        EX: %s 12 8 <FW_image>.bin 0 <1>\n\n", exec);
}

/******************************************************************************
* Usage:  updateRetimerFw  <i2c bus number>  <retimer number> <firmware filename> <update/read> <VersionStr> <verbosity>
* i2c bus number          : must be digits [3-12]
* retimer number          : must be digits [0-7], 8 for all retimers update
* update/read/write       : 0=Update, 1=Read
* versionStr(optional)    : versionStr for message registry
* verbosity(debug)        : 1=enabled, 0=disable 
*******************************************************************************/

int main(int argc, char *argv[])
{
	char i2c_device[MAX_NAME_SIZE] = { 0 };
	int fd = -1;
	int ret = 0;
	char imageFilename[MAX_NAME_SIZE];
	uint8_t retimerToUpdate = INIT_UINT8;
	uint8_t retimerNotUpdated = INIT_UINT8;
	uint8_t retimerToRead = INIT_UINT8;
	uint8_t command = INIT_UINT8;
	uint32_t imageFilenameSize = 0;
	int imagefd = -1;
	int dummyfd = -1;

	// Check input argument number
	if (argc < 5) {
		ret = -ERROR_INPUT_ARGUMENTS;
		goto exit;
	}

	// Check i2c_bus input
	if (checkDigit_i2c(argv[1])) {
		ret = -ERROR_INPUT_I2C_ARGUMENT;
		goto exit;
	}

	// Check input argument for valid retimer number
	if (checkDigit_retimer(argv[2])) {
		ret = -ERROR_INPUT_I2C_ARGUMENT;
		goto exit;
	}

	retimerNum = atoi(argv[2]);
	retimerToUpdate = atoi(argv[2]);
	retimerToRead = atoi(argv[2]);

	/* Check if passed filename is too small for imageFilename buffer*/
	imageFilenameSize = strlen(argv[3]);

	if (imageFilenameSize >= MAX_NAME_SIZE) {
		ret = -ERROR_INPUT_ARGUMENTS;
		goto exit;
	}

	strncpy(imageFilename, argv[3], imageFilenameSize + 1);

	command = atoi(argv[4]);

	// FW Version
	if (argc > 5) {
		versionStr = argv[5];
		fprintf(stdout, "[DEBUG]%s, %d %s\n", __func__, __LINE__,
			argv[5]);
	} else {
		versionStr = DEFAULT_VERSION;
	}

	// Enable verbose mode
	if (argc == 7) {
		verbosity = atoi(argv[6]);
		fprintf(stdout, "[DEBUG]%s, %d %d\n", __func__, __LINE__,
			verbosity);
	}

	sprintf(i2c_device, "/dev/i2c-%d", atoi(argv[1]));

	fd = open(i2c_device, O_RDWR | O_NONBLOCK);

	if (fd < 0) {
		fprintf(stderr, "Error opening i2c file: %s\n",
			strerror(errno));
		ret = -ERROR_OPEN_I2C_DEVICE;
		goto exit;
	}

	switch (command) {
	case RETIMER_FW_UPDATE: // Update

		// Load FW image, calcualte checkSum and update FW image size
		fprintf(stdout, "Start FW update procedure...\n");
		fprintf(stdout, "Read FW Image...%s Verion %s \n",
			imageFilename, versionStr);
		fprintf(stdout, "Retimer under update ...%d \n", retimerNum);

		prepareMessageRegistry(
			retimerToUpdate, "TargetDetermined",
			MSG_REG_DEV_FOLLOWED_BY_VER,
			"xyz.openbmc_project.Logging.Entry.Level.Informational",
			NULL);

		imagefd = open(imageFilename, O_RDONLY);

		if (imagefd < 0) {
			fprintf(stderr, "Error opening file: %s\n",
				strerror(errno));
			close(imagefd);
			imagefd = -1;
			prepareMessageRegistry(
				retimerToUpdate, "VerificationFailed",
				MSG_REG_DEV_FOLLOWED_BY_VER,
				"xyz.openbmc_project.Logging.Entry.Level.Critical",
				NULL);
		}
		prepareMessageRegistry(
			retimerToUpdate, "TransferringToComponent",
			MSG_REG_VER_FOLLOWED_BY_DEV,
			"xyz.openbmc_project.Logging.Entry.Level.Informational",
			NULL);

		ret = copyImageToFpga(imagefd, fd, FPGA_I2C_CNTRL_ADDR);
		if (ret) {
			fprintf(stderr,
				"FW Update FW image copy to FPGA failed  error code%d!!!",
				ret);
			prepareMessageRegistry(
				retimerToUpdate, "TransferFailed",
				MSG_REG_VER_FOLLOWED_BY_DEV,
				"xyz.openbmc_project.Logging.Entry.Level.Critical",
				NULL);
			goto exit;
		}

		// Trigger FW Update to one or more retimer at a time and monitor the update progress and its completion
		ret = startRetimerFwUpdate(fd, retimerToUpdate,
					   &retimerNotUpdated);
		if (ret) {
			fprintf(stderr,
				"FW Update for Retimer failed for retimer with error code%d retimerNotUpdated %d!!!",
				ret, retimerNotUpdated);
			prepareMessageRegistry(
				retimerNotUpdated, "ApplyFailed",
				MSG_REG_DEV_FOLLOWED_BY_VER,
				"xyz.openbmc_project.Logging.Entry.Level.Critical",
				NULL);

			if (retimerToUpdate ^ retimerNotUpdated) {
				prepareMessageRegistry(
					(retimerToUpdate ^ retimerNotUpdated),
					"UpdateSuccessful",
					MSG_REG_DEV_FOLLOWED_BY_VER,
					"xyz.openbmc_project.Logging.Entry.Level.Informational",
					NULL);

				prepareMessageRegistry(
					(retimerToUpdate ^ retimerNotUpdated),
					"AwaitToActivate",
					MSG_REG_DEV_FOLLOWED_BY_VER,
					"xyz.openbmc_project.Logging.Entry.Level.Informational",
					"AC power cycle");
			}
			goto exit;
		}
		prepareMessageRegistry(
			retimerToUpdate, "UpdateSuccessful",
			MSG_REG_DEV_FOLLOWED_BY_VER,
			"xyz.openbmc_project.Logging.Entry.Level.Informational",
			NULL);

		prepareMessageRegistry(
			retimerToUpdate, "AwaitToActivate",
			MSG_REG_DEV_FOLLOWED_BY_VER,
			"xyz.openbmc_project.Logging.Entry.Level.Informational",
			"AC power cycle");
		break;

	case RETIMER_FW_READ: // 10.0 Read Retimer image

		fprintf(stdout, "#10 Trigger Retimer Read ...%d\n",
			retimerToRead);
		dummyfd = open("Dummyfile", O_RDWR | O_APPEND | O_CREAT, 0644);

		if (dummyfd < 0) {
			fprintf(stderr, "Error creating file: %s\n",
				strerror(errno));
		}

		if (ftruncate(dummyfd, MAX_FW_IMAGE_SIZE) < 0) {
			fprintf(stderr,
				"FW READ for Retimer failed for retimer !!!");
			close(dummyfd);
			dummyfd = -1;
			goto exit;
		}

		// Create and Pass dummy blank FILE of 256KB to clear DPRAM before reading content from Retimer
		ret = copyImageToFpga(dummyfd, fd, FPGA_I2C_CNTRL_ADDR);
		if (ret) {
			fprintf(stderr,
				"FW read FW image copy to FPGA failed  error code%d!!!",
				ret);
			close(dummyfd);
			goto exit;
		}

		// Initiate FW READ to one of the retimer at a time and monitor the read progress and status
		ret = readRetimerfw(fd, retimerToRead);
		if (ret) {
			fprintf(stderr,
				"FW READ for Retimer failed for retimer %d!!!",
				retimerToRead);
			goto exit;
		}

		ret = copyImageFromFpga(dummyfd, fd, FPGA_I2C_CNTRL_ADDR);
		if (ret) {
			fprintf(stderr,
				"FW read FW image copy from FPGA failed  error code%d!!!",
				ret);
			close(dummyfd);
			goto exit;
		}
		close(dummyfd);
		dummyfd = -1;
		break;

	default:
		fprintf(stderr,
			"Incorrect option passed to FWUpdate utility %d!!!",
			command);
		ret = -ERROR_INPUT_ARGUMENTS;
		break;

	} // end of switch case

exit:
	if (fd != -1) {
		close(fd);
	}
	if (imagefd != -1) {
		close(imagefd);
	}
	if (dummyfd != -1) {
		close(dummyfd);
	}

	if ((ret == -ERROR_INPUT_ARGUMENTS) ||
	    (ret == -ERROR_INPUT_I2C_ARGUMENT)) {
		show_usage(argv[0]);
	}
	if (ret) {
		printf("!!!!! Retimer UPDATE F A I L (%d) !!!!!!\n", ret);
	} else
		printf("!!!!! Retimer UPDATE SUCCESSFUL (%d) !!!!!!\n", ret);

	return ret;
}
