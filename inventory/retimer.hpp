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

#include "rt_util.hpp"

#include <filesystem>
#include <iostream>
#include <sdbusplus/bus/match.hpp>
#include <stdexcept>
#include <xyz/openbmc_project/Association/Definitions/server.hpp>
#include <xyz/openbmc_project/Inventory/Decorator/Asset/server.hpp>
#include <xyz/openbmc_project/Inventory/Item/Chassis/server.hpp>
#include <xyz/openbmc_project/Inventory/Item/server.hpp>
#include <xyz/openbmc_project/State/Decorator/OperationalStatus/server.hpp>

using namespace nvidia::retimer::common;

namespace nvidia::retimer::device
{

using RtInherit = sdbusplus::server::object::object<
    sdbusplus::xyz::openbmc_project::State::Decorator::server::
        OperationalStatus,
    sdbusplus::xyz::openbmc_project::Inventory::server::Item,
    sdbusplus::xyz::openbmc_project::Inventory::Item::server::Chassis,
    sdbusplus::xyz::openbmc_project::Inventory::Decorator::server::Asset,
    sdbusplus::xyz::openbmc_project::Association::server::Definitions>;

/**
 * @class Retimer
 */
class Retimer : public RtInherit, public Util
{
  public:
    Retimer() = delete;
    Retimer(const Retimer&) = delete;
    Retimer(Retimer&&) = delete;
    Retimer& operator=(const Retimer&) = delete;
    Retimer& operator=(Retimer&&) = delete;
    ~Retimer() = default;

    Retimer(sdbusplus::bus::bus& bus, const std::string& objPath, uint8_t busN,
        uint8_t address, const std::string& name) :
        RtInherit(bus, (objPath).c_str(), RtInherit::action::defer_emit),
        bus(bus)
    {

        b = busN;
        d = address;
        sdbusplus::xyz::openbmc_project::Inventory::Decorator::server::Asset::
            manufacturer(getManufacturer());
        sdbusplus::xyz::openbmc_project::Inventory::Decorator::server::Asset::
            model(getModel());
        sdbusplus::xyz::openbmc_project::Inventory::Decorator::server::Asset::
            partNumber(getPartNumber());
        sdbusplus::xyz::openbmc_project::Inventory::Decorator::server::Asset::
            serialNumber(getSerialNumber());
        sdbusplus::xyz::openbmc_project::Inventory::server::Item::present(
            getPresence());
        sdbusplus::xyz::openbmc_project::Inventory::server::Item::prettyName(
            "Retimer " + name);
        sdbusplus::xyz::openbmc_project::State::Decorator::server::
            OperationalStatus::functional(true);
        auto chassisType =
            sdbusplus::xyz::openbmc_project::Inventory::Item::server::Chassis::
                convertChassisTypeFromString("xyz.openbmc_project.Inventory."
                                             "Item.Chassis.ChassisType.Module");
        sdbusplus::xyz::openbmc_project::Inventory::Item::server::Chassis::type(
            chassisType);

        emit_object_added();
    }

    const std::string& getInventoryPath() const
    {
        return inventoryPath;
    }

  private:
    /** @brief systemd bus member */
    sdbusplus::bus::bus& bus;
    std::string inventoryPath;
};

} // namespace nvidia::cec::device
