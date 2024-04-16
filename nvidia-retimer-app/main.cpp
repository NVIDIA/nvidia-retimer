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
        // set SKU property
        std::string switchPath = retimerSwitchesBasePath + std::to_string(i) +
                                 retimerSwitchesPath + std::to_string(i);
        std::string skuId = retimerApp->getSKUId(switchPath);
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