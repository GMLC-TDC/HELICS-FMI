/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "fmiObjects.h"

fmi2Object::fmi2Object(const std::string& fmuname,
                       fmi2Component cmp,
                       std::shared_ptr<const fmiInfo> keyInfo,
                       std::shared_ptr<const fmiCommonFunctions> comFunc):
    comp(cmp),
    info(std::move(keyInfo)), commonFunctions(std::move(comFunc)), name(fmuname)
{
}

fmi2Object::~fmi2Object()
{
    commonFunctions->fmi2FreeInstance(comp);
}
void fmi2Object::setupExperiment(fmi2Boolean toleranceDefined,
                                 fmi2Real tolerance,
                                 fmi2Real startTime,
                                 fmi2Boolean stopTimeDefined,
                                 fmi2Real stopTime)
{
    auto ret = commonFunctions->fmi2SetupExperiment(
        comp, toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

fmuMode fmi2Object::getCurrentMode() const
{
    return currentMode;
}

void fmi2Object::setMode(fmuMode mode)
{
    if (mode == fmuMode::error) {
        currentMode = fmuMode::error;
        throw(fmiErrorException());
    }
    fmi2Status ret = fmi2Error;
    if (mode == fmuMode::terminated) {
        switch (currentMode) {
            case fmuMode::instantiatedMode:
                ret = commonFunctions->fmi2EnterInitializationMode(comp);
                handleNonOKReturnValues(ret);
                [[fallthrough]];
            case fmuMode::initializationMode:
                ret = commonFunctions->fmi2ExitInitializationMode(comp);
                handleNonOKReturnValues(ret);
                break;
            case fmuMode::error:
                return;
        }
        ret = commonFunctions->fmi2Terminate(comp);
        handleNonOKReturnValues(ret);
        currentMode = fmuMode::terminated;
        return;
    }

    switch (currentMode) {
        case fmuMode::instantiatedMode:
            switch (mode) {
                case fmuMode::instantiatedMode:
                case fmuMode::continuousTimeMode:
                case fmuMode::terminated:
                case fmuMode::error:
                    break;
                case fmuMode::initializationMode:
                    if (inputSize() == 0) {
                        setDefaultInputs();
                    }
                    if (outputSize() == 0) {
                        setDefaultOutputs();
                    }
                    ret = commonFunctions->fmi2EnterInitializationMode(comp);
                    break;
                case fmuMode::eventMode:
                case fmuMode::stepMode:
                    fmi2Object::setMode(
                        fmuMode::initializationMode);  // go into initialization first
                    ret = commonFunctions->fmi2ExitInitializationMode(comp);
            }
            break;
        case fmuMode::initializationMode:
            if ((mode == fmuMode::eventMode) || (mode == fmuMode::stepMode)) {
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
    currentMode = fmuMode::instantiatedMode;
    auto ret = commonFunctions->fmi2Reset(comp);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

template<>
std::string fmi2Object::get<std::string>(const std::string& param) const
{
    auto ref = info->getVariableInfo(param);
    if (ref.type._value != fmi_variable_type::string) {
        handleNonOKReturnValues(fmi2Status::fmi2Discard);
        return "";  // if we get here just return an empty string otherwise we threw an exception
    }
    fmi2String res;
    auto retval = commonFunctions->fmi2GetString(comp, &(ref.valueRef), 1, &res);
    if (retval != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(retval);
    }
    return std::string(res);  // this should copy the actual the string
}

void fmi2Object::get(const fmiVariableSet& vrset, fmi2Real value[]) const
{
    auto ret = commonFunctions->fmi2GetReal(comp, vrset.getValueRef(), vrset.getVRcount(), value);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}
void fmi2Object::get(const fmiVariableSet& vrset, fmi2Integer value[]) const
{
    auto ret =
        commonFunctions->fmi2GetInteger(comp, vrset.getValueRef(), vrset.getVRcount(), value);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

void fmi2Object::get(const fmiVariableSet& vrset, fmi2String value[]) const
{
    auto ret = commonFunctions->fmi2GetString(comp, vrset.getValueRef(), vrset.getVRcount(), value);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

void fmi2Object::set(const fmiVariableSet& vrset, fmi2Integer value[])
{
    auto ret =
        commonFunctions->fmi2GetInteger(comp, vrset.getValueRef(), vrset.getVRcount(), value);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

void fmi2Object::set(const fmiVariableSet& vrset, fmi2Real value[])
{
    auto retval =
        commonFunctions->fmi2SetReal(comp, vrset.getValueRef(), vrset.getVRcount(), value);
    if (retval != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(retval);
    }
}

void fmi2Object::set(const std::string& param, const char* val)
{
    auto ref = info->getVariableInfo(param);
    if (!(ref.type._value == fmi_variable_type::string)) {
        handleNonOKReturnValues(fmi2Status::fmi2Discard);
        return;
    }
    auto ret = commonFunctions->fmi2SetString(comp, &(ref.valueRef), 1, &val);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

void fmi2Object::set(const std::string& param, const std::string& val)
{
    auto ref = info->getVariableInfo(param);
    if (!(ref.type._value == fmi_variable_type::string)) {
        handleNonOKReturnValues(fmi2Status::fmi2Discard);
        return;
    }
    fmi2String val2 = val.c_str();
    auto ret = commonFunctions->fmi2SetString(comp, &(ref.valueRef), 1, &val2);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

void fmi2Object::setFlag(const std::string& param, bool val)
{
    auto ref = info->getVariableInfo(param);
    if (!(ref.type._value == fmi_variable_type::string)) {
        handleNonOKReturnValues(fmi2Status::fmi2Discard);
        return;
    }
    fmi2Boolean val2 = val ? fmi2True : fmi2False;
    auto ret = commonFunctions->fmi2SetBoolean(comp, &(ref.valueRef), 1, &val2);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
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
    size_t sz;
    auto ret = commonFunctions->fmi2SerializedFMUstateSize(comp, FMUstate, &sz);
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
    return sz;
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

fmi2Real fmi2Object::getPartialDerivative(int index_x, int index_y, double dx)
{
    double dy;
    commonFunctions->fmi2GetDirectionalDerivative(comp,
                                                  &(info->getVariableInfo(index_x).valueRef),
                                                  1,
                                                  &(info->getVariableInfo(index_y).valueRef),
                                                  1,
                                                  &dx,
                                                  &dy);
    return dy;
}

/** check if an output is real and actually is an output*/
bool isRealOutput(const variableInformation& vI)
{
    return ((vI.index >= 0) && (vI.type._value == fmi_variable_type::real) &&
            (fmi_causality::output == vI.causality._value || fmi_causality::local== vI.causality._value));
}

/** check if an input is real and actually is an input*/
bool isRealInput(const variableInformation& vI)
{
    return ((vI.index >= 0) && (vI.type._value == fmi_variable_type::real) &&
            (vI.causality._value == fmi_causality::input));
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
    activeOutputIndices.clear();
    for (const auto& outName : outNames) {
        const auto& vI = info->getVariableInfo(outName);
        if (isRealOutput(vI)) {
            activeOutputs.push(vI.valueRef);
            activeOutputIndices.push_back(vI.index);
        }
        // TODO(PT):: what to do if this condition is not valid?
    }
}

void fmi2Object::setOutputVariables(const std::vector<int>& outIndices)
{
    activeOutputIndices.clear();
    activeOutputs.clear();
    for (const auto& outIndex : outIndices) {
        const auto& vI = info->getVariableInfo(outIndex);
        if (isRealOutput(vI)) {
            activeOutputs.push(vI.valueRef);
            activeOutputIndices.push_back(vI.index);
        }
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
    activeInputIndices.clear();
    for (const auto& inName : inNames) {
        const auto& vI = info->getVariableInfo(inName);
        if (isRealInput(vI)) {
            activeInputs.push(vI.valueRef);
            activeInputIndices.push_back(vI.index);
        }
    }
}

void fmi2Object::setInputVariables(const std::vector<int>& inIndices)
{
    activeInputs.clear();
    activeInputIndices.clear();
    for (const auto& inIndex : inIndices) {
        const auto& vI = info->getVariableInfo(inIndex);
        if (isRealInput(vI)) {
            activeInputs.push(vI.valueRef);
            activeInputIndices.push_back(vI.index);
        }
    }
}


bool fmi2Object::addOutputVariable(const std::string& outputName)
{
    const auto& vI = info->getVariableInfo(outputName);
    if (isRealOutput(vI)) {
        activeOutputs.push(vI.valueRef);
        activeOutputIndices.push_back(vI.index);
        return true;
    }
    return false;
}
bool fmi2Object::addOutputVariable(int index)
{
    const auto& vI = info->getVariableInfo(index);
    if (isRealOutput(vI)) {
        activeOutputs.push(vI.valueRef);
        activeOutputIndices.push_back(vI.index);
        return true;
    }
    return false;
}
bool fmi2Object::addInputVariable(const std::string& inputName)
{
    const auto& vI = info->getVariableInfo(inputName);
    if (isRealInput(vI)) {
        activeInputs.push(vI.valueRef);
        activeInputIndices.push_back(vI.index);
        return true;
    }
    return false;
}
bool fmi2Object::addInputVariable(int index)
{
    const auto& vI = info->getVariableInfo(index);
    if (isRealInput(vI)) {
        activeInputs.push(vI.valueRef);
        activeInputIndices.push_back(vI.index);
        return true;
    }
    return false;
}

void fmi2Object::setDefaultInputs()
{
    setInputVariables(info->getVariableNames("input"));
}

void fmi2Object::setDefaultOutputs()
{
    setOutputVariables(info->getVariableNames("output"));
}

void fmi2Object::setInputs(const fmi2Real inputs[])
{
    if (inputs != nullptr) {
        auto ret = commonFunctions->fmi2SetReal(comp,
                                                activeInputs.getValueRef(),
                                                activeInputs.getVRcount(),
                                                inputs);
        if (ret != fmi2Status::fmi2OK) {
            handleNonOKReturnValues(ret);
        }
    }
}

void fmi2Object::getCurrentInputs(fmi2Real inputs[])
{
    fmi2Status ret = fmi2Status::fmi2Warning;
    if (inputs != nullptr) {
        ret = commonFunctions->fmi2GetReal(comp,
                                           activeInputs.getValueRef(),
                                           activeInputs.getVRcount(),
                                           inputs);
    }
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

void fmi2Object::getOutputs(fmi2Real outputs[]) const
{
    fmi2Status ret = fmi2Status::fmi2Warning;
    if (outputs != nullptr) {
        ret = commonFunctions->fmi2GetReal(comp,
                                           activeOutputs.getValueRef(),
                                           activeOutputs.getVRcount(),
                                           outputs);
    }
    if (ret != fmi2Status::fmi2OK) {
        handleNonOKReturnValues(ret);
    }
}

fmi2Real fmi2Object::getOutput(size_t outNum) const
{
    fmi2Real out = -1e48;
    if (outNum < activeOutputs.getVRcount()) {
        commonFunctions->fmi2GetReal(comp, activeOutputs.getValueRef() + outNum, 1, &out);
    }
    return out;
}

fmiVariableSet fmi2Object::getVariableSet(const std::string& variable) const
{
    return info->getVariableSet(variable);
}

fmiVariableSet fmi2Object::getVariableSet(int index) const
{
    return info->getVariableSet(index);
}

std::vector<std::string> fmi2Object::getOutputNames() const
{
    std::vector<std::string> oVec;
    if (activeOutputs.getVRcount() == 0) {
        oVec = info->getVariableNames("output");
    } else {
        oVec.reserve(activeOutputs.getVRcount());
        for (const auto& os : activeOutputIndices) {
            oVec.push_back(info->getVariableInfo(os).name);
        }
    }

    return oVec;
}

std::vector<std::string> fmi2Object::getInputNames() const
{
    std::vector<std::string> oVec;
    if (activeInputs.getVRcount() == 0) {
        oVec = info->getVariableNames("input");
    } else {
        oVec.reserve(activeInputs.getVRcount());
        for (const auto& os : activeInputIndices) {
            oVec.push_back(info->getVariableInfo(os).name);
        }
    }
    return oVec;
}

bool fmi2Object::isParameter(const std::string& param, fmi_variable_type type)
{
    const auto& vi = info->getVariableInfo(param);
    if (vi.index >= 0) {
        if ((vi.causality._value == fmi_causality::parameter) ||
            (vi.causality._value == fmi_causality::input)) {
            if (vi.type == type) {
                return true;
            }
            if (type._value == fmi_variable_type::numeric) {
                if (vi.type._value != fmi_variable_type::string) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool fmi2Object::isVariable(const std::string& var, fmi_variable_type type)
{
    const auto& vi = info->getVariableInfo(var);
    if (vi.index >= 0) {
        if (vi.type == type) {
            return true;
        }
        if (type._value == fmi_variable_type::numeric) {
            if (vi.type._value != fmi_variable_type::string) {
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
