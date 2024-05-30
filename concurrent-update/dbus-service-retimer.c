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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-bus.h>
#include <errno.h>
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <assert.h>
#include <sys/stat.h>

#include "updateRetimerFwOverI2C.h"

// SHA384 Hash compute
#define BLOCK_SIZE (64 * 1024) // 64 KB block size
#define HASH_LENGTH 48

#define RETIMER_PATH "/com/Nvidia/ComputeHash/HGX_FW_PCIeRetimer_"
#define MAX_RETIMERS 8
#define DBUS_ERR "org.openbmc.error"
#define INIT_INT -1

sd_bus *busHandle = NULL;

typedef struct hash_compute {
	char hashAlgo[64];
	char hashDigest[97];
} hash_t;

const char hashingAlgorithm[64] = "SHA384";

hash_t g_retimerHash[MAX_RETIMERS];

int readFWImagenComputeHash(unsigned retimerId)
{
	int dummyfd = INIT_INT;
	char i2c_device[MAX_NAME_SIZE] = { 0 };
	int fd = INIT_INT;
	int ret = INIT_INT;
	int bus = FPGA_I2C_BUS;
	char hashValue[HASH_LENGTH] = { 0 };

	sprintf(i2c_device, "/dev/i2c-%u", bus);

	fd = open(i2c_device, O_RDWR | O_NONBLOCK);

	if (fd < 0) {
		fprintf(stderr, "Error opening i2c file: %s\n",
			strerror(errno));
		ret = -ERROR_OPEN_I2C_DEVICE;
		goto exit;
	}

	// Check if directory does not exists
	if (access("/tmp/hash", F_OK) == -1) {
		// Create the directory
		if (mkdir("/tmp/hash", 0700) == -1) {
			fprintf(stderr, "Error creating file: %s\n",
				strerror(errno));
			ret = EXIT_FAILURE;
			goto exit;
		}
	}

	dummyfd = open("/tmp/hash/RetimerFW.dat", O_RDWR | O_CREAT, 0644);

	if (dummyfd < 0) {
		fprintf(stderr, "Error creating file: %s\n", strerror(errno));
		ret = EXIT_FAILURE;
		goto exit;
	}

	if (ftruncate(dummyfd, MAX_FW_IMAGE_SIZE) < 0) {
		fprintf(stderr, "ftruncate failed with  %s\n!!!",
			strerror(errno));
		ret = EXIT_FAILURE;
		goto exit;
	}

	// Create and Pass dummy blank FILE of 256KB to clear DPRAM before reading content from Retimer
	ret = copyImageFromFileToFpga(dummyfd, fd, FPGA_I2C_CNTRL_ADDR);
	if (ret) {
		fprintf(stderr,
			"FW read FW image copy to FPGA failed  error code%d!!!",
			ret);
		ret = EXIT_FAILURE;
		goto exit;
	}
	// Initiate FW READ to one of the retimer at a time and monitor the read progress and status
	ret = readRetimerfw(fd, retimerId);
	if (ret) {
		fprintf(stderr, "FW READ for Retimer failed for retimer %u!!!",
			retimerId);
		ret = EXIT_FAILURE;
		goto exit;
	}

	// Reset the file offset to the beginning of the file
	if (lseek(dummyfd, 0, SEEK_SET) == -1) {
		fprintf(stderr, "lseek failed %s\n!!!", strerror(errno));
		ret = EXIT_FAILURE;
		goto exit;
	}

	ret = copyImageFromFpga(dummyfd, fd, FPGA_I2C_CNTRL_ADDR);
	if (ret) {
		fprintf(stderr,
			"FW read FW image copy from FPGA failed  error code%d!!!",
			ret);
		ret = EXIT_FAILURE;
		goto exit;
	}

	unsigned char hash[HASH_LENGTH];
	unsigned char block[BLOCK_SIZE];

	// Reset the file offset to the beginning of the file
	if (lseek(dummyfd, 0, SEEK_SET) == -1) {
		fprintf(stderr, "lseek failed %s\n!!!", strerror(errno));
		ret = EXIT_FAILURE;
		goto exit;
	}
	// Create and initialize the SHA384 context
	EVP_MD_CTX *ctx = EVP_MD_CTX_new();
	if (ctx == NULL) {
		fprintf(stderr, "Failed to create SHA384 context\n");
		ret = EXIT_FAILURE;
		goto exit;
	}
	if (!EVP_DigestInit_ex(ctx, EVP_sha384(), NULL)) {
		fprintf(stderr, "Failed to initialize SHA384 context\n");
		EVP_MD_CTX_free(ctx);
		ret = EXIT_FAILURE;
		goto exit;
	}

	// Read the file in blocks and update the SHA384 context
	int bytes_read = 0;
	while ((bytes_read = read(dummyfd, block, sizeof(block))) > 0) {
		//EVP_DigestUpdate(ctx, buffer, bytes_read);
		if (!EVP_DigestUpdate(ctx, block, bytes_read)) {
			fprintf(stderr, "Failed to update SHA384 context\n");
			EVP_MD_CTX_free(ctx);
			ret = EXIT_FAILURE;
			goto exit;
		}
	}

	if (bytes_read == -1) {
		// Handle error
		fprintf(stderr,
			"Read failed over RetimerFW.dat file error code %s \n",
			strerror(errno));
		ret = EXIT_FAILURE;
		goto exit;
	}

	// Finalize the SHA384 hash
	unsigned int hash_len = HASH_LENGTH;
	if (!EVP_DigestFinal_ex(ctx, hash, &hash_len)) {
		fprintf(stderr, "Failed to finalize SHA384 hash\n");
		EVP_MD_CTX_free(ctx);
		ret = EXIT_FAILURE;
		goto exit;
	}

	// Print the SHA384 hash
	for (int i = 0; i < HASH_LENGTH; i++) {
		sprintf(hashValue + (i * 2), "%02x", hash[i]);
	}
	strncpy(g_retimerHash[retimerId].hashDigest, hashValue,
		sizeof(g_retimerHash[retimerId].hashDigest));

	// Free the SHA384 context
	EVP_MD_CTX_free(ctx);

exit:
	// Close the file
	if (dummyfd != -1) {
		// Deleting RetimerFW.dat
		if (unlink("/tmp/hash/RetimerFW.dat") == -1) {
			fprintf(stderr, "unlink  failed error code!!!");
			ret = EXIT_FAILURE;
		}
		close(dummyfd);
	}
	close(fd);
	return ret;
}

