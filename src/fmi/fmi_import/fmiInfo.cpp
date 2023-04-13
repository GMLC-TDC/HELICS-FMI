/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "fmiInfo.h"

#include "formatInterpreters/tinyxml2ReaderElement.h"
#include "gmlc/utilities/stringConversion.h"
#include "units/units.hpp"

#include <utility>

using gmlc::utilities::convertToLowerCase;

FmiInfo::FmiInfo() {}

FmiInfo::FmiInfo(const std::string& fileName)
{
    loadFile(fileName);
}

int FmiInfo::loadFile(const std::string& fileName)
{
    std::shared_ptr<readerElement> reader = std::make_shared<tinyxml2ReaderElement>(fileName);
    if (!reader->isValid()) {
        return (-1);
    }
    headerInfo["xmlfile"] = fileName;
    headerInfo["xmlfilename"] = fileName;

    loadFmiHeader(reader);
    loadUnitInformation(reader);
    loadVariables(reader);
    loadStructure(reader);
    return 0;
}

static const std::map<std::string, int> flagMap{
    {"modelExchangeCapable", modelExchangeCapable},
    {"coSimulationCapable", coSimulationCapable},
    {"canGetAndSetFMUstate", canGetAndSetFMUstate},
    {"providesDirectionalDerivative", providesDirectionalDerivative},
    {"canSerializeFMUstate", canSerializeFMUstate},
    {"canGetAndSetFMUstate", canGetAndSetFMUstate},
    {"needsExecutionTool", needsExecutionTool},
    {"completedIntegratorStepNotNeeded", completedIntegratorStepNotNeeded},
    {"canHandleVariableCommunicationStepSize", canHandleVariableCommunicationStepSize},
    {"canInterpolateInputs", canInterpolateInputs},
    {"canRunAsynchronously", canRunAsynchronously},
    {"canBeInstantiatedOnlyOncePerProcess", canBeInstantiatedOnlyOncePerProcess},
    {"canNotUseMemoryManagementFunctions", canNotUseMemoryManagementFunctions}};

void loadFmuFlag(std::bitset<32>& capabilities, const readerAttribute& att)
{
    auto fnd = flagMap.find(att.getName());
    if (fnd != flagMap.end()) {
        capabilities.set(fnd->second, (att.getText() == "true"));
    }
}

bool FmiInfo::checkFlag(fmuCapabilityFlags flag) const
{
    return capabilities[flag];
}

int FmiInfo::getCounts(fmiVariableType countType) const
{
    std::size_t cnt{static_cast<std::size_t>(-1)};
    switch (countType) {
        case fmiVariableType::meObject:
            cnt = checkFlag(modelExchangeCapable) ? 1 : 0;
            break;
        case fmiVariableType::csObject:
            cnt = checkFlag(coSimulationCapable) ? 1 : 0;
            break;
        case fmiVariableType::local:
            cnt = local.size();
            break;
        case fmiVariableType::any:
            cnt = variables.size();
            break;
        case fmiVariableType::input:
            cnt = inputs.size();
            break;
        case fmiVariableType::output:
            cnt = outputs.size();
            break;
        case fmiVariableType::parameter:
        case fmiVariableType::calculatedParameter:
            cnt = parameters.size();
            break;

        case fmiVariableType::independent:
        case fmiVariableType::unknown:
            cnt = initUnknown.size();
            break;
        case fmiVariableType::derivative:
        case fmiVariableType::state:
            cnt = states.size();
            break;
        case fmiVariableType::units:
            cnt = units.size();
            break;
        case fmiVariableType::nonZero:
            cnt = derivDep.size();
            break;
        case fmiVariableType::event:
            cnt = eventIndicators;
            break;
        default:
            break;
    }

    if (cnt == static_cast<std::size_t>(-1)) {
        return (-1);
    }
    return static_cast<int>(cnt);
}

static const std::string emptyString{};

const std::string& FmiInfo::getString(const std::string& field) const
{
    auto fnd = headerInfo.find(field);
    if (fnd == headerInfo.end()) {
        return emptyString;
    }
    return fnd->second;
}

double FmiInfo::getReal(const std::string& field) const
{
    auto fld = convertToLowerCase(field);
    if (fld == "version") {
        return fmiVersion;
    }
    if ((fld == "start") || (fld == "starttime")) {
        return defaultExpirement.startTime;
    }
    if ((fld == "stop") || (fld == "stoptime")) {
        return defaultExpirement.stopTime;
    }
    if ((fld == "step") || (fld == "stepsize")) {
        return defaultExpirement.stepSize;
    }
    if (fld == "tolerance") {
        return defaultExpirement.tolerance;
    }
    return (-1.0e-48);
}

