#include "rt_manager.hpp"

#include <fmt/format.h>
#include <sys/types.h>
#include <unistd.h>

#include <regex>
#include <xyz/openbmc_project/Common/Device/error.hpp>

using namespace phosphor::logging;
using namespace sdbusplus::xyz::openbmc_project::Common::Device::Error;
using namespace nvidia::retimer::common;

namespace nvidia::retimer::manager
{

RtManager::RtManager(sdbusplus::bus::bus& bus) :
    bus(bus)
{
    using namespace sdeventplus;

    nlohmann::json fruJson = loadJSONFile(RT_JSON_PATH);
    if (fruJson == nullptr)
    {
        log<level::ERR>("InternalFailure when parsing the JSON file");
        return;
    }
    for (const auto& fru : fruJson.at("RT"))
    {
        try
        {
            const std::string baseinvInvPath = BASE_INV_PATH;

            std::string id = fru.at("Index");
            std::string busN = fru.at("Bus");
            std::string address = fru.at("Address");
            std::string invpath = baseinvInvPath + "/retimer" + id;

            uint8_t busId = std::stoi(busN);
            uint8_t devAddr = std::stoi(address, nullptr, 16);

            auto invMatch = std::find_if(
                rtInvs.begin(), rtInvs.end(), [&invpath](auto& inv) {
                    return inv->getInventoryPath() == invpath;
                });
            if (invMatch != rtInvs.end())
            {
                continue;
            }
            auto inv =
                std::make_unique<Retimer>(bus, invpath, busId, devAddr, id);
            rtInvs.emplace_back(std::move(inv));
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}

} // namespace nvidia::cec::manager
