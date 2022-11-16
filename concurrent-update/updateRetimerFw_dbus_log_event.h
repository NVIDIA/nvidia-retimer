#ifndef UPDATERETIMERFW_DBUS_LOG_EVENT_H_
#define UPDATERETIMERFW_DBUS_LOG_EVENT_H_
#include "updateRetimerFwOverI2C.h"
#include <stdbool.h>

/* no return, we will call and fail silently if busctl isn't present */
void emitLogMessage(char *message, char *arg0, char *arg1, char *severity,
		    char *resolution, bool genericMessage);

#endif
