#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-bus.h>
#include <systemd/sd-event.h>
#include <pthread.h>
#include <time.h>
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

#define STOP "com.Nvidia.BackgroundComputeHash.ComputationStatus.Stopped"
#define RUN "com.Nvidia.BackgroundComputeHash.ComputationStatus.Running"
#define CSM_DISABLED "xyz.openbmc_project.State.FeatureReady.States.Disabled"
#define CSM_STARTING "xyz.openbmc_project.State.FeatureReady.States.Starting"
#define CSM_ENABLED "xyz.openbmc_project.State.FeatureReady.States.Enabled"
#define HASHCOMPUTE_STATUS_PATH "/com/Nvidia/ComputeHash"
#define HASHCOMPUTE_STATUS_INTERFACE "com.Nvidia.BackgroundComputeHash"

// Time taken to compute hash for 8 retimer is approx. 8 min.
// So, Hash computation to be done at the interval of 10 min.
#define HASH_COMPUTE_INTERVAL 600   
#define STATUS_CHECK_INTERVAL 5 
#define WAIT_TIME 5

sd_bus *busHandle = NULL;

typedef struct hash_compute {
	uint64_t timeStamp;
	char hashAlgo[64];
	char hashDigest[97];
} hash_t;

/**
* @struct background_compute_status
 * @param[in] enabled - field representing current Status of hash computation.
 * @param[in] request - field representing requested Status, on which computation need to be transitioned .
*/
typedef struct background_compute_status {
	char enabled[59];
	char request[59];
} status_t;
status_t status;

const char hashingAlgorithm[64] = "SHA384";

hash_t g_retimerHash[MAX_RETIMERS];

/**
* @brief enum operation
 * @param[in] READ - enum indicating thread to read from variable.
 * @param[in] WRITE - enum indicating thread to write into variable.
*/
enum operation { READ, WRITE };

pthread_mutex_t lock_digest;
pthread_mutex_t lock_timestamp;
pthread_mutex_t lock_enable;
pthread_mutex_t lock_request;

/**
 * @brief read/write value of Enable property by different thread,
 *         provide locking mechanism.
 * @param[in] value - pointer to read/write enable value
 * @param[in] operation - READ/WRITE
 */
void readWriteEnable(char *value, enum operation read_write_operation)
{
	int len = sizeof(status.enabled);
	pthread_mutex_lock(&lock_enable);
	if (read_write_operation == WRITE) {
		strncpy(status.enabled, value, len);
	} else if (read_write_operation == READ) {
		strncpy(value, status.enabled, len);
	}
	pthread_mutex_unlock(&lock_enable);
}

/**
 * @brief read/write value of request by different threads,
 *         provide locking mechanism.
 * @param[in] value - pointer to read/write request value
 * @param[in] operation - READ/WRITE
 */
void readWriteRequest(char *value, enum operation read_write_operation)
{
	int len = sizeof(status.request);
	pthread_mutex_lock(&lock_request);
	if (read_write_operation == WRITE) {
		strncpy(status.request, value, len);
	} else if (read_write_operation == READ) {
		strncpy(value, status.request, len);
	}
	pthread_mutex_unlock(&lock_request);
}
/**
 * @brief read/write value of Timestamp property by different thread,
 *         provide locking mechanism.
 * @param[in] time - pointer to read/write Timestamp value
 * @param[in] retimerId - retimer number for which operation is to be performed
 * @param[in] operation - READ/WRITE
 */
void readWriteTimeStamp(uint64_t *time, int retimerId, enum operation read_write_operation)
{
	pthread_mutex_lock(&lock_timestamp);
	if (read_write_operation == WRITE) {
		g_retimerHash[retimerId].timeStamp = *time;
	} else if (read_write_operation == READ) {
		*time = g_retimerHash[retimerId].timeStamp;
	}
	pthread_mutex_unlock(&lock_timestamp);
}
/**
 * @brief read/write value of Hash Digest property for different retimer by different thread,
 *         provide locking mechanism.
 * @param[in] hashValue - pointer to read/write Hash Digest value
 * @param[in] retimerId - Retimer number for which operation is to be performed.
 * @param[in] operation - READ/WRITE
 */