static const VariableInformation emptyVI{};
static const FmiVariableSet emptyVset;

const VariableInformation& FmiInfo::getVariableInfo(const std::string& variableName) const
{
    auto variablefind = variableLookup.find(variableName);
    if (variablefind == variableLookup.end()) {
        return emptyVI;
    }
    return variables[variablefind->second];
}

const VariableInformation& FmiInfo::getVariableInfo(unsigned int index) const
{
    if (index >= variables.size()) {
        return emptyVI;
    }
    return variables[index];
}

FmiVariableSet FmiInfo::getReferenceSet(const std::vector<std::string>& variableList) const
{
    FmiVariableSet vset;
    for (const auto& vname : variableList) {
        const auto& vref = getVariableInfo(vname);
        if (vref.valueRef > 0) {
            vset.push(vref.valueRef);
        }
    }
    return vset;
}

FmiVariableSet FmiInfo::getVariableSet(const std::string& variable) const
{
    const auto& vref = getVariableInfo(variable);
    if (vref.valueRef > 0) {
        return {vref.valueRef};
    }
    return emptyVset;
}

FmiVariableSet FmiInfo::getVariableSet(unsigned int index) const
{
    if (index >= variables.size()) {
        return emptyVset;
    }
    return {variables[index].valueRef};
}

FmiVariableSet FmiInfo::getOutputReference() const
{
    FmiVariableSet vset;
    vset.reserve(outputs.size());
    for (const auto& outInd : outputs) {
        vset.push(variables[outInd].valueRef);
    }
    return vset;
}

FmiVariableSet FmiInfo::getInputReference() const
{
    FmiVariableSet vset;
    vset.reserve(inputs.size());
    for (const auto& inInd : inputs) {
        vset.push(variables[inInd].valueRef);
    }
    return vset;
}

std::vector<std::string> FmiInfo::getVariableNames(const std::string& type) const
{
    std::vector<std::string> vnames;
    if (type == "state") {
        for (const auto& varIndex : states) {
            vnames.push_back(variables[varIndex].name);
        }
    } else {
        auto caus = fmi_causality::_from_string(type.c_str());
        for (const auto& var : variables) {
            if ((caus._value == fmi_causality::any) || (var.causality == caus)) {
                vnames.push_back(var.name);
            }
        }
    }

    return vnames;
}

static const std::vector<int> emptyVec;

const std::vector<int>& FmiInfo::getVariableIndices(const std::string& type) const
{
    if (type == "state") {
        return states;
    }
    if (type == "deriv") {
        return deriv;
    }
    if (type == "parameter") {
        return parameters;
    }
    if ((type == "inputs") || (type == "input")) {
        return inputs;
    }
    if ((type == "outputs") || (type == "output")) {
        return outputs;
    }
    if (type == "local") {
        return local;
    }
    if (type == "unknown") {
        return initUnknown;
    }
    return emptyVec;
}

/** get the variable indices of the derivative dependencies*/
const std::vector<std::pair<index_t, int>>& FmiInfo::getDerivDependencies(int variableIndex) const
{
    return derivDep.getSet(variableIndex);
}
const std::vector<std::pair<index_t, int>>& FmiInfo::getOutputDependencies(int variableIndex) const
{
    return outputDep.getSet(variableIndex);
}
const std::vector<std::pair<index_t, int>>& FmiInfo::getUnknownDependencies(int variableIndex) const
{
    return unknownDep.getSet(variableIndex);
}

