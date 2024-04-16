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

#pragma once

#include "config.h"

#include "retimer.hpp"

#include <nlohmann/json.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus/match.hpp>
#include <sdeventplus/event.hpp>

using namespace nvidia::retimer::device;
using namespace phosphor::logging;
using json = nlohmann::json;

namespace nvidia::retimer::manager
{

class RtManager
{
  public:
    RtManager() = delete;
    ~RtManager() = default;
    RtManager(const RtManager&) = delete;
    RtManager& operator=(const RtManager&) = delete;
    RtManager(RtManager&&) = delete;
    RtManager& operator=(RtManager&&) = delete;

    RtManager(sdbusplus::bus::bus& bus);

  private:
    sdbusplus::bus::bus& bus;

    std::vector<std::unique_ptr<Retimer>> rtInvs;
};

} // namespace nvidia::cec::manager
