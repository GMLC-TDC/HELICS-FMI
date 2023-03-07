/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "FmiHelics.hpp"
#include <string>

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

void publishOutput(helics::Publication& pub, fmi2Object* cs, int index)
{
    const auto& var = cs->getOutput(index);
    switch (var.type) {
        case fmi_variable_type::boolean: {
            auto val = cs->get<fmi2Boolean>(var);
            pub.publish((val == fmi2False) ? false : true);
        } break;
        case fmi_variable_type::integer:
        case fmi_variable_type::enumeration: {
            auto val = cs->get<std::int64_t>(var);
            pub.publish(val);
        } break;
        case fmi_variable_type::real:
        case fmi_variable_type::numeric: {
            auto val = cs->get<double>(var);
            pub.publish(val);
        } break;
        case fmi_variable_type::string:
        default: {
            auto val = cs->get<std::string_view>(var);
            pub.publish(val);
        } break;
    }
}

void grabInput(helics::Input& inp, fmi2Object* cs, int index)
{
    const auto& var = cs->getInput(index);
    switch (var.type) {
        case fmi_variable_type::boolean: {
            auto val = inp.getValue<fmi2Boolean>();
            cs->set(var, val);
        } break;
        case fmi_variable_type::integer:
        case fmi_variable_type::enumeration: {
            auto val = inp.getValue<fmi2Integer>();
            cs->set(var, val);
        } break;
        case fmi_variable_type::real:
        case fmi_variable_type::numeric: {
            auto val = inp.getValue<fmi2Real>();
            cs->set(var, val);
        } break;
        case fmi_variable_type::string:
        default: {
            auto val = inp.getValue<std::string>();
            cs->set(var, val);
        } break;
    }
}

void setDefault(helics::Input& inp, fmi2Object* cs, int index)
{
    const auto& var = cs->getInput(index);
    switch (var.type) {
        case fmi_variable_type::boolean: {
            auto val = cs->get<bool>(var);
            inp.setDefault(val);
        } break;
        case fmi_variable_type::integer:
        case fmi_variable_type::enumeration: {
            auto val = cs->get<std::int64_t>(var);
            inp.setDefault(val);
        } break;
        case fmi_variable_type::real:
        case fmi_variable_type::numeric: {
            auto val = cs->get<double>(var);
            inp.setDefault(val);
        } break;
        case fmi_variable_type::string:
        default: {
            auto val = cs->get<std::string_view>(var);
            inp.setDefault(val);
        } break;
    }
}
}  // namespace helicsfmi
