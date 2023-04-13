/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

/** @file
@brief file containing classes and types for managing information about FMU's
*/
#pragma once

#include "fmi2TypesPlatform.h"
#include "fmiEnumDefinitions.h"
#include "units/units.hpp"
#include "utilities/matrixDataOrdered.hpp"

#ifdef __GNUC__
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#    include "boost/container/small_vector.hpp"
#    pragma GCC diagnostic pop
#else
#    include "boost/container/small_vector.hpp"
#endif

#include <bitset>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/** data class containing the default experiment information*/
class FmuDefaultExpirement {
  public:
    double startTime = 0.0;
    double stopTime = 0.0;
    double stepSize = 0.00;
    double tolerance = 1e-8;
};

/** data class containing information about a variable*/
class VariableInformation {
  public:
    int index = -1;
    int derivativeIndex = -1;
    fmi2ValueReference valueRef = 0;
    int aliasIndex = 0;
    std::string name;
    std::string description;
    std::string declType;
    std::string unit;
    std::string initial;
    bool multiSet = false;
    bool reinit = false;
    bool derivative = false;
    bool isAlias = false;
    fmi_variability variability{fmi_variability::unknown};
    fmi_causality causality{fmi_causality::unknown};
    fmi_variable_type type{fmi_variable_type::unknown};
    double start = 0;
    double min = -1e48;
    double max = 1e48;
};

class FmiUnitDef {
  public:
    std::string name;
    double factor{1.0};
    double offset{0.0};
    FmiUnitDef() = default;
    FmiUnitDef(std::string_view n, double mult): name(n), factor(mult) {}
};

/** data class for storing fmi unit information*/
class FmiUnit: public FmiUnitDef {
  public:
    std::vector<FmiUnitDef> baseUnits;
    std::vector<FmiUnitDef> displayUnits;
    units::precise_unit unitValue;
};

/**data class matching the definition of an FMI type*/
class FmiTypeDefinition {
  public:
    std::string name;
    std::string description;
    std::string quantity;
    std::string unit;
    std::string displayUnit;
    fmi_variable_type type;
    bool relativeQuantity{false};
    bool unbounded{false};
    double min;
    double max;
    double nominal;
};

/** class defining a single variable */
class FmiVariable {
  public:
    fmi2ValueReference vRef{static_cast<fmi2ValueReference>(-1)};
    fmi_variable_type type{fmi_variable_type::unknown};
    int index{-1};
    FmiVariable() = default;
    FmiVariable(fmi2ValueReference ref, fmi_variable_type type1, int startIndex):
        vRef{ref}, type(type1), index{startIndex}
    {
    }
    FmiVariable(const VariableInformation& var):
        vRef{var.valueRef}, type(var.type), index(var.index)
    {
    }
};

/** class for storing references to fmi variables*/
class FmiVariableSet {
  public:
    FmiVariableSet();
    FmiVariableSet(fmi2ValueReference newvr);
    FmiVariableSet(const FmiVariableSet& vset);
    FmiVariableSet(FmiVariableSet&& vset) noexcept;

    FmiVariableSet& operator=(const FmiVariableSet& other);
    FmiVariableSet& operator=(FmiVariableSet&& other) noexcept;

    const fmi2ValueReference* getValueRef() const;
    size_t getVRcount() const;
    fmi_variable_type getType() const;
    /** add a new reference
    @param[in] newvr the value reference to add
    */
    void push(fmi2ValueReference newvr);
    /** add a variable set the existing variable set*
    @param[in] vset the variableSet to add
    */
    void push(const FmiVariableSet& vset);
    /** reserve a set amount of space in the set
    @param[in] newSize the number of elements to reserve*/
    void reserve(size_t newSize);
    void remove(fmi2ValueReference rmvr);
    void clear();

  private:
    fmi_variable_type type = fmi_variable_type::real;
    // boost::container::small_vector<fmi2ValueReference, 4> vrset;
    std::vector<fmi2ValueReference> vrset;
};

