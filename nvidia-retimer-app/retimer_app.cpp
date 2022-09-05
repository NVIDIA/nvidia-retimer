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

    if (path.find(retimerSwitchesPath) == 0)
    {
        std::string skuId = getSKUId(path);
        if (!skuId.empty())
        {
            dbusMap.objectPath =
                retimerInventoryPath + path.substr(retimerSwitchesPath.size());
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