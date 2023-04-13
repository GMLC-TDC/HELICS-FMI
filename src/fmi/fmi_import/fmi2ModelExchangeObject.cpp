/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "fmiObjects.h"

fmi2ModelExchangeObject::fmi2ModelExchangeObject(
    const std::string& fmuname,
    fmi2Component cmp,
    std::shared_ptr<const FmiInfo> keyInfo,
    std::shared_ptr<const fmiCommonFunctions> comFunc,
    std::shared_ptr<const fmiModelExchangeFunctions> meFunc):
    fmi2Object(fmuname, cmp, keyInfo, comFunc),
    ModelExchangeFunctions(meFunc)
{
    numIndicators = info->getCounts(fmiVariableType::event);
    numStates = info->getCounts(fmiVariableType::state);
    if (numStates == 0) {
        hasTime = false;
    }
}

void fmi2ModelExchangeObject::setMode(FmuMode mode)
{
    fmi2Status ret = fmi2Error;
    switch (currentMode) {
        case FmuMode::INSTANTIATED:
        case FmuMode::INITIALIZATION:

            if (mode == FmuMode::CONTINUOUS_TIME) {
                fmi2Object::setMode(FmuMode::EVENT);
                if (numStates > 0) {
                    ret = ModelExchangeFunctions->fmi2EnterContinuousTimeMode(comp);
                } else {
                    ret = fmi2OK;
                }
            } else {
                fmi2Object::setMode(mode);
            }
            break;
        case FmuMode::CONTINUOUS_TIME:
            if (mode == FmuMode::EVENT) {
                ret = ModelExchangeFunctions->fmi2EnterEventMode(comp);
            }
            break;
        case FmuMode::EVENT:
            if (mode == FmuMode::EVENT) {
                ret = ModelExchangeFunctions->fmi2EnterEventMode(comp);
            } else if (mode == FmuMode::CONTINUOUS_TIME) {
                if (numStates > 0) {
                    ret = ModelExchangeFunctions->fmi2EnterContinuousTimeMode(comp);
                } else {
                    ret = fmi2OK;
                }
            }
            break;
        default:
            fmi2Object::setMode(mode);
            return;
    }

    if (ret == fmi2OK) {
        currentMode = mode;
    } else if (currentMode != mode) {
        handleNonOKReturnValues(ret);
    }
}

void fmi2ModelExchangeObject::newDiscreteStates(fmi2EventInfo* fmi2eventInfo)
{
    auto ret = ModelExchangeFunctions->fmi2NewDiscreteStates(comp, fmi2eventInfo);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}
void fmi2ModelExchangeObject::completedIntegratorStep(fmi2Boolean noSetFMUStatePriorToCurrentPoint,
                                                      fmi2Boolean* enterEventMode,
                                                      fmi2Boolean* terminatesSimulation)
{
    auto ret = ModelExchangeFunctions->fmi2CompletedIntegratorStep(comp,
                                                                   noSetFMUStatePriorToCurrentPoint,
                                                                   enterEventMode,
                                                                   terminatesSimulation);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}
void fmi2ModelExchangeObject::setTime(fmi2Real time)
{
    if (hasTime) {
        auto ret = ModelExchangeFunctions->fmi2SetTime(comp, time);
        if (ret != fmi2Status::fmi2OK) {
            handleNonOKReturnValues(ret);
        }
    }
}
void fmi2ModelExchangeObject::setStates(const fmi2Real x[])
{
    auto ret = ModelExchangeFunctions->fmi2SetContinuousStates(comp, x, numStates);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}
void fmi2ModelExchangeObject::getDerivatives(fmi2Real derivatives[]) const
{
    auto ret = ModelExchangeFunctions->fmi2GetDerivatives(comp, derivatives, numStates);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}
void fmi2ModelExchangeObject::getEventIndicators(fmi2Real eventIndicators[]) const
{
    auto ret = ModelExchangeFunctions->fmi2GetEventIndicators(comp, eventIndicators, numIndicators);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}
void fmi2ModelExchangeObject::getStates(fmi2Real x[]) const
{
    auto ret = ModelExchangeFunctions->fmi2GetContinuousStates(comp, x, numStates);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}
void fmi2ModelExchangeObject::getNominalsOfContinuousStates(fmi2Real x_nominal[]) const
{
    auto ret =
        ModelExchangeFunctions->fmi2GetNominalsOfContinuousStates(comp, x_nominal, numStates);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

std::vector<std::string> fmi2ModelExchangeObject::getStateNames() const
{
    return info->getVariableNames("state");
}