void FmiInfo::loadFmiHeader(std::shared_ptr<readerElement>& reader)
{
    auto att = reader->getFirstAttribute();
    while (att.isValid()) {
        headerInfo.emplace(att.getName(), att.getText());
        auto lcname = convertToLowerCase(att.getName());
        if (lcname != att.getName()) {
            headerInfo.emplace(convertToLowerCase(att.getName()), att.getText());
        }
        att = reader->getNextAttribute();
    }
    // get the fmi version information
    auto versionFind = headerInfo.find("fmiversion");
    if (versionFind != headerInfo.end()) {
        fmiVersion = std::stod(versionFind->second);
    }
    if (reader->hasElement("ModelExchange")) {
        capabilities.set(modelExchangeCapable, true);
        reader->moveToFirstChild("ModelExchange");
        att = reader->getFirstAttribute();
        while (att.isValid()) {
            if (att.getName() == "modelIdentifier") {
                headerInfo["MEIdentifier"] = att.getText();
                headerInfo["meidentifier"] = att.getText();
            } else {
                loadFmuFlag(capabilities, att);
            }

            att = reader->getNextAttribute();
        }
        reader->moveToParent();
    }
    if (reader->hasElement("CoSimulation")) {
        reader->moveToFirstChild("CoSimulation");
        capabilities.set(coSimulationCapable, true);
        att = reader->getFirstAttribute();
        while (att.isValid()) {
            if (att.getName() == "modelIdentifier") {
                headerInfo["CoSimIdentifier"] = att.getText();
                headerInfo["cosimidentifier"] = att.getText();
            } else if (att.getName() == "maxOutputDerivativeOrder") {
                maxOrder = std::stoi(att.getText());
            } else {
                loadFmuFlag(capabilities, att);
            }

            att = reader->getNextAttribute();
        }
        reader->moveToParent();
    }
    if (reader->hasElement("DefaultExperiment")) {
        reader->moveToFirstChild("DefaultExperiment");
        att = reader->getFirstAttribute();
        while (att.isValid()) {
            if (att.getName() == "startTime") {
                defaultExpirement.startTime = att.getValue();
            } else if (att.getName() == "stopTime") {
                defaultExpirement.stopTime = att.getValue();
            } else if (att.getName() == "stepSize") {
                defaultExpirement.stepSize = att.getValue();
            } else if (att.getName() == "tolerance") {
                defaultExpirement.tolerance = att.getValue();
            }

            att = reader->getNextAttribute();
        }
        reader->moveToParent();
    }
}

void loadUnitInfo(std::shared_ptr<readerElement>& reader, FmiUnit& unitInfo);

void FmiInfo::loadUnitInformation(std::shared_ptr<readerElement>& reader)
{
    reader->bookmark();
    reader->moveToFirstChild("UnitDefinitions");
    reader->moveToFirstChild("Unit");
    int vcount{0};
    while (reader->isValid()) {
        reader->moveToNextSibling("Unit");
        ++vcount;
    }
    units.resize(vcount);
    reader->moveToParent();
    // now load the variables
    reader->moveToFirstChild("Unit");
    int index{0};
    while (reader->isValid()) {
        loadUnitInfo(reader, units[index]);
        reader->moveToNextSibling("Unit");
        ++index;
    }
    reader->restore();
}

void loadUnitInfo(std::shared_ptr<readerElement>& reader, FmiUnit& unitInfo)
{
    static std::map<std::string_view, units::precise_unit> baseUnitMap{{"m", units::precise::m},
                                                                       {"s", units::precise::s},
                                                                       {"kg", units::precise::kg},
                                                                       {"mol", units::precise::mol},
                                                                       {"rad", units::precise::rad},
                                                                       {"cd", units::precise::cd},
                                                                       {"K",
                                                                        units::precise::candela},
                                                                       {"A", units::precise::A}};
    unitInfo.name = reader->getAttributeText("name");
    if (reader->hasElement("BaseUnit")) {
        reader->moveToFirstChild("BaseUnit");
        auto att = reader->getFirstAttribute();
        while (att.isValid()) {
            if (att.getName() == "offset") {
                unitInfo.offset = att.getValue();
            } else if (att.getName() == "factor") {
                unitInfo.factor = att.getValue();
            } else {
                unitInfo.baseUnits.emplace_back(att.getName(), att.getValue());
            }
            att = reader->getNextAttribute();
        }
        reader->moveToParent();
    }

    if (reader->hasElement("DisplayUnit")) {
        reader->moveToFirstChild("DisplayUnit");
        while (reader->isValid()) {
            FmiUnitDef Dunit;
            Dunit.name = reader->getAttributeText("name");
            Dunit.factor = reader->getAttributeValue("factor");
            Dunit.offset = reader->getAttributeValue("offset");
            unitInfo.displayUnits.push_back(Dunit);
            reader->moveToNextSibling("DisplayUnit");
        }
        reader->moveToParent();
    }
    [[maybe_unused]] auto def = units::unit_from_string(unitInfo.name);
    units::precise_unit build = units::precise::one;
    for (const auto& udef : unitInfo.baseUnits) {
        auto fnd = baseUnitMap.find(udef.name);
        if (fnd != baseUnitMap.end()) {
            build = build * fnd->second.pow(static_cast<int>(udef.factor));
        }
    }
    build = units::precise_unit(unitInfo.factor, build);
}

/** load a single variable information from the XML
@param[in] reader the readerElement to load from
@param[out] vInfo the variable information to store the data to
*/
static void loadVariableInfo(std::shared_ptr<readerElement>& reader, VariableInformation& vInfo);

