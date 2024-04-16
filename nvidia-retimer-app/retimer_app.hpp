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
#include <iomanip>
#include <iostream>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus/match.hpp>
#include <sdeventplus/event.hpp>
#include <sstream>

struct DBusMapping
{
    std::string objectPath;
    std::string interface;
    std::string propertyName;
};

const size_t numOfRetimers = 8;
// software id maps to component identifer in the package
constexpr auto retimerSoftwareId = "0x8000";
const std::string retimerSwitchesBasePath =
    "/xyz/openbmc_project/inventory/system/fabrics/HGX_PCIeRetimerTopology_";
const std::string retimerSwitchesPath = "/Switches/PCIeRetimer_";
const std::string retimerInventoryPath =
    "/xyz/openbmc_project/inventory/system/chassis/HGX_PCIeRetimer_";
const std::string retimerFWInventoryBasePath = "/xyz/openbmc_project/software/";
const std::string retimerFWInventoryPath =
    retimerFWInventoryBasePath + "HGX_FW_PCIeRetimer_";
constexpr auto gpuMgrService = "xyz.openbmc_project.GpuMgr";
constexpr auto dbusProperties = "org.freedesktop.DBus.Properties";
constexpr auto switchInterface = "xyz.openbmc_project.Inventory.Item.Switch";
constexpr auto assetInterface = "xyz.openbmc_project.Inventory.Decorator.Asset";
constexpr auto versionInterface = "xyz.openbmc_project.Software.Version";

using PropertyValue = std::variant<std::string>;
using Interface = std::string;
using Property = std::string;
using Value =
    std::variant<bool, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t,
                 uint64_t, double, std::string, std::vector<uint8_t>>;
using PropertyMap = std::map<Property, Value>;
using InterfaceMap = std::map<Interface, PropertyMap>;

class RetimerApp
{
public:
    /**
     * @brief Construct a new Retimer app object
     *
     * @param[in] bus
     */
    RetimerApp(sdbusplus::bus::bus &bus) : bus(bus) {}
    /**
     * @brief Get property from D-Bus object
     *
     * @param[in] dbusMap
     * @param[in] value
     */
    void getDBusProperty(const DBusMapping &dbusMap, std::string &value);

    /**
     * @brief Set property of D-Bus object
     *
     * @param[in] dbusMap
     * @param[in] value
     */
    void setDBusProperty(const DBusMapping &dbusMap, const std::string &value);

    /**
     * @brief get SKU id for retimer. SKU ID is formed by concatenating device and
     * vendor Id both are of size 2 bytes and leading zeros is added to make sure
     * SKU is always 4 bytes
     *
     * @param[in] objPath - retimer device object path
     * @return std::string - SKU property
     */
    std::string getSKUId(const std::string &objPath);

    /**
     * @brief switch object callback
     *
     * @param[in] m - message
     */
    void softwareObjectCallback(sdbusplus::message::message &m);

    /**
     * @brief switch object callback
     *
     * @param[in] m - message
     */
    void switchObjectCallback(sdbusplus::message::message &m);

    /**
     * @brief add event listener in nvswitch and software id path
     *
     * @param[in] bus
     * @param[in] loop
     */
    void listenForGPUManagerEvents();

private:
    sdbusplus::bus::bus &bus;
    std::unique_ptr<sdbusplus::bus::match_t> switchObjectAddedMatch;
    std::unique_ptr<sdbusplus::bus::match_t> softwareObjectAddedMatch;
};
