#pragma once
#include "xyz/openbmc_project/Common/error.hpp"

#include <fmt/format.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/elog.hpp>
#include <phosphor-logging/log.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;
using namespace phosphor::logging;

namespace nvidia::retimer::common
{

inline json loadJSONFile(const char* path)
{
    std::ifstream ifs(path);

    if (!ifs.good())
    {
        log<level::ERR>(std::string("Unable to open file "
                                    "PATH=" +
                                    std::string(path))
                            .c_str());
        return nullptr;
    }
    auto data = json::parse(ifs, nullptr, false);
    if (data.is_discarded())
    {
        log<level::ERR>(std::string("Failed to parse json "
                                    "PATH=" +
                                    std::string(path))
                            .c_str());
        return nullptr;
    }
    return data;
}
inline std::string getCommand()
{
    return "";
}

template <typename T, typename... Types>
inline std::string getCommand(T arg1, Types... args)
{
    std::string cmd = " " + arg1 + getCommand(args...);

    return cmd;
}
template <typename T, typename... Types>
inline std::vector<std::string> executeCmd(T&& path, Types... args)
{
    std::vector<std::string> stdOutput;
    std::array<char, 128> buffer;

    std::string cmd = path + getCommand(args...);

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                  pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        stdOutput.emplace_back(buffer.data());
    }

    return stdOutput;
}

inline bool invalidChar(char c)
{
    return !std::isprint(static_cast<unsigned char>(c));
}
inline void stripUnicode(std::string& str)
{
    str.erase(std::remove_if(str.begin(), str.end(),
                             invalidChar),
              str.end());
}
class Util
{
  protected:
    uint8_t b, d;

  public:
    virtual ~Util() = default;

    //   protected:
    virtual bool getPresence() const
    {
        return true;
    }

    virtual std::string runCommand(std::string command) const
    {
        std::string s = "";
        try
        {
            std::string cmd =
                fmt::format("aries-info {0:s} {1:d} {2:#02x}",
                            command, b, d);
            auto output = executeCmd(cmd);
            for (const auto& i : output)
                s += i;
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
        std::cerr << command << " =  " << s << std::endl;
        stripUnicode(s);
        return s;
    }

    virtual std::string getSerialNumber() const
    {
        return runCommand("serial");
    }
    virtual std::string getPartNumber() const
    {
        return runCommand("pn");
    }
    virtual std::string getManufacturer() const
    {
        return runCommand("manufacturer");
    }
    virtual std::string getModel() const
    {
        return runCommand("model");
    }
    virtual std::string getVersion() const
    {
        return runCommand("version");
    }
};
} // namespace nvidia::cec::common
