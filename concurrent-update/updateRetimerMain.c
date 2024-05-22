/*
 * SPDX-FileCopyrightText: Copyright (c) 2022-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h> // for lseek()
#include <fcntl.h>
#include "updateRetimerFwOverI2C.h"

extern uint8_t verbosity;
extern const uint8_t mask_retimer[];
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
	char *versionStr = NULL;
	uint32_t imageFilenameSize = 0;
	int imagefd = -1;
	int dummyfd = -1;
	struct stat st = { 0 };
	size_t fw_size = 0;
	const unsigned char *imageMappedAddr = NULL;
	update_operation *update_ops = NULL;
	int update_ops_count = -1;

	// set stdout to line-buffered so it interleaves correctly with stderr
	setvbuf(stdout, NULL, _IOLBF, 0);

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

		imagefd = open(imageFilename, O_RDONLY);

		if (imagefd < 0) {
			fprintf(stderr, "Error opening file: %s\n",
				strerror(errno));
			prepareMessageRegistry(
				retimerToUpdate, "VerificationFailed", versionStr,
				MSG_REG_VER_FOLLOWED_BY_DEV,
				"xyz.openbmc_project.Logging.Entry.Level.Critical",
				NULL, 0);
			goto exit;
		}

		if (fstat(imagefd, &st)) {
			fprintf(stderr, "\nfstat error: [%s]\n", strerror(errno));
			prepareMessageRegistry(
				retimerToUpdate, "TransferFailed", versionStr,
				MSG_REG_VER_FOLLOWED_BY_DEV,
				"xyz.openbmc_project.Logging.Entry.Level.Critical",
				NULL, 0);
			goto exit;
		}
		fw_size = st.st_size;
		imageMappedAddr = mmap(NULL, fw_size, PROT_READ, MAP_PRIVATE, imagefd, 0);
		if (imageMappedAddr == MAP_FAILED) {
			perror("Memory-mapping of FW image for processing failed");
			prepareMessageRegistry(
				retimerToUpdate, "TransferFailed", versionStr,
				MSG_REG_VER_FOLLOWED_BY_DEV,
				"xyz.openbmc_project.Logging.Entry.Level.Critical",
				NULL, 0);
			goto exit;
		}

		close(imagefd);
		imagefd = -1;

		ret = parseCompositeImage(imageMappedAddr, fw_size, versionStr,
			&update_ops, &update_ops_count);
		if (ret) {
			fprintf(stderr, "parseCompositeImage returned: [%d]\n", ret);
			prepareMessageRegistry(
				retimerToUpdate, "VerificationFailed", versionStr,
				MSG_REG_VER_FOLLOWED_BY_DEV,
				"xyz.openbmc_project.Logging.Entry.Level.Critical",
				NULL, 0);
			goto exit;
		}
		// if update_ops_count <= 0, we will not enter update loop below
		// and retimer update will exit successfully. Only need to check
		// if update_ops_count > 0 and we did not get the array.
		if (update_ops_count > 0 && !update_ops) {
			ret = -ERROR_UNKNOWN;
			fprintf(stderr, "update_ops_count is %d but update_ops is NULL\n",
				update_ops_count);
			prepareMessageRegistry(
				retimerToUpdate, "VerificationFailed", versionStr,
				MSG_REG_VER_FOLLOWED_BY_DEV,
				"xyz.openbmc_project.Logging.Entry.Level.Critical",
				NULL, 0);
			goto exit;
		}

		// Composite and bare images reconverge here with update_operations filled in
		// Perform target filtering intersection and log TargetDetermined logs
		for (int uo = 0; uo < update_ops_count; uo++) {
			fprintf(stdout, "update operation %d, startOffset %#zx, " \
				"imageLength %zu, applyBitmap %#x, actual bitmap %#x, " \
				"imageCrc %#x, versionString %s\n",
				uo, update_ops[uo].startOffset, update_ops[uo].imageLength,
				update_ops[uo].applyBitmap,
				update_ops[uo].applyBitmap & retimerToUpdate,
				update_ops[uo].imageCrc,
				update_ops[uo].versionString);
			update_ops[uo].applyBitmap &= retimerToUpdate;
			prepareMessageRegistry(
				update_ops[uo].applyBitmap, "TargetDetermined",
				update_ops[uo].versionString, MSG_REG_DEV_FOLLOWED_BY_VER,
				"xyz.openbmc_project.Logging.Entry.Level.Informational",
				NULL, 0);
		}

		for (int uo = 0; uo < update_ops_count; uo++) {
			fprintf(stderr, "performing update_ops[%d]\n", uo);
			if (!update_ops[uo].applyBitmap) {
				fprintf(stdout, "applyBitmap for update_ops[%d] is 0, skipping\n", uo);
				continue;
			}
			prepareMessageRegistry(
				update_ops[uo].applyBitmap, "TransferringToComponent",
				update_ops[uo].versionString, MSG_REG_VER_FOLLOWED_BY_DEV,
				"xyz.openbmc_project.Logging.Entry.Level.Informational",
				NULL, 0);

			ret = copyImageFromMemToFpga(imageMappedAddr + update_ops[uo].startOffset,
				update_ops[uo].imageLength, update_ops[uo].imageCrc, fd,
				FPGA_I2C_CNTRL_ADDR);
			if (ret) {
				fprintf(stderr,
					"FW Update FW image copy to FPGA failed  error code%d!!!\n",
					ret);
				prepareMessageRegistry(
					update_ops[uo].applyBitmap, "TransferFailed",
					update_ops[uo].versionString, MSG_REG_VER_FOLLOWED_BY_DEV,
					"xyz.openbmc_project.Logging.Entry.Level.Critical",
					NULL, 0);
				goto exit;
			}

			// Trigger FW Update to one or more retimer at a time and monitor the update progress and its completion
			ret = startRetimerFwUpdate(fd, update_ops[uo].applyBitmap,
						update_ops[uo].versionString, &retimerNotUpdated);
			if (ret) {
				fprintf(stderr,
					"FW Update for Retimer %d failed for retimer with error code " \
					"%d retimerNotUpdated %d!!!\n",
					update_ops[uo].applyBitmap, ret, retimerNotUpdated);
				prepareMessageRegistry(
					retimerNotUpdated, "ApplyFailed", update_ops[uo].versionString,
					MSG_REG_VER_FOLLOWED_BY_DEV,
					"xyz.openbmc_project.Logging.Entry.Level.Critical",
					NULL, 0);

				if (update_ops[uo].applyBitmap ^ retimerNotUpdated) {
					prepareMessageRegistry(
						(update_ops[uo].applyBitmap ^ retimerNotUpdated),
						"UpdateSuccessful", update_ops[uo].versionString,
						MSG_REG_DEV_FOLLOWED_BY_VER,
						"xyz.openbmc_project.Logging.Entry.Level.Informational",
						NULL, 0);

					prepareMessageRegistry(
						(update_ops[uo].applyBitmap ^ retimerNotUpdated),
						"AwaitToActivate", update_ops[uo].versionString,
						MSG_REG_VER_FOLLOWED_BY_DEV,
						"xyz.openbmc_project.Logging.Entry.Level.Informational",
						"AC power cycle", 0);
				}
				goto exit;
			}
			prepareMessageRegistry(
				update_ops[uo].applyBitmap, "UpdateSuccessful",
				update_ops[uo].versionString, MSG_REG_DEV_FOLLOWED_BY_VER,
				"xyz.openbmc_project.Logging.Entry.Level.Informational",
				NULL, 0);

			prepareMessageRegistry(
				update_ops[uo].applyBitmap, "AwaitToActivate",
				update_ops[uo].versionString, MSG_REG_VER_FOLLOWED_BY_DEV,
				"xyz.openbmc_project.Logging.Entry.Level.Informational",
				"AC power cycle", 0);
		}
		break;

	case RETIMER_FW_READ: // 10.0 Read Retimer image

		fprintf(stdout, "#10 Trigger Retimer Read ...%d\n",
			retimerToRead);
		dummyfd = open("/tmp/Dummyfile", O_RDWR | O_CREAT, 0644);

		if (dummyfd < 0) {
			fprintf(stderr, "Error creating file: %s\n",
				strerror(errno));
			goto exit;
		}

		if (ftruncate(dummyfd, MAX_FW_IMAGE_SIZE) < 0) {
			fprintf(stderr,
				"FW READ for Retimer failed for retimer !!!");
			close(dummyfd);
			dummyfd = -1;
			goto exit;
		}

		// Create and Pass dummy blank FILE of 256KB to clear DPRAM before reading content from Retimer
		ret = copyImageFromFileToFpga(dummyfd, fd, FPGA_I2C_CNTRL_ADDR);
		if (ret) {
			fprintf(stderr,
				"FW read FW image copy to FPGA failed  error code%d!!!",
				ret);
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
		lseek(dummyfd, 0, 0);
		ret = copyImageFromFpga(dummyfd, fd, FPGA_I2C_CNTRL_ADDR);
		if (ret) {
			fprintf(stderr,
				"FW read FW image copy from FPGA failed  error code%d!!!",
				ret);
			goto exit;
		}
		close(dummyfd);

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
	if (imageMappedAddr != NULL && imageMappedAddr != MAP_FAILED && st.st_size >= 0) {
		munmap((void *)imageMappedAddr, st.st_size);
	}
	if (update_ops) {
		free(update_ops);
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
