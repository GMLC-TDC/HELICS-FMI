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
void publishOutput(helics::Publication& pub, fmi2Object* fmiObj, std::size_t index);
/** direct helics input data to an input*/
void grabInput(helics::Input& inp, fmi2Object* fmiObj, std::size_t index);
/** set the default values of a fmi input to be the helics default so there isn't value problems*/
void setDefault(helics::Input& inp, fmi2Object* fmiObj, std::size_t index);

/** generate a helics log level from an FMI category description*/
int fmiCategory2HelicsLogLevel(std::string_view category);

}  // namespace helicsfmi