void readWriteHash(char *hashValue, int retimerId, enum operation read_write_operation)
{
	pthread_mutex_lock(&lock_digest);
	int hash_len = sizeof(g_retimerHash[retimerId].hashDigest);
	if (read_write_operation == WRITE) {
		strncpy(g_retimerHash[retimerId].hashDigest, hashValue,
			hash_len);
	} else if (read_write_operation == READ) {
		strncpy(hashValue, g_retimerHash[retimerId].hashDigest,
			hash_len);
	}
	pthread_mutex_unlock(&lock_digest);
}

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
	ret = copyImageToFpga(dummyfd, fd, FPGA_I2C_CNTRL_ADDR);
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
	
	readWriteHash(hashValue, retimerId, WRITE);
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

static int property_get_status(sd_bus *bus, const char *path,
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
	char value[59] = { 0 };
	readWriteEnable(value, READ);
	return sd_bus_message_append(reply, "s", value);
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
	char hashValue[97] = { 0 };
	readWriteHash(hashValue, retimerId, READ);
	return sd_bus_message_append(reply, "s", hashValue);
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

static int property_get_timeStamp(sd_bus *bus, const char *path,
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
	uint64_t time = 0;
	readWriteTimeStamp(&time, retimerId, READ);
	return sd_bus_message_append(reply, "t", time);
}

/**
 * @brief Function emitting callback signal when Status of hash compuatation changes. 
 */
int propertyChangeCallback(sd_bus_message *m,
			   __attribute__((unused)) void *userdata,
			   __attribute__((unused)) sd_bus_error *ec)
{
	const char *state = NULL;
	const char *interface = NULL;
	const char *property_name = NULL;

	sd_bus_message_read(m, "sa{sv}", &interface, 1, &property_name, "s",
			    &state);

	if (state) {
		if (strcmp(state, CSM_STARTING) == 0) {
			readWriteRequest(STOP, WRITE);
		} else if (strcmp(state, CSM_DISABLED) == 0) {
			readWriteRequest(RUN, WRITE);
		} else {
			fprintf(stderr, "Unknown state of CSM service \n");
		}
	}
	return 0;
}

/**
 * @brief continuoulsy perform hash computation at regular interval in background on 
 *        a child thread. Can be stop/start using external dbus method call. 
 */
void *backgroundHashCompute()
{
	uint64_t set_time;
	set_time = time(NULL);
	uint64_t interval = HASH_COMPUTE_INTERVAL;
	while (1) {
		uint64_t curr_time;
		curr_time = time(NULL);
		if (curr_time > set_time) {
			set_time = curr_time + interval;

			for (int retimerId = 0; retimerId < MAX_RETIMERS;
			     retimerId++) {
				char value[59] = { 0 };
				readWriteRequest(value, READ);
				if (strcmp(value, STOP) == 0) {
					readWriteEnable(value, WRITE);
					fprintf(stdout,
						"Background calculation stopped based on request \n");
					set_time = time(NULL);
					break;
				} else {
					readWriteEnable(value, WRITE);
				}
				int ret;
				ret = readFWImagenComputeHash(retimerId);
				if (ret) {
					fprintf(stderr,
						"Error Occured while computing Hash for Retimer %d \n",
						retimerId);
				} else {
					curr_time = time(NULL);
					readWriteTimeStamp(&curr_time,
							   retimerId, WRITE);
				}
			}
		}
		sleep(WAIT_TIME);
	}
	return NULL;
}
char prev[59] = { 0 };

/**
 * @brief check for current status of hash computation running/stopped on fixed interval of time. 
 */
int checkStatusChange(__attribute__((unused)) sd_event_source *src,
		      __attribute__((unused)) uint64_t usec,
		      __attribute__((unused)) void *userdata)
{
		char val[59] = { 0 };
	readWriteEnable(val, READ);
	if (strcmp(val, prev) != 0) {
		readWriteEnable(prev, READ);
		int r = sd_bus_emit_properties_changed(
			busHandle, HASHCOMPUTE_STATUS_PATH,
			HASHCOMPUTE_STATUS_INTERFACE, "Status", NULL);
		if (r < 0) {
			fprintf(stderr,
				"error while emitting property change signal %d \n",
				r);
		}
	}
	sd_event_source_set_time_relative(src,
					  STATUS_CHECK_INTERVAL * 1000 * 1000);
	sd_event_source_set_enabled(src, SD_EVENT_ON);
	return 1;
}

static const sd_bus_vtable vtable_x[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_PROPERTY("Status", "s", property_get_status,
			offsetof(status_t, enabled),
			SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
	SD_BUS_VTABLE_END
};
int main()
{
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

    strncpy(status.enabled,RUN,59);
    strncpy(status.request, RUN,59);
    ret = sd_bus_add_object_vtable(busHandle, NULL, HASHCOMPUTE_STATUS_PATH, HASHCOMPUTE_STATUS_INTERFACE,
 					       vtable_x, NULL);    
	if (ret < 0)
	{
		fprintf(stderr, "Failed to register object : %s\n",
			 strerror(errno));
		sd_bus_unref(busHandle);
		return EXIT_FAILURE;
	}
	/* Register 8 D-Bus objects, one for each value */
	for (int i = 0; i < 8; i++) {
		g_retimerHash[i].timeStamp = 0;
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
			SD_BUS_PROPERTY("Digest", "s", property_get_hashDigest,
					offsetof(hash_t, hashDigest),
					SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
			SD_BUS_PROPERTY("Algorithm", "s",
					property_get_hashAlgorithm,
					offsetof(hash_t, hashAlgo),
					SD_BUS_VTABLE_PROPERTY_CONST),
			SD_BUS_PROPERTY("TimeStamp", "t", property_get_timeStamp,
					offsetof(hash_t, timeStamp),
					SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
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

	if (pthread_mutex_init(&lock_digest, NULL) != 0) {
		fprintf(stderr, "Failed to initialize mutex for digest\n");
		return EXIT_FAILURE;
	}
	if (pthread_mutex_init(&lock_timestamp, NULL) != 0) {
		fprintf(stderr, "Failed to initialize mutex for timeStamp\n");
		return EXIT_FAILURE;
	}
	if (pthread_mutex_init(&lock_enable, NULL) != 0) {
		fprintf(stderr, "Failed to initialize mutex for enable\n");
		return EXIT_FAILURE;
	}
	if (pthread_mutex_init(&lock_request, NULL) != 0) {
		fprintf(stderr, "Failed to initialize mutex for request\n");
		return EXIT_FAILURE;
	}
	pthread_t t1;
	pthread_create(&t1, NULL, backgroundHashCompute, NULL);
	const char *match =
		"type='signal',interface='org.freedesktop.DBus.Properties',member='PropertiesChanged',path='/xyz/openbmc_project/state/configurableStateManager/FWUpdate'";
	sd_bus_add_match_async(busHandle, NULL, match, propertyChangeCallback,
			       NULL, NULL);

	sd_event *event = NULL;
	sd_event_default(&event);
	sd_bus_attach_event(busHandle, event, SD_EVENT_PRIORITY_NORMAL);
	sd_event_source *src = NULL;
	sd_event_add_time(event, &src, CLOCK_MONOTONIC, 0, 0, checkStatusChange,
			  NULL);
	sd_event_loop(event);
	/* Unreference the bus */
	pthread_join(t1, NULL);
	sd_bus_unref(busHandle);
	return EXIT_SUCCESS;
}
