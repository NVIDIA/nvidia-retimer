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

int main(void)
{
    DBusMapping dbusMap;
    auto bus = sdbusplus::bus::new_default();
    auto event = sdeventplus::Event::get_default();
    std::unique_ptr<RetimerApp> retimerApp = std::make_unique<RetimerApp>(bus);

    // handle cases where object may already exists
    for (size_t i = 0; i < numOfRetimers; i++)
    {
        std::string skuId{};
        try
        {
            const std::string& switchPath = retimerApp->getSwitchDBusObject(retimerSwitchesBasePath + std::to_string(i));
            skuId = retimerApp->getSKUId(switchPath);
        }
        catch (const std::exception &e)
        {
            // ignore the error when the app starts
        }
        if (!skuId.empty())
        {
            dbusMap.objectPath = retimerInventoryPath + std::to_string(i);
            dbusMap.interface = assetInterface;
            dbusMap.propertyName = "SKU";
            try
            {
                retimerApp->setDBusProperty(dbusMap, skuId);
            }
            catch (const std::exception &e)
            {
                // ignore the error when the app starts
            }
        }
        // set software Id property
        dbusMap.objectPath = retimerFWInventoryPath + std::to_string(i);
        dbusMap.interface = versionInterface;
        dbusMap.propertyName = "SoftwareId";
        try
        {
            retimerApp->setDBusProperty(dbusMap, retimerSoftwareId);
        }
        catch (const std::exception &e)
        {
            // ignore the error when the app starts
        }
    }
    retimerApp->listenForGPUManagerEvents();
    bus.attach_event(event.get(), SD_EVENT_PRIORITY_NORMAL);
    event.loop();
    return 0;
}