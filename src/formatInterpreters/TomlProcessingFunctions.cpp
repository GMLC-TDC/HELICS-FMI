/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TomlProcessingFunctions.hpp"

#include <fstream>
#include <string>

namespace helics_fmi::fileops {

bool hasTomlExtension(std::string_view tomlString)
{
    auto ext = tomlString.substr(tomlString.length() - 4);
    return ((ext == "toml") || (ext == "TOML") || (ext == ".ini") || (ext == ".INI"));
}

toml::value loadToml(const std::string& tomlString)
{
    if (tomlString.size() > 128) {
        try {
            return helics_fmi::fileops::loadTomlStr(tomlString);
        }
        catch (const std::invalid_argument&) {
            // just pass through this was an assumption
        }
    }
    std::ifstream file(tomlString, std::ios_base::binary);

    try {
        if (file.is_open()) {
            return toml::parse(file);
        }
        return loadTomlStr(tomlString);
    }
    catch (const toml::exception& se) {
        throw(std::invalid_argument(se.what()));
    }
}

toml::value loadTomlStr(const std::string& tomlString)
{
    try {
        std::istringstream tstring(tomlString);
        toml::value pr = toml::parse(tstring);
        return pr;
    }
    catch (const toml::exception& se) {
        throw(std::invalid_argument(se.what()));
    }
}
// NOLINTNEXTLINE
static const std::string emptyString;

std::string getName(const toml::value& element)
{
    std::string retval = toml::find_or(element, "key", emptyString);
    if (retval.empty()) {
        retval = toml::find_or(element, "name", emptyString);
    }
    return retval;
}

std::string tomlAsString(const toml::value& element)
{
    switch (element.type()) {
        case toml::value_t::string:
            return element.as_string(std::nothrow_t());
        case toml::value_t::floating:
            return std::to_string(element.as_floating(std::nothrow_t()));
        case toml::value_t::integer:
            return std::to_string(element.as_integer(std::nothrow_t()));
        default: {
            std::ostringstream str;
            str << element;
            return str.str();
        }
    }
}
}  // namespace helics_fmi::fileops
