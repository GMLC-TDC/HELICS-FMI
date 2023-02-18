/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef FMI_ENUM_DEFINITIONS_H_
#define FMI_ENUM_DEFINITIONS_H_
#pragma once

#include <string>
#define BETTER_ENUMS_NO_CONSTEXPR
#include "better_enums/enum.h"

/** enumeration of the known fmu types*/
BETTER_ENUM(fmu_type, int, unknown, modelExchange, cosimulation)

BETTER_ENUM(fmi_variability, int, continuous = 0, constant, fixed, tunable, discrete, unknown)

BETTER_ENUM(fmi_causality,
            int,
            local,
            parameter,
            calculatedParameter,
            input,
            output,
            independent,
            unknown,
            any)

/** enumeration of fmi variable types*/
enum class fmiVariableType {
    local,
    any,
    input,
    output,
    parameter,
    calculatedParameter,
    independent,
    unknown,
    derivative,
    state,
    units,
    nonZero,
    event,
    meObject,
    csObject
};

BETTER_ENUM(fmi_variable_type,
            int,
            real = 0,
            integer,
            boolean,
            string,
            enumeration,
            unknown,
            numeric)

BETTER_ENUM(fmi_dependency_type,
            int,
            dependent = 0,
            constant,
            fixed,
            tunable,
            discrete,
            independent,
            unknown)

enum fmuCapabilityFlags : int {
    modelExchangeCapable,
    coSimulationCapable,
    canGetAndSetFMUstate,
    providesDirectionalDerivative,
    canSerializeFMUstate,
    needsExecutionTool,
    completedIntegratorStepNotNeeded,
    canHandleVariableCommunicationStepSize,
    canInterpolateInputs,
    canRunAsynchronously,
    canBeInstantiatedOnlyOncePerProcess,
    canNotUseMemoryManagementFunctions,

};

/** enumeration of platform support types*/
enum fmuPlatformSupport : int {
    win32,
    win64,
    macos,
    linux,
};

#endif
