/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "fmiObjects.h"

#include <cstdlib>

fmi2Object::fmi2Object(const std::string& fmuname,
                       fmi2Component cmp,
                       std::shared_ptr<const FmiInfo> keyInfo,
                       std::shared_ptr<const fmiCommonFunctions> comFunc):
    comp(cmp),
    info(std::move(keyInfo)), commonFunctions(std::move(comFunc)), name(fmuname)
{
}

fmi2Object::~fmi2Object()
{
    if (!noFree && commonFunctions->fmi2FreeInstance) {
        commonFunctions->fmi2FreeInstance(comp);
    }
}

void fmi2Object::setupExperiment()
{
    const auto& exp = info->getExperiment();
    const fmi2Boolean toleranceDefined = (exp.tolerance > 0.0) ? fmi2True : fmi2False;
    const fmi2Boolean stopTimeDefined = (exp.stopTime > 0.0) ? fmi2True : fmi2False;
    auto ret = commonFunctions->fmi2SetupExperiment(
        comp, toleranceDefined, exp.tolerance, exp.startTime, stopTimeDefined, exp.stopTime);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

void fmi2Object::setupExperiment(bool toleranceDefined,
                                 fmi2Real tolerance,
                                 fmi2Real startTime,
                                 bool stopTimeDefined,
                                 fmi2Real stopTime)
{
    auto ret = commonFunctions->fmi2SetupExperiment(comp,
                                                    toleranceDefined ? fmi2True : fmi2False,
                                                    tolerance,
                                                    startTime,
                                                    stopTimeDefined ? fmi2True : fmi2False,
                                                    stopTime);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

FmuMode fmi2Object::getCurrentMode() const
{
    return currentMode;
}

void fmi2Object::setMode(FmuMode mode)
{
    if (mode == FmuMode::ERROR) {
        currentMode = FmuMode::ERROR;
        throw(fmiErrorException());
    }
    fmi2Status ret = fmi2Error;
    if (mode == FmuMode::TERMINATED) {
        switch (currentMode) {
            case FmuMode::INSTANTIATED:
                ret = commonFunctions->fmi2EnterInitializationMode(comp);
                handleNonOKReturnValues(ret);
                [[fallthrough]];
            case FmuMode::INITIALIZATION:
                ret = commonFunctions->fmi2ExitInitializationMode(comp);
                handleNonOKReturnValues(ret);
                break;
            case FmuMode::ERROR:
                return;
        }
        ret = commonFunctions->fmi2Terminate(comp);
        handleNonOKReturnValues(ret);
        currentMode = FmuMode::TERMINATED;
        return;
    }

    switch (currentMode) {
        case FmuMode::INSTANTIATED:
            switch (mode) {
                case FmuMode::INSTANTIATED:
                case FmuMode::CONTINUOUS_TIME:
                case FmuMode::TERMINATED:
                case FmuMode::ERROR:
                    break;
                case FmuMode::INITIALIZATION:
                    if (inputSize() == 0) {
                        setDefaultInputs();
                    }
                    if (outputSize() == 0) {
                        setDefaultOutputs();
                    }
                    ret = commonFunctions->fmi2EnterInitializationMode(comp);
                    break;
                case FmuMode::EVENT:
                case FmuMode::STEP:
                    fmi2Object::setMode(FmuMode::INITIALIZATION);  // go into initialization first
                    ret = commonFunctions->fmi2ExitInitializationMode(comp);
            }
            break;
        case FmuMode::INITIALIZATION:
            if ((mode == FmuMode::EVENT) || (mode == FmuMode::STEP)) {
                ret = commonFunctions->fmi2ExitInitializationMode(comp);
            }

            break;
        default:
            break;
    }

    if (ret == fmi2OK) {
        currentMode = mode;
    } else if (currentMode == mode) {
        ret = fmi2OK;
    }
    handleNonOKReturnValues(ret);
}

void fmi2Object::reset()
{
    currentMode = FmuMode::INSTANTIATED;
    auto ret = commonFunctions->fmi2Reset(comp);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

void fmi2Object::get(const FmiVariableSet& vrset, fmi2Real value[]) const
{
    auto ret = commonFunctions->fmi2GetReal(comp, vrset.getValueRef(), vrset.getVRcount(), value);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}
void fmi2Object::get(const FmiVariableSet& vrset, fmi2Integer value[]) const
{
    auto ret =
        commonFunctions->fmi2GetInteger(comp, vrset.getValueRef(), vrset.getVRcount(), value);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

void fmi2Object::get(const FmiVariableSet& vrset, fmi2String value[]) const
{
    auto ret = commonFunctions->fmi2GetString(comp, vrset.getValueRef(), vrset.getVRcount(), value);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

void fmi2Object::set(const FmiVariableSet& vrset, fmi2Integer value[])
{
    auto ret =
        commonFunctions->fmi2GetInteger(comp, vrset.getValueRef(), vrset.getVRcount(), value);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

void fmi2Object::set(const FmiVariableSet& vrset, fmi2Real value[])
{
    auto retval =
        commonFunctions->fmi2SetReal(comp, vrset.getValueRef(), vrset.getVRcount(), value);
    if (retval != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(retval);
    }
}

void fmi2Object::set(const FmiVariable& param, const char* val)
{
    if (param.type._value == fmi_variable_type::string) {
        auto ret = commonFunctions->fmi2SetString(comp, &(param.vRef), 1, &val);
        if (ret != fmi2Status::fmi2OK) {
            handleNonOKReturnValues(ret);
        }
    } else {
        char* pEnd;
        const double vdouble = std::strtod(val, &pEnd);
        if (pEnd != val) {
            set(param, vdouble);
        }
    }
}

void fmi2Object::set(const FmiVariable& param, const std::string& val)
{
    set(param, val.c_str());
}

bool fmi2Object::setFlag(const std::string& param, bool val)
{
    if (param == "exception_on_discard") {
        exceptionOnDiscard = val;
        return true;
    }
    if (param == "exception_on_warning") {
        exceptionOnWarning = val;
        return true;
    }
    if (param == "no_free") {
        noFree = val;
        return true;
    }
    const auto& ref = info->getVariableInfo(param);
    fmi2Status ret{fmi2Status::fmi2Discard};
    switch (ref.type._value) {
        case fmi_variable_type::boolean: {
            const fmi2Boolean val2 = val ? fmi2True : fmi2False;
            ret = commonFunctions->fmi2SetBoolean(comp, &(ref.valueRef), 1, &val2);
        } break;
        case fmi_variable_type::integer: {
            const int val2 = val ? 1 : 0;
            ret = commonFunctions->fmi2SetInteger(comp, &(ref.valueRef), 1, &val2);
        } break;
        default:
            break;
    }

    return (ret == fmi2Status::fmi2OK);
}

void fmi2Object::getFMUState(fmi2FMUstate* FMUstate)
{
    auto ret = commonFunctions->fmi2GetFMUstate(comp, FMUstate);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}
void fmi2Object::setFMUState(fmi2FMUstate FMUstate)
{
    auto ret = commonFunctions->fmi2SetFMUstate(comp, FMUstate);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

size_t fmi2Object::serializedStateSize(fmi2FMUstate FMUstate)
{
    size_t size;
    auto ret = commonFunctions->fmi2SerializedFMUstateSize(comp, FMUstate, &size);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
    return size;
}
void fmi2Object::serializeState(fmi2FMUstate FMUstate, fmi2Byte serializedState[], size_t size)
{
    auto ret = commonFunctions->fmi2SerializeFMUstate(comp, FMUstate, serializedState, size);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}
void fmi2Object::deSerializeState(const fmi2Byte serializedState[],
                                  size_t size,
                                  fmi2FMUstate* FMUstate)
{
    auto ret = commonFunctions->fmi2DeSerializeFMUstate(comp, serializedState, size, FMUstate);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}
void fmi2Object::getDirectionalDerivative(const fmi2ValueReference vUnknown_ref[],
                                          size_t nUnknown,
                                          const fmi2ValueReference vKnown_ref[],
                                          size_t unknown,
                                          const fmi2Real dvKnown[],
                                          fmi2Real dvUnknown[])
{
    auto ret = commonFunctions->fmi2GetDirectionalDerivative(
        comp, vUnknown_ref, nUnknown, vKnown_ref, unknown, dvKnown, dvUnknown);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

fmi2Real fmi2Object::getPartialDerivative(int index_x, int index_y, double deltax)
{
    double deltay;
    commonFunctions->fmi2GetDirectionalDerivative(comp,
                                                  &(info->getVariableInfo(index_x).valueRef),
                                                  1,
                                                  &(info->getVariableInfo(index_y).valueRef),
                                                  1,
                                                  &deltax,
                                                  &deltay);
    return deltay;
}

/** check if an output is real and actually is an output*/
static bool isInput(const VariableInformation& vInfo)
{
    return ((vInfo.index >= 0) && (vInfo.causality._value == fmi_causality::input));
}

/** check if an output is real and actually is an output*/
static bool isOutput(const VariableInformation& vInfo)
{
    return ((vInfo.index >= 0) &&
            (fmi_causality::output == vInfo.causality._value ||
             fmi_causality::local == vInfo.causality._value));
}

/** check if an output is real and actually is an output*/
/*static bool isRealOutput(const variableInformation& vInfo)
{
    return ((vInfo.index >= 0) && (vInfo.type._value == fmi_variable_type::real) &&
            (fmi_causality::output == vInfo.causality._value ||
             fmi_causality::local == vInfo.causality._value));
}
*/

/** check if an input is real and actually is an input*/
static bool isRealInput(const VariableInformation& vInfo)
{
    return ((vInfo.index >= 0) && (vInfo.type._value == fmi_variable_type::real) &&
            (vInfo.causality._value == fmi_causality::input));
}

void fmi2Object::setOutputVariables(const std::vector<std::string>& outNames)
{
    if (outNames.size() == 1) {
        if (outNames[0] == "all") {
            setDefaultOutputs();
            return;
        }
    }
    activeOutputs.clear();
    for (const auto& outName : outNames) {
        addOutputVariable(outName);
    }
}

void fmi2Object::setOutputVariables(const std::vector<int>& outIndices)
{
    activeOutputs.clear();
    for (const auto& outIndex : outIndices) {
        addOutputVariable(outIndex);
    }
}

void fmi2Object::setInputVariables(const std::vector<std::string>& inNames)
{
    if (inNames.size() == 1) {
        if (inNames[0] == "all") {
            setDefaultInputs();
            return;
        }
    }
    activeInputs.clear();
    for (const auto& inName : inNames) {
        addInputVariable(inName);
    }
}

void fmi2Object::setInputVariables(const std::vector<int>& inIndices)
{
    activeInputs.clear();
    for (const auto& inIndex : inIndices) {
        addInputVariable(inIndex);
    }
}

static const FmiVariable emptyVariable{static_cast<fmi2ValueReference>(-1),
                                       fmi_variable_type::unknown,
                                       -1};

const FmiVariable& fmi2Object::addOutputVariable(const std::string& outputName)
{
    const auto& vInfo = info->getVariableInfo(outputName);
    if (isOutput(vInfo)) {
        return activeOutputs.emplace_back(vInfo.valueRef, vInfo.type, vInfo.index);
    }
    return emptyVariable;
}
const FmiVariable& fmi2Object::addOutputVariable(int index)
{
    const auto& vInfo = info->getVariableInfo(index);
    if (isOutput(vInfo)) {
        return activeOutputs.emplace_back(vInfo.valueRef, vInfo.type, vInfo.index);
    }
    return emptyVariable;
}
const FmiVariable& fmi2Object::addInputVariable(const std::string& inputName)
{
    const auto& vInfo = info->getVariableInfo(inputName);
    if (isInput(vInfo)) {
        return activeInputs.emplace_back(vInfo.valueRef, vInfo.type, vInfo.index);
    }
    return emptyVariable;
}
const FmiVariable& fmi2Object::addInputVariable(int index)
{
    const auto& vInfo = info->getVariableInfo(index);
    if (isRealInput(vInfo)) {
        return activeInputs.emplace_back(vInfo.valueRef, vInfo.type, vInfo.index);
    }
    return emptyVariable;
}

void fmi2Object::setDefaultInputs()
{
    setInputVariables(info->getVariableNames("input"));
}

void fmi2Object::setDefaultOutputs()
{
    setOutputVariables(info->getVariableNames("output"));
}

FmiVariableSet fmi2Object::getVariableSet(const std::string& variable) const
{
    return info->getVariableSet(variable);
}

FmiVariableSet fmi2Object::getVariableSet(int index) const
{
    return info->getVariableSet(index);
}

const FmiVariable& fmi2Object::getInput(int index) const
{
    return activeInputs.at(index);
}
const FmiVariable& fmi2Object::getOutput(int index) const
{
    return activeOutputs.at(index);
}

std::vector<std::string> fmi2Object::getOutputNames() const
{
    std::vector<std::string> oVec;
    if (activeOutputs.empty()) {
        oVec = info->getVariableNames("output");
    } else {
        oVec.reserve(activeOutputs.size());
        for (const auto& output : activeOutputs) {
            oVec.push_back(info->getVariableInfo(output.index).name);
        }
    }

    return oVec;
}

std::vector<std::string> fmi2Object::getInputNames() const
{
    std::vector<std::string> oVec;
    if (activeInputs.empty()) {
        oVec = info->getVariableNames("input");
    } else {
        oVec.reserve(activeInputs.size());
        for (const auto& input : activeInputs) {
            oVec.push_back(info->getVariableInfo(input.index).name);
        }
    }
    return oVec;
}

bool fmi2Object::isParameter(const std::string& param, fmi_variable_type type)
{
    const auto& varInfo = info->getVariableInfo(param);
    if (varInfo.index >= 0) {
        if ((varInfo.causality._value == fmi_causality::parameter) ||
            (varInfo.causality._value == fmi_causality::input)) {
            if (varInfo.type == type) {
                return true;
            }
            if (type._value == fmi_variable_type::numeric) {
                if (varInfo.type._value != fmi_variable_type::string) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool fmi2Object::isVariable(const std::string& var, fmi_variable_type type)
{
    const auto& varInfo = info->getVariableInfo(var);
    if (varInfo.index >= 0) {
        if (varInfo.type == type) {
            return true;
        }
        if (type._value == fmi_variable_type::numeric) {
            if (varInfo.type._value != fmi_variable_type::string) {
                return true;
            }
        }
    }

    return false;
}

void fmi2Object::handleNonOKReturnValues(fmi2Status retval) const
{
    switch (retval) {
        case fmi2Status::fmi2OK:
        case fmi2Status::fmi2Pending:
            return;
        case fmi2Status::fmi2Discard:
            if (exceptionOnDiscard) {
                throw(fmiDiscardException());
            }
            break;
        case fmi2Status::fmi2Warning:
            if (exceptionOnWarning) {
                throw(fmiWarningException());
            }
            break;
        case fmi2Status::fmi2Error:
            throw(fmiErrorException());
        case fmi2Status::fmi2Fatal:
            throw(fmiFatalException());
        default:
            throw(fmiException());
    }
}
