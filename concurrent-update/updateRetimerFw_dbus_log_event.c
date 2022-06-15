#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <systemd/sd-bus.h>
#include "updateRetimerFw_dbus_log_event.h"

#define BUFFER_LENGTH 1024

#define BUSCTL_COMMAND "busctl"
#define LOG_SERVICE "xyz.openbmc_project.Logging"
#define LOG_PATH "/xyz/openbmc_project/logging"
#define LOG_CREATE_INTERFACE "xyz.openbmc_project.Logging.Create"
#define LOG_CREATE_FUNCTION "Create"
#define LOG_CREATE_SIGNATURE "ssa{ss}"

void emitLogMessage(char *message, char *arg0, char *arg1, char *severity,
		    char *resolution)
{
	static sd_bus *bus = NULL;
	sd_bus_default_system(&bus);
	if (bus == NULL) {
		fprintf(stderr, "Bus is null");
		return;
	}
	sd_bus_error err = SD_BUS_ERROR_NULL;
	sd_bus_message *reply = NULL;
	char args[BUFFER_LENGTH];
	char updateMessage[BUFFER_LENGTH];
	snprintf(args, BUFFER_LENGTH, "%s,%s", arg0, arg1);
	snprintf(updateMessage, BUFFER_LENGTH, "Update.1.0.%s", message);
	debug_print("Attempting call\n");
	if (resolution) {
		if (sd_bus_call_method(
			    bus, LOG_SERVICE, LOG_PATH, LOG_CREATE_INTERFACE,
			    LOG_CREATE_FUNCTION, &err, &reply,
			    LOG_CREATE_SIGNATURE, updateMessage, severity, 3,
			    "REDFISH_MESSAGE_ID", updateMessage,
			    "REDFISH_MESSAGE_ARGS", args,
			    "xyz.openbmc_project.Logging.Entry.Resolution",
			    resolution) < 0) {
			fprintf(stderr, "Unable to call log creation function");
		}
	} else {
		if (sd_bus_call_method(
			    bus, LOG_SERVICE, LOG_PATH, LOG_CREATE_INTERFACE,
			    LOG_CREATE_FUNCTION, &err, &reply,
			    LOG_CREATE_SIGNATURE, updateMessage, severity, 2,
			    "REDFISH_MESSAGE_ID", updateMessage,
			    "REDFISH_MESSAGE_ARGS", args) < 0) {
			fprintf(stderr, "Unable to call log creation function");
		}
	}
	debug_print("Call completed\n");
}
