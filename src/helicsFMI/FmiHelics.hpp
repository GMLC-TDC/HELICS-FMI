/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "fmi/fmi_import/fmiImport.h"
#include "fmi/fmi_import/fmiObjects.h"
#include "helics/application_api/HelicsPrimaryTypes.hpp"
#include "helics/application_api/ValueFederate.hpp"

#include <exception>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace helicsfmi {

#define LOG_ERROR(message) logMessage(HELICS_LOG_LEVEL_ERROR, message)
#define LOG_WARNING(message) logMessage(HELICS_LOG_LEVEL_WARNING, message)

#define LOG_SUMMARY(message)                                                                       \
    if (logLevel >= HELICS_LOG_LEVEL_SUMMARY) {                                                    \
        logMessage(HELICS_LOG_LEVEL_SUMMARY, message);                                             \
    }

#define LOG_CONNECTIONS(message)                                                                   \
    if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {                                                \
        logMessage(HELICS_LOG_LEVEL_CONNECTIONS, message);                                         \
    }

#define LOG_INTERFACES(message)                                                                    \
    if (logLevel >= HELICS_LOG_LEVEL_INTERFACES) {                                                 \
        logMessage(HELICS_LOG_LEVEL_INTERFACES, message);                                          \
    }

#define LOG_TIMING(message)                                                                        \
    if (logLevel >= HELICS_LOG_LEVEL_TIMING) {                                                     \
        logMessage(HELICS_LOG_LEVEL_TIMING, message);                                              \
    }
#define LOG_DATA_MESSAGES(message)                                                                 \
    if (logLevel >= HELICS_LOG_LEVEL_DATA) {                                                       \
        logMessage(HELICS_LOG_LEVEL_DATA, message);                                                \
    }
#define LOG_DEBUG_MESSAGES(message)                                                                \
    if (logLevel >= HELICS_LOG_LEVEL_DEBUG) {                                                      \
        logMessage(HELICS_LOG_LEVEL_DEBUG, message);                                               \
    }

#define LOG_TRACE(message)                                                                         \
    if (logLevel >= HELICS_LOG_LEVEL_TRACE) {                                                      \
        logMessage(HELICS_LOG_LEVEL_TRACE, message);                                               \
    }

// All errors derive from this one
class Error: public std::runtime_error {
    int actual_exit_code;
    std::string error_name{"Error"};

  public:
    [[nodiscard]] int get_exit_code() const { return actual_exit_code; }

    [[nodiscard]] std::string get_name() const { return error_name; }

    Error(std::string name, std::string msg, int exit_code = -101):
        runtime_error(msg), actual_exit_code(exit_code), error_name(std::move(name))
    {
    }
};

/** get the corresponding helics type name to an FMI variable type*/
std::string_view getHelicsTypeString(fmi_variable_type type);
/** get the corresponding helics data type to an FMI variable type*/
helics::DataType getHelicsType(fmi_variable_type type);

/** publish output data to a helics publication*/
void publishOutput(helics::Publication& pub,
                   fmi2Object* fmiObj,
                   std::size_t index,
                   bool logValues = false);
/** direct helics input data to an input*/
void grabInput(helics::Input& inp, fmi2Object* fmiObj, std::size_t index, bool logValues = false);
/** set the default values of a fmi input to be the helics default so there isn't value problems*/
void setDefault(helics::Input& inp, fmi2Object* fmiObj, std::size_t index);

/** generate a helics log level from an FMI category description*/
int fmiCategory2HelicsLogLevel(std::string_view category);

enum class FileType :std::int32_t
{
    none=0,
    unrecognized,
    fmu,
    json,
    rawJson,
    xml,
    rawXML,
    toml,
    ssp
};

/** get the filetype of a file for later parsing*/
FileType getFileType(std::string_view fileName);
}  // namespace helicsfmi