/* D-Bus method implementation */
static int method_computeHash(sd_bus_message *m,
			      __attribute__((unused)) void *userdata,
			      __attribute__((unused)) sd_bus_error *ret_error)
{
	int ret = INIT_INT;
	unsigned retimerId = 0xFF;
	char retimerPath[256] = { 0 };

	ret = sd_bus_message_read(m, "u", &retimerId);
	if (ret < 0) {
		fprintf(stderr, "Failed to extract retimerId: %s",
			strerror(errno));
		return EXIT_FAILURE;
	}

	if (retimerId >= MAX_RETIMERS) {
		fprintf(stderr, "Invalid retimer Id");
		sd_bus_error_set_const(
			ret_error, DBUS_ERR,
			"xyz.openbmc_project.Common.Error.InvalidArgument");
		return sd_bus_reply_method_return(m, "x", 0);
	}
	// send the response to the client,
	// client will rely on Digest property for the result
	ret = sd_bus_reply_method_return(m, NULL);
	if (ret < 0) {
		fprintf(stderr, "Invalid response:%s \n", strerror(errno));
		return EXIT_FAILURE;
	}
	// reset the hash value
	strncpy(g_retimerHash[retimerId].hashDigest, "",
		sizeof(g_retimerHash[retimerId].hashDigest));
	ret = readFWImagenComputeHash(retimerId);
	if (ret) {
		fprintf(stderr,
			"Error while calculating retimer hash for retimer: %u",
			retimerId);
		// hash computation has failed, send signal with empty property value
		snprintf(retimerPath, sizeof(retimerPath), "%s%u", RETIMER_PATH,
			 retimerId);
		sd_bus_emit_properties_changed(busHandle, retimerPath,
					       "com.Nvidia.ComputeHash",
					       "Digest", NULL);
		return ret;
	}
	snprintf(retimerPath, sizeof(retimerPath), "%s%u", RETIMER_PATH,
		 retimerId);
	sd_bus_emit_properties_changed(busHandle, retimerPath,
				       "com.Nvidia.ComputeHash", "Digest",
				       NULL);
	return ret;
}

