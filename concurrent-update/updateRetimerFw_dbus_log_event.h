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

#ifndef UPDATERETIMERFW_DBUS_LOG_EVENT_H_
#define UPDATERETIMERFW_DBUS_LOG_EVENT_H_
#include "updateRetimerFwOverI2C.h"
#include <stdbool.h>

/* no return, we will call and fail silently if busctl isn't present */
void emitLogMessage(char *message, char *arg0, char *arg1, char *severity,
		    char *resolution, bool genericMessage);
void setErrorNamespace(char **errorNamespace);

#endif
