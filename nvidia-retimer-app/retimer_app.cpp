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

#include "retimer_app.hpp"

void RetimerApp::getDBusProperty(const DBusMapping &dbusMap,
                                 std::string &value)
{
    auto method = bus.new_method_call(gpuMgrService, dbusMap.objectPath.c_str(),
                                        dbusProperties, "Get");
    method.append(dbusMap.interface.c_str(), dbusMap.propertyName.c_str());
    auto reply = bus.call(method);
    PropertyValue propertyValue;
    reply.read(propertyValue);
    value = std::get<std::string>(propertyValue);
}

void RetimerApp::setDBusProperty(const DBusMapping &dbusMap,
                                 const std::string &value)
{
    auto method = bus.new_method_call(gpuMgrService, dbusMap.objectPath.c_str(),
                                        dbusProperties, "Set");
    PropertyValue propertyValue = value;
    method.append(dbusMap.interface.c_str(), dbusMap.propertyName.c_str(),
                    propertyValue);
    bus.call_noreply(method);
}

std::string RetimerApp::getSKUId(const std::string &objPath)
{
    DBusMapping dbusMap;
    std::string deviceId, vendorId;
    dbusMap.objectPath = objPath;
    dbusMap.interface = switchInterface;
    dbusMap.propertyName = "DeviceId";
    try
    {
        getDBusProperty(dbusMap, deviceId);
    }
    catch (const std::exception &e)
    {
        return "";
    }
    dbusMap.propertyName = "VendorId";
    try
    {
        getDBusProperty(dbusMap, vendorId);
    }
    catch (const std::exception &e)
    {
        return "";
    }
    if (deviceId.empty() || vendorId.empty())
    {
        std::cerr << "DeviceId or VendorId is empty for retimer " << objPath
                  << '\n';
        return "";
    }

    std::stringstream ss;
    ss << std::hex;
    ss << std::setw(4) << std::setfill('0') << deviceId.substr(2);
    ss << std::setw(4) << std::setfill('0') << vendorId.substr(2);
    std::string skuId = ss.str();
    transform(skuId.begin(), skuId.end(), skuId.begin(), ::toupper);
    return ("0x" + skuId);
}

void RetimerApp::softwareObjectCallback(sdbusplus::message::message &m)
{
    DBusMapping dbusMap;
    sdbusplus::message::object_path objPath;
    InterfaceMap interfaces;
    m.read(objPath, interfaces);

    std::string path(std::move(objPath));

    for (const auto &intf : interfaces)
    {
        if (intf.first == versionInterface)
        {
            if (path.find(retimerFWInventoryPath) == 0)
            {
                dbusMap.objectPath =
                    retimerFWInventoryPath + path.substr(retimerFWInventoryPath.size());
                dbusMap.interface = versionInterface;
                dbusMap.propertyName = "SoftwareId";
                try
                {
                    setDBusProperty(dbusMap, retimerSoftwareId);
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
    }
}

void RetimerApp::switchObjectCallback(sdbusplus::message::message &m)
{
    DBusMapping dbusMap;
    std::string path = m.get_path();

    if (path.find(retimerSwitchesBasePath) == 0)
    {
        std::string skuId = getSKUId(path);
        if (!skuId.empty())
        {
            std::string retimerId = path.substr(path.rfind("_") + 1);
            dbusMap.objectPath = retimerInventoryPath + retimerId;
            dbusMap.interface = assetInterface;
            dbusMap.propertyName = "SKU";
            try
            {
                setDBusProperty(dbusMap, skuId);
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        }
        else
        {
            std::cerr << "Error while getting SKU property" << "\n";
        }
    }
}

void RetimerApp::listenForGPUManagerEvents()
{
    try
    {
        switchObjectAddedMatch = std::make_unique<sdbusplus::bus::match_t>(
            bus,
            ("interface='org.freedesktop.DBus.Properties',type='signal',"
            "member='PropertiesChanged',arg0='xyz.openbmc_project.Inventory.Item."
            "Switch',"),
            std::bind(std::mem_fn(&RetimerApp::switchObjectCallback), this,
                    std::placeholders::_1));

        std::string arg0Path = "arg0path='" + retimerFWInventoryBasePath + "',";
        softwareObjectAddedMatch = std::make_unique<sdbusplus::bus::match_t>(
            bus,
            ("interface='org.freedesktop.DBus.ObjectManager',type='signal',"
            "member='InterfacesAdded'," +
            arg0Path),
            std::bind(std::mem_fn(&RetimerApp::softwareObjectCallback), this,
                    std::placeholders::_1));
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}