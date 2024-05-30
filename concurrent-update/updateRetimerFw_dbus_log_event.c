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
		    char *resolution, bool genericMessage)
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
	if (genericMessage) {
		snprintf(updateMessage, BUFFER_LENGTH, "%s", message);
	} else
		snprintf(updateMessage, BUFFER_LENGTH, "Update.1.0.%s",
			 message);

	debug_print("Attempting call\n");
	if (resolution) {
		if (sd_bus_call_method(
			    bus, LOG_SERVICE, LOG_PATH, LOG_CREATE_INTERFACE,
			    LOG_CREATE_FUNCTION, &err, &reply,
			    LOG_CREATE_SIGNATURE, updateMessage, severity, 4,
			    "REDFISH_MESSAGE_ID", updateMessage,
			    "REDFISH_MESSAGE_ARGS", args,
			    "xyz.openbmc_project.Logging.Entry.Resolution",
			    resolution, "namespace", "FWUpdate") < 0) {
			fprintf(stderr, "Unable to call log creation function");
		}
	} else {
		if (sd_bus_call_method(bus, LOG_SERVICE, LOG_PATH,
				       LOG_CREATE_INTERFACE,
				       LOG_CREATE_FUNCTION, &err, &reply,
				       LOG_CREATE_SIGNATURE, updateMessage,
				       severity, 3, "REDFISH_MESSAGE_ID",
				       updateMessage, "REDFISH_MESSAGE_ARGS",
				       args, "namespace", "FWUpdate") < 0) {
			fprintf(stderr, "Unable to call log creation function");
		}
	}
	debug_print("Call completed\n");
}
