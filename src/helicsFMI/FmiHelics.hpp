/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "fmi/fmi_import/fmiImport.h"
#include "fmi/fmi_import/fmiObjects.h"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/application_api/helicsPrimaryTypes.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace helicsfmi {
/** get the corresponding helics type name to an FMI variable type*/
std::string_view getHelicsTypeString(fmi_variable_type type);
/** get the corresponding helics data type to an FMI variable type*/
helics::DataType getHelicsType(fmi_variable_type type);

/** publish output data to a helics publication*/
void publishOutput(helics::Publication& pub, fmi2Object* cs, int index);
/** direct helics input data to an input*/
void grabInput(helics::Input& inp, fmi2Object* cs, int index);
/** set the default values of a fmi input to be the helics default so there isn't value problems*/
void setDefault(helics::Input& inp, fmi2Object* cs, int index);

}  // namespace helicsfmi
