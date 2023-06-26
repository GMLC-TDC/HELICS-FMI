/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "FmiHelics.hpp"

#include <fmt/format.h>
#include <unordered_map>

namespace helicsfmi {
std::string_view getHelicsTypeString(fmi_variable_type type)
{
    switch (type) {
        case fmi_variable_type::boolean:
            return helics::typeNameString<bool>();
        case fmi_variable_type::real:
        case fmi_variable_type::numeric:
            return helics::typeNameString<double>();
        case fmi_variable_type::integer:
            return helics::typeNameString<std::int64_t>();
        case fmi_variable_type::string:
        default:
            return helics::typeNameString<std::string>();
        case fmi_variable_type::enumeration:
            return helics::typeNameString<int>();
    }
}

helics::DataType getHelicsType(fmi_variable_type type)
{
    switch (type) {
        case fmi_variable_type::boolean:
            return helics::helicsType<bool>();
        case fmi_variable_type::real:
        case fmi_variable_type::numeric:
            return helics::helicsType<double>();
        case fmi_variable_type::integer:
            return helics::helicsType<std::int64_t>();
        case fmi_variable_type::string:
        default:
            return helics::helicsType<std::string>();
        case fmi_variable_type::enumeration:
            return helics::helicsType<int>();
    }
}

void publishOutput(helics::Publication& pub, fmi2Object* fmiObj, std::size_t index, bool logValues)
{
    const auto& var = fmiObj->getOutput(static_cast<int>(index));
    switch (var.type) {
        case fmi_variable_type::boolean: {
            auto val = fmiObj->get<fmi2Boolean>(var);
            pub.publish(val != fmi2False);
            if (logValues) {
                fmiObj->logMessage("data", fmt::format("publishing {} to {}", val, pub.getName()));
            }
        } break;
        case fmi_variable_type::integer:
        case fmi_variable_type::enumeration: {
            auto val = fmiObj->get<std::int64_t>(var);
            pub.publish(val);
            if (logValues) {
                fmiObj->logMessage("data", fmt::format("publishing {} to {}", val, pub.getName()));
            }
        } break;
        case fmi_variable_type::real:
        case fmi_variable_type::numeric: {
            auto val = fmiObj->get<double>(var);
            pub.publish(val);
            if (logValues) {
                fmiObj->logMessage("data", fmt::format("publishing {} to {}", val, pub.getName()));
            }
        } break;
        case fmi_variable_type::string:
        default: {
            auto val = fmiObj->get<std::string_view>(var);
            pub.publish(val);
            if (logValues) {
                fmiObj->logMessage("data", fmt::format("publishing {} to {}", val, pub.getName()));
            }
        } break;
    }
}

void grabInput(helics::Input& inp, fmi2Object* fmiObj, std::size_t index, bool logValues)
{
    if (!inp.isUpdated()) {
        return;
    }
    const auto& var = fmiObj->getInput(static_cast<int>(index));
    switch (var.type) {
        case fmi_variable_type::boolean: {
            auto val = inp.getValue<fmi2Boolean>();
            fmiObj->set(var, val);
            if (logValues) {
                fmiObj->logMessage("data", fmt::format("received {} for {}", val, inp.getName()));
            }
        } break;
        case fmi_variable_type::integer:
        case fmi_variable_type::enumeration: {
            auto val = inp.getValue<fmi2Integer>();
            fmiObj->set(var, val);
            if (logValues) {
                fmiObj->logMessage("data", fmt::format("received {} for {}", val, inp.getName()));
            }
        } break;
        case fmi_variable_type::real:
        case fmi_variable_type::numeric: {
            auto val = inp.getValue<fmi2Real>();
            fmiObj->set(var, val);
            if (logValues) {
                fmiObj->logMessage("data", fmt::format("received {} for {}", val, inp.getName()));
            }
        } break;
        case fmi_variable_type::string:
        default: {
            auto val = inp.getValue<std::string>();
            fmiObj->set(var, val);
            if (logValues) {
                fmiObj->logMessage("data", fmt::format("received {} for {}", val, inp.getName()));
            }
        } break;
    }
}

void setDefault(helics::Input& inp, fmi2Object* fmiObj, std::size_t index)
{
    const auto& var = fmiObj->getInput(static_cast<int>(index));
    switch (var.type) {
        case fmi_variable_type::boolean: {
            auto val = fmiObj->get<bool>(var);
            inp.setDefault(val);
        } break;
        case fmi_variable_type::integer:
        case fmi_variable_type::enumeration: {
            auto val = fmiObj->get<std::int64_t>(var);
            inp.setDefault(val);
        } break;
        case fmi_variable_type::real:
        case fmi_variable_type::numeric: {
            auto val = fmiObj->get<double>(var);
            inp.setDefault(val);
        } break;
        case fmi_variable_type::string:
        default: {
            auto val = fmiObj->get<std::string_view>(var);
            inp.setDefault(val);
        } break;
    }
}

static const std::unordered_map<std::string_view, int> logLevelsTranslation{
    {"logEvents", HELICS_LOG_LEVEL_DEBUG},
    {"logSingularLinearSystems", HELICS_LOG_LEVEL_DATA},
    {"logNonLinearSystems", HELICS_LOG_LEVEL_DATA},
    {"logDynamicStateSelection", HELICS_LOG_LEVEL_DATA},
    {"logStatusWarning", HELICS_LOG_LEVEL_WARNING},
    {"logStatusDiscard", HELICS_LOG_LEVEL_WARNING},
    {"logStatusError", HELICS_LOG_LEVEL_ERROR},
    {"logStatusFatal", HELICS_LOG_LEVEL_ERROR},
    {"logStatusPending", HELICS_LOG_LEVEL_TIMING},
    {"logAll", HELICS_LOG_LEVEL_DEBUG},
    // the rest are HELICS logging levels
    {"error", HELICS_LOG_LEVEL_ERROR},
    {"profiling", HELICS_LOG_LEVEL_PROFILING},
    {"warning", HELICS_LOG_LEVEL_WARNING},
    {"summary", HELICS_LOG_LEVEL_SUMMARY},
    {"connections", HELICS_LOG_LEVEL_CONNECTIONS},
    {"interfaces", HELICS_LOG_LEVEL_INTERFACES},
    {"timing", HELICS_LOG_LEVEL_TIMING},
    {"data", HELICS_LOG_LEVEL_DATA},
    {"debug", HELICS_LOG_LEVEL_DEBUG},
    {"trace", HELICS_LOG_LEVEL_TRACE},

    {"NONE", HELICS_LOG_LEVEL_NO_PRINT},
    {"NO_PRINT", HELICS_LOG_LEVEL_NO_PRINT},
    {"NOPRINT", HELICS_LOG_LEVEL_NO_PRINT},
    {"ERROR", HELICS_LOG_LEVEL_ERROR},
    {"PROFILING", HELICS_LOG_LEVEL_PROFILING},
    {"WARNING", HELICS_LOG_LEVEL_WARNING},
    {"SUMMARY", HELICS_LOG_LEVEL_SUMMARY},
    {"CONNECTIONS", HELICS_LOG_LEVEL_CONNECTIONS},
    {"INTERFACES", HELICS_LOG_LEVEL_INTERFACES},
    {"TIMING", HELICS_LOG_LEVEL_TIMING},
    {"DATA", HELICS_LOG_LEVEL_DATA},
    {"DEBUG", HELICS_LOG_LEVEL_DEBUG},
    {"TRACE", HELICS_LOG_LEVEL_TRACE},
    {"None", HELICS_LOG_LEVEL_NO_PRINT},
    {"No_print", HELICS_LOG_LEVEL_NO_PRINT},
    {"No_Print", HELICS_LOG_LEVEL_NO_PRINT},
    {"Noprint", HELICS_LOG_LEVEL_NO_PRINT},
    {"NoPrint", HELICS_LOG_LEVEL_NO_PRINT},
    {"Error", HELICS_LOG_LEVEL_ERROR},
    {"Profiling", HELICS_LOG_LEVEL_PROFILING},
    {"Warning", HELICS_LOG_LEVEL_WARNING},
    {"Summary", HELICS_LOG_LEVEL_SUMMARY},
    {"Connections", HELICS_LOG_LEVEL_CONNECTIONS},
    {"Interfaces", HELICS_LOG_LEVEL_INTERFACES},
    {"Timing", HELICS_LOG_LEVEL_TIMING},
    {"Data", HELICS_LOG_LEVEL_DATA},
    {"Debug", HELICS_LOG_LEVEL_DEBUG},
    {"Trace", HELICS_LOG_LEVEL_TRACE}};

int fmiCategory2HelicsLogLevel(std::string_view category)
{
    auto llevel = logLevelsTranslation.find(category);
    if (llevel != logLevelsTranslation.end()) {
        return llevel->second;
    }
    if (category.find("arning") != std::string::npos) {
        // first letter is missing, capitalization shouldn't matter and there isn't much confusion
        return HELICS_LOG_LEVEL_WARNING;
    }
    if (category.find("rror") != std::string::npos) {
        return HELICS_LOG_LEVEL_ERROR;
    }
    return HELICS_LOG_LEVEL_DEBUG;
}

static const std::unordered_map<std::string_view, FileType> typeMap{{"fmu", FileType::fmu},
                                                                    {"FMU", FileType::fmu},
                                                                    {"Fmu", FileType::fmu},
                                                                    {"json", FileType::json},
                                                                    {"JSON", FileType::json},
                                                                    {"Json", FileType::json},
                                                                    {"jsn", FileType::json},
                                                                    {"JSN", FileType::json},
                                                                    {"toml", FileType::toml},
                                                                    {"TOML", FileType::toml},
                                                                    {"Toml", FileType::toml},
                                                                    {"tml", FileType::toml},
                                                                    {"toml", FileType::toml},
                                                                    {"xml", FileType::xml},
                                                                    {"XML", FileType::xml},
                                                                    {"Xml", FileType::xml},
                                                                    {"ssp", FileType::ssp},
                                                                    {"SSP", FileType::ssp},
                                                                    {"Ssp", FileType::ssp}};

FileType getFileType(std::string_view fileName)
{
    auto nschar = fileName.find_first_not_of(" \n\t");
    if (nschar == std::string_view::npos) {
        return FileType::none;
    }
    auto loc = fileName.find_last_of('.');
    if (loc < fileName.size() - 5) {
        if (fileName[nschar] == '{') {
            return FileType::rawJson;
        }
        if (fileName[nschar] == '<') {
            return FileType::rawXML;
        }
        return FileType::unrecognized;
    }
    std::string_view type = fileName.substr(loc + 1);
    auto fnd = typeMap.find(type);
    if (fnd != typeMap.end()) {
        return fnd->second;
    }
    return FileType::unrecognized;
}

}  // namespace helicsfmi