static int property_get_hashDigest(sd_bus *bus, const char *path,
				   const char *interface, const char *property,
				   sd_bus_message *reply,
				   __attribute__((unused)) void *userdata,
				   __attribute__((unused)) sd_bus_error *error)
{
	assert(bus);
	assert(reply);
	assert(path);
	assert(interface);
	assert(property);
	unsigned retimerId = 0xFF;
	sscanf(path, "/com/Nvidia/ComputeHash/HGX_FW_PCIeRetimer_%u",
	       &retimerId);
	return sd_bus_message_append(reply, "s",
				     g_retimerHash[retimerId].hashDigest);
}

static int
property_get_hashAlgorithm(sd_bus *bus, const char *path, const char *interface,
			   const char *property, sd_bus_message *reply,
			   __attribute__((unused)) void *userdata,
			   __attribute__((unused)) sd_bus_error *error)
{
	assert(bus);
	assert(reply);
	assert(path);
	assert(interface);
	assert(property);
	return sd_bus_message_append(reply, "s", hashingAlgorithm);
}

int main()
{
	/* set stdout to line-buffered so it interleaves correctly with stderr */
	setvbuf(stdout, NULL, _IOLBF, 0);

	/* Connect to the system bus */
	int ret = sd_bus_open_system(&busHandle);
	if (ret < 0) {
		fprintf(stderr, "Failed to connect to system bus: %s\n",
			strerror(errno));
		return EXIT_FAILURE;
	}

	/* Request a well-known name */
	ret = sd_bus_request_name(busHandle, DBUS_SERVICE_NAME, 0);
	if (ret < 0) {
		fprintf(stderr, "Failed to request service name: %s\n",
			strerror(errno));
		sd_bus_unref(busHandle);
		return EXIT_FAILURE;
	}

	/* object Mapper */
	ret = sd_bus_add_object_manager(busHandle, NULL,
					"/com/Nvidia/ComputeHash");
	if (ret < 0) {
		fprintf(stderr, "Failed to add object manager: %s\n",
			strerror(errno));
		sd_bus_unref(busHandle);
		return EXIT_FAILURE;
	}

	/* Register 8 D-Bus objects, one for each value */
	for (int i = 0; i < 8; i++) {
		/* Construct the object path */
		char path[64];
		snprintf(path, sizeof(path), "%s%d", RETIMER_PATH, i);
		/* Construct the interface name */
		char interface[64];
		snprintf(interface, sizeof(interface),
			 "com.Nvidia.ComputeHash");
		/* Construct the method name */
		sd_bus_vtable vtable[] = {
			SD_BUS_VTABLE_START(0),
			SD_BUS_METHOD("GetHash", "u", NULL, method_computeHash,
				      SD_BUS_VTABLE_UNPRIVILEGED),
			SD_BUS_PROPERTY("Digest", "s", property_get_hashDigest,
					offsetof(hash_t, hashDigest),
					SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
			SD_BUS_PROPERTY("Algorithm", "s",
					property_get_hashAlgorithm,
					offsetof(hash_t, hashAlgo),
					SD_BUS_VTABLE_PROPERTY_CONST),
			SD_BUS_VTABLE_END
		};
		ret = sd_bus_add_object_vtable(busHandle, NULL, path, interface,
					       vtable, NULL);
		if (ret < 0) {
			fprintf(stderr, "Failed to register object: %s\n",
				strerror(errno));
			sd_bus_unref(busHandle);
			return EXIT_FAILURE;
		}
	}

	/* Run the bus loop */
	for (;;) {
		ret = sd_bus_process(busHandle, NULL);
		if (ret < 0) {
			fprintf(stderr, "Failed to process bus: %s\n",
				strerror(errno));
			sd_bus_unref(busHandle);
			return EXIT_FAILURE;
		}
		if (ret > 0) {
			continue;
		}
		ret = sd_bus_wait(busHandle, (uint64_t)-1);
		if (ret < 0) {
			fprintf(stderr, "Failed to wait on bus: %s\n",
				strerror(errno));
			sd_bus_unref(busHandle);
			return EXIT_FAILURE;
		}
	}
	/* Unreference the bus */
	sd_bus_unref(busHandle);
	return EXIT_SUCCESS;
}