/*
valueReference="100663424"
description="Constant output value"
variability="tunable"
*/

static const std::string ScalarVString("ScalarVariable");

void FmiInfo::loadVariables(std::shared_ptr<readerElement>& reader)
{
    reader->bookmark();
    reader->moveToFirstChild("ModelVariables");
    // Loop over the variables to be able to allocate memory efficiently later on
    reader->moveToFirstChild(ScalarVString);
    int vcount = 0;

    while (reader->isValid()) {
        ++vcount;
        reader->moveToNextSibling(ScalarVString);
    }
    variables.resize(vcount);
    reader->moveToParent();
    // now load the variables
    reader->moveToFirstChild(ScalarVString);
    int index = 0;
    while (reader->isValid()) {
        loadVariableInfo(reader, variables[index]);
        variables[index].index = index;
        auto res = variableLookup.emplace(variables[index].name, index);
        if (!res.second) {  // if we failed on the emplace operation, then we need to override
            // this should be unusual but it is possible
            variableLookup[variables[index].name] = index;
        }
        // this one may fail and that is ok since this is a secondary detection mechanism for purely
        // lower case parameters and may not be needed
        variableLookup.emplace(convertToLowerCase(variables[index].name), index);
        switch (variables[index].causality) {
            case fmi_causality::parameter:
                parameters.push_back(index);
                break;
            case fmi_causality::local:
                local.push_back(index);
                break;
            case fmi_causality::input:
                inputs.push_back(index);
                break;
            default:
                break;
        }
        reader->moveToNextSibling(ScalarVString);
        ++index;
    }
    reader->restore();
}

static void loadVariableInfo(std::shared_ptr<readerElement>& reader, VariableInformation& vInfo)
{
    auto att = reader->getFirstAttribute();
    while (att.isValid()) {
        if (att.getName() == "name") {
            vInfo.name = att.getText();
        } else if (att.getName() == "valueReference") {
            vInfo.valueRef = static_cast<fmi2ValueReference>(att.getInt());
        } else if (att.getName() == "description") {
            vInfo.description = att.getText();
        } else if (att.getName() == "variability") {
            vInfo.variability = fmi_variability::_from_string(att.getText().c_str());
        } else if (att.getName() == "causality") {
            vInfo.causality = fmi_causality::_from_string(att.getText().c_str());
        } else if (att.getName() == "initial") {
            vInfo.initial = att.getText();
        }
        att = reader->getNextAttribute();
    }
    if (reader->hasElement("Real")) {
        vInfo.type = fmi_variable_type::real;
        reader->moveToFirstChild("Real");
        att = reader->getFirstAttribute();
        while (att.isValid()) {
            if (att.getName() == "declaredType") {
                vInfo.declType = att.getText();
            } else if (att.getName() == "unit") {
                vInfo.unit = att.getText();
            } else if (att.getName() == "start") {
                vInfo.start = att.getValue();
            } else if (att.getName() == "derivative") {
                vInfo.derivative = true;
                vInfo.derivativeIndex = static_cast<int>(att.getInt());
            } else if (att.getName() == "min") {
                vInfo.min = att.getValue();
            } else if (att.getName() == "max") {
                vInfo.max = att.getValue();
            }
            att = reader->getNextAttribute();
        }
        if (vInfo.variability == +fmi_variability::unknown) {
            vInfo.variability = fmi_variability::continuous;
        }
        reader->moveToParent();
    } else if (reader->hasElement("Boolean")) {
        vInfo.type = fmi_variable_type::boolean;
        reader->moveToFirstChild("Boolean");
        att = reader->getFirstAttribute();
        while (att.isValid()) {
            if (att.getName() == "start") {
                vInfo.start = (att.getText() == "true") ? 1.0 : 0.0;
            }
            att = reader->getNextAttribute();
        }
        if (vInfo.variability == +fmi_variability::unknown) {
            vInfo.variability = fmi_variability::discrete;
        }
        reader->moveToParent();
    } else if (reader->hasElement("String")) {
        vInfo.type = fmi_variable_type::string;
        reader->moveToFirstChild("String");
        att = reader->getFirstAttribute();
        while (att.isValid()) {
            if (att.getName() == "start") {
                vInfo.initial = att.getText();
            }
            att = reader->getNextAttribute();
        }
        reader->moveToParent();
    } else if (reader->hasElement("Integer")) {
        vInfo.type = fmi_variable_type::integer;
        reader->moveToFirstChild("Integer");
        att = reader->getFirstAttribute();
        while (att.isValid()) {
            if (att.getName() == "start") {
                vInfo.initial = att.getValue();
            } else if (att.getName() == "min") {
                vInfo.declType = att.getValue();
            } else if (att.getName() == "max") {
                vInfo.max = att.getValue();
            }
            att = reader->getNextAttribute();
        }
        if (vInfo.variability == +fmi_variability::unknown) {
            vInfo.variability = fmi_variability::discrete;
        }
        reader->moveToParent();
    } else if (reader->hasElement("Enumeration")) {
        vInfo.type = fmi_variable_type::enumeration;
        reader->moveToFirstChild("Enumeration");
        att = reader->getFirstAttribute();
        while (att.isValid()) {
            if (att.getName() == "start") {
                vInfo.initial = att.getValue();
            } else if (att.getName() == "declaredType") {
                vInfo.declType = att.getValue();
            }
            att = reader->getNextAttribute();
        }
        if (vInfo.variability == +fmi_variability::unknown) {
            vInfo.variability = fmi_variability::discrete;
        }
        reader->moveToParent();
    }
}

