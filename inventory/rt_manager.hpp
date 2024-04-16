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
