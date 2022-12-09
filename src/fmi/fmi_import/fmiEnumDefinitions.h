/*
 * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
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