auto depkindNum(const std::string& depknd)
{
    if (depknd == "dependent") {
        return 1;
    }
    if (depknd == "fixed") {
        return 2;
    }
    if (depknd == "constant") {
        return 3;
    }
    if (depknd == "tunable") {
        return 4;
    }
    if (depknd == "discrete") {
        return 5;
    }
    return 6;
}

static const std::string unknownString("Unknown");
static const std::string depString("dependencies");
static const std::string depKindString("dependenciesKind");

static void loadDependencies(std::shared_ptr<readerElement>& reader,
                             std::vector<int>& store,
                             matrixData<int>& depData)
{
    reader->moveToFirstChild(unknownString);
    while (reader->isValid()) {
        auto att = reader->getAttribute("index");
        auto attDep = reader->getAttribute(depString);
        auto attDepKind = reader->getAttribute(depKindString);
        auto row = static_cast<index_t>(att.getValue());
        auto dep = gmlc::utilities::str2vector<int>(attDep.getText(), 0, " ");
        gmlc::utilities::stringVector depknd = (attDepKind.isValid()) ?
            gmlc::utilities::stringOps::splitline(
                attDepKind.getText(), " ", gmlc::utilities::stringOps::delimiter_compression::on) :
            gmlc::utilities::stringVector();
        store.push_back(row - 1);
        auto validdepkind = !depknd.empty();
        for (size_t kk = 0; kk < dep.size(); ++kk) {
            if (dep[kk] > 0) {
                depData.assign(row - 1, dep[kk] - 1, (validdepkind) ? (depkindNum(depknd[kk])) : 1);
            }
        }
        reader->moveToNextSibling(unknownString);
    }
    reader->moveToParent();
}

void FmiInfo::loadStructure(std::shared_ptr<readerElement>& reader)
{
    reader->bookmark();
    // get the output dependencies
    outputDep.setRowLimit(static_cast<index_t>(variables.size()));
    reader->moveToFirstChild("ModelStructure");

    reader->moveToFirstChild("Outputs");
    if (reader->isValid()) {
        loadDependencies(reader, outputs, outputDep);
    }
    reader->moveToParent();
    // get the derivative dependencies
    reader->moveToFirstChild("Derivatives");
    derivDep.setRowLimit(static_cast<index_t>(variables.size()));
    if (reader->isValid()) {
        loadDependencies(reader, deriv, derivDep);
        for (auto& der : deriv) {
            states.push_back(variables[der].derivativeIndex);
        }
    }

    reader->moveToParent();
    // get the initial unknowns dependencies
    unknownDep.setRowLimit(static_cast<index_t>(variables.size()));
    reader->moveToFirstChild("InitialUnknowns");
    if (reader->isValid()) {
        loadDependencies(reader, initUnknown, unknownDep);
    }
    reader->restore();
}

bool checkType(const VariableInformation& info, fmi_variable_type type, fmi_causality caus)
{
    if (info.causality != caus) {
        if ((info.causality != +fmi_causality::input) || (caus != +fmi_causality::parameter)) {
            return false;
        }
    }
    if (info.type == type) {
        return true;
    }
    if (type == +fmi_variable_type::numeric) {
        switch (info.type) {
            case fmi_variable_type::boolean:
            case fmi_variable_type::integer:
            case fmi_variable_type::real:
            case fmi_variable_type::enumeration:
                return true;
            default:
                return false;
        }
    }
    return false;
}