class readerElement;
/** class to extract and store the information in an FMU XML file*/
class FmiInfo {
  private:
    std::map<std::string, std::string> headerInfo;  //!< the header information contained in strings
    double fmiVersion{0.0};  //!< the fmi version used
    // int numberOfEvents;  //!< the number of defined events
    int maxOrder{0};  //!< the maximum derivative order for CoSimulation FMU's
    std::bitset<32> capabilities;  //!< bitset containing the capabilities of the FMU
    std::vector<VariableInformation> variables;  //!< information all the defined variables
    std::vector<FmiUnit> units;  //!< all the units defined in the FMU
    FmuDefaultExpirement
        defaultExpirement;  //!< the information about the specified default experiment

    std::map<std::string, int>
        variableLookup;  //!< map translating strings to indices into the variables array

    matrixDataOrdered<sparse_ordering::row_ordered, int>
        outputDep;  //!< the output dependency information
    matrixDataOrdered<sparse_ordering::row_ordered, int>
        derivDep;  //!< the derivative dependency information
    matrixDataOrdered<sparse_ordering::row_ordered, int>
        unknownDep;  //!< the initial unknown dependency information
    std::vector<int> outputs;  //!< a list of the output indices
    std::vector<int> parameters;  //!< a list of all the parameters
    std::vector<int> local;  //!< a list of the local variables
    std::vector<int> states;  //!< a list of the states
    std::vector<int> deriv;  //!< a list of the derivative information
    std::vector<int> initUnknown;  //!< a list of the unknowns
    std::vector<int> inputs;  //!< a list of the inputs
    int eventIndicators{0};  //!< number of event indicators
  public:
    FmiInfo();
    explicit FmiInfo(const std::string& fileName);
    int loadFile(const std::string& fileName);
    /** check if a given flag is set*/
    bool checkFlag(fmuCapabilityFlags flag) const;

    const FmuDefaultExpirement& getExperiment() const { return defaultExpirement; }
    /** get the counts for various items in a fmu
    @param[in] countType the type of counts to get
    @return the count*/
    int getCounts(fmiVariableType countType) const;
    const std::string& getString(const std::string& field) const;
    /** get a Real variable by name*/
    double getReal(const std::string& field) const;
    const VariableInformation& getVariableInfo(const std::string& variableName) const;
    const VariableInformation& getVariableInfo(unsigned int index) const;
    /** get a set of variables for the specified parameters*/
    FmiVariableSet getReferenceSet(const std::vector<std::string>& variableList) const;
    /** get a variable set with a single member*/
    FmiVariableSet getVariableSet(const std::string& variable) const;
    /** get a variable set with a single member based on index*/
    FmiVariableSet getVariableSet(unsigned int index) const;
    /** get a set of the current outputs*/
    FmiVariableSet getOutputReference() const;
    /** get a set of the current inputs*/
    FmiVariableSet getInputReference() const;
    /** get a list of variable names by type
    @param[in] type the type of variable
    @return a vector of strings with the names of the variables
    */
    std::vector<std::string> getVariableNames(const std::string& type) const;
    /** get a list of variable indices by type
    @param[in] type the type of variable
    @return a vector of ints with the indices of the variables
    */
    const std::vector<int>& getVariableIndices(const std::string& type) const;
    /** get the variable indices of the derivative dependencies*/
    const std::vector<std::pair<index_t, int>>& getDerivDependencies(int variableIndex) const;
    const std::vector<std::pair<index_t, int>>& getOutputDependencies(int variableIndex) const;
    const std::vector<std::pair<index_t, int>>& getUnknownDependencies(int variableIndex) const;

  private:
    void loadFmiHeader(std::shared_ptr<readerElement>& reader);
    void loadVariables(std::shared_ptr<readerElement>& reader);
    void loadUnitInformation(std::shared_ptr<readerElement>& reader);
    void loadStructure(std::shared_ptr<readerElement>& reader);
};

enum class FmuMode {
    INSTANTIATED,
    INITIALIZATION,
    CONTINUOUS_TIME,
    EVENT,
    STEP,  //!< step Mode is a synonym for event mode that make more sense for cosimulation
    TERMINATED,
    ERROR,
};

bool checkType(const VariableInformation& info, fmi_variable_type type, fmi_causality caus);
