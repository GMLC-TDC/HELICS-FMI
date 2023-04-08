/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "fmiImport.h"

#include <exception>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

/** base fmiException*/
class fmiException: public std::exception {
  public:
    virtual const char* what() const noexcept override { return "fmi Exception"; }
};

/** fmi exception for when fmiDiscard was returned*/
class fmiDiscardException: public fmiException {
  public:
    virtual const char* what() const noexcept override { return "return fmiDiscard"; }
};
/** fmi exception for when fmiWarning was returned*/
class fmiWarningException: public fmiException {
  public:
    virtual const char* what() const noexcept override { return "return fmiWarning"; }
};

/** fmi exception for when an error was returned*/
class fmiErrorException: public fmiException {
  public:
    virtual const char* what() const noexcept override { return "return fmiError"; }
};

/** fmi exception for when a fatal error was returned*/
class fmiFatalException: public fmiException {
  public:
    virtual const char* what() const noexcept override { return "return fmiFatal"; }
};

/** base class containing the operation functions for working with an FMU*/
class fmi2Object {
  public:
    fmi2Object(const std::string& name,
               fmi2Component cmp,
               std::shared_ptr<const fmiInfo> info,
               std::shared_ptr<const fmiCommonFunctions> comFunc);
    virtual ~fmi2Object();
    void setupExperiment(fmi2Boolean toleranceDefined,
                         fmi2Real tolerance,
                         fmi2Real startTime,
                         fmi2Boolean stopTimeDefined,
                         fmi2Real stopTime);
    virtual void setMode(fmuMode newMode);
    fmuMode getCurrentMode() const;
    void reset();

    template<typename T>
    T get(const fmiVariable& param) const
    {
        fmi2Status retval = fmi2Status::fmi2Discard;
        if constexpr (std::is_arithmetic_v<T>) {
            T ret(0);
            switch (param.type) {
                case fmi_variable_type::real: {
                    fmi2Real res;
                    retval = commonFunctions->fmi2GetReal(comp, &(param.vRef), 1, &res);
                    ret = static_cast<T>(res);
                } break;
                case fmi_variable_type::integer: {
                    fmi2Integer res;
                    retval = commonFunctions->fmi2GetInteger(comp, &(param.vRef), 1, &res);
                    ret = static_cast<T>(res);
                } break;
                case fmi_variable_type::boolean: {
                    fmi2Boolean res;
                    retval = commonFunctions->fmi2GetBoolean(comp, &(param.vRef), 1, &res);
                    ret = static_cast<T>(res);
                } break;
                case fmi_variable_type::enumeration: {
                    fmi2Integer res;
                    retval = commonFunctions->fmi2GetInteger(comp, &(param.vRef), 1, &res);
                    ret = static_cast<T>(res);
                } break;
                default:
                    retval = fmi2Status::fmi2Discard;
                    break;
            }

            if (retval != fmi2Status::fmi2OK) {
                handleNonOKReturnValues(retval);
            }
            return ret;
        } else if constexpr (std::is_constructible<T, fmi2String>::value) {
            if (param.type._value != fmi_variable_type::string) {
                handleNonOKReturnValues(fmi2Status::fmi2Discard);
                return T{""};  // if we get here just return an empty string otherwise we threw an
                               // exception
            }
            fmi2String res = nullptr;
            retval = commonFunctions->fmi2GetString(comp, &(param.vRef), 1, &res);
            if (retval != fmi2Status::fmi2OK) {
                handleNonOKReturnValues(retval);
            }
            if (res == nullptr) {
                return T{""};
            }
            return T{res};  // this should copy the actual the string
        } else {
            if (retval != fmi2Status::fmi2OK) {
                handleNonOKReturnValues(retval);
            }
            return T{};
        }
    }

    template<typename T>
    T get(const std::string& param) const
    {
        return get<T>(info->getVariableInfo(param));
    }

    void get(const fmiVariableSet& vrset, fmi2Real[]) const;
    void get(const fmiVariableSet& vrset, fmi2Integer[]) const;
    void get(const fmiVariableSet& vrset, fmi2String[]) const;

    void set(const fmiVariableSet& vrset, fmi2Integer[]);

    void set(const fmiVariableSet& vrset, fmi2Real[]);

    template<typename T>
    void set(const std::string& param, T&& val)
    {
        set(info->getVariableInfo(param), std::forward<T>(val));
    }

    void set(const fmiVariable& param, const char* val);
    void set(const fmiVariable& param, const std::string& val);

    template<typename T>
    void set(const fmiVariable& param, T val)
    {
        fmi2Status ret = fmi2Status::fmi2Discard;
        switch (param.type._value) {
            case fmi_variable_type::real: {
                fmi2Real val2 = static_cast<fmi2Real>(val);
                ret = commonFunctions->fmi2SetReal(comp, &(param.vRef), 1, &val2);
            } break;
            case fmi_variable_type::integer: {
                fmi2Integer val2 = static_cast<fmi2Integer>(val);
                ret = commonFunctions->fmi2SetInteger(comp, &(param.vRef), 1, &val2);
            } break;
            case fmi_variable_type::boolean: {
                fmi2Boolean val2 = static_cast<fmi2Boolean>(val);
                ret = commonFunctions->fmi2SetBoolean(comp, &(param.vRef), 1, &val2);
            } break;
            case fmi_variable_type::enumeration: {
                fmi2Integer val2 = static_cast<fmi2Integer>(val);
                ret = commonFunctions->fmi2SetInteger(comp, &(param.vRef), 1, &val2);
            } break;
            default:
                break;
        }
        if (ret != fmi2Status::fmi2OK) {
            handleNonOKReturnValues(ret);
        }
    }
    /** set a flag on the fmi object *
    @return true if a flag was set
    */
    bool setFlag(const std::string& param, bool val);
    void getFMUState(fmi2FMUstate* FMUState);
    void setFMUState(fmi2FMUstate FMUState);

    size_t serializedStateSize(fmi2FMUstate FMUState);
    void serializeState(fmi2FMUstate FMUState, fmi2Byte serializedState[], size_t size);

    void deSerializeState(const fmi2Byte serializedState[], size_t size, fmi2FMUstate* FMUstate);
    void getDirectionalDerivative(const fmi2ValueReference vUnknown_ref[],
                                  size_t nUnknown,
                                  const fmi2ValueReference vKnown_ref[],
                                  size_t unknown,
                                  const fmi2Real dvKnown[],
                                  fmi2Real dvUnknown[]);

    fmi2Real getPartialDerivative(int index_x, int index_y, double deltax);
    void setOutputVariables(const std::vector<std::string>& outNames);
    void setOutputVariables(const std::vector<int>& outIndices);
    void setInputVariables(const std::vector<std::string>& inNames);
    void setInputVariables(const std::vector<int>& inIndices);
    const fmiVariable& addOutputVariable(const std::string& outputName);
    const fmiVariable& addOutputVariable(int index);
    const fmiVariable& addInputVariable(const std::string& inputName);
    const fmiVariable& addInputVariable(int index);

    fmiVariableSet getVariableSet(const std::string& variable) const;
    fmiVariableSet getVariableSet(int index) const;

    const fmiInfo& fmuInformation() const { return *info; }

    int inputSize() const { return static_cast<int>(activeInputs.size()); }
    int outputSize() const { return static_cast<int>(activeOutputs.size()); }

    const fmiVariable& getInput(int index) const;
    const fmiVariable& getOutput(int index) const;

    std::vector<std::string> getOutputNames() const;
    std::vector<std::string> getInputNames() const;

    bool isParameter(const std::string& param, fmi_variable_type type = fmi_variable_type::numeric);
    bool isVariable(const std::string& var, fmi_variable_type type = fmi_variable_type::numeric);
    std::shared_ptr<const fmiCommonFunctions> getFmiCommonFunctions() const
    {
        return commonFunctions;
    }

    std::shared_ptr<FmiLogger> getLogger() const { return logger; }

    void setLogger(std::shared_ptr<FmiLogger> logFunction) { logger = std::move(logFunction); }
    /** set the logging callback*/
    void setLoggingCallback(std::function<void(std::string_view)> callback)
    {
        if (logger) {
            logger->setLoggerCallback(std::move(callback));
        }
    }

    fmi2Component getFmiComponent() const { return comp; }
    /** get the name of the object*/
    const std::string& getName() const { return name; }

  protected:
    fmi2Component comp;
    fmuMode currentMode = fmuMode::instantiatedMode;
    std::shared_ptr<const fmiInfo> info;

    std::vector<fmiVariable> activeInputs;
    std::vector<fmiVariable> activeOutputs;

    void handleNonOKReturnValues(fmi2Status retval) const;
    /** set the inputs to all the defined inputs*/
    void setDefaultInputs();
    /** set the outputs to be all defined outputs*/
    void setDefaultOutputs();

  private:
    /// flag indicating that an exception should be thrown when an input is discarded
    bool exceptionOnDiscard{true};
    /// flag indicating that an exception should be thrown on a fmiWarning
    bool exceptionOnWarning{false};
    /// @brief  flag indicating that the free function should not be called on destructor
    bool noFree{false};
    std::shared_ptr<const fmiCommonFunctions> commonFunctions;
    const std::string name;
    std::shared_ptr<FmiLogger> logger;
};

/** template overload for getting strings*/
template<>
std::string fmi2Object::get<std::string>(const std::string& param) const;

/** class containing the information for working with a model exchange object*/
class fmi2ModelExchangeObject: public fmi2Object {
  public:
    fmi2ModelExchangeObject(const std::string& name,
                            fmi2Component cmp,
                            std::shared_ptr<const fmiInfo> info,
                            std::shared_ptr<const fmiCommonFunctions> comFunc,
                            std::shared_ptr<const fmiModelExchangeFunctions> MEFunc);
    void newDiscreteStates(fmi2EventInfo* fmi2eventInfo);
    /** call for a completed integrator step
    @param[in] noSetFMUStatePriorToCurrentPoint flag indicating that the state will not be updated
    at a prior time point
    @param[out] enterEventMode output flag indicating that an event was triggered and event Mode
    needs to be entered
    @param[out] terminatesSimulation output flag indicating thaat the simulation was terminated
    @throw an error if the underlying fmi returns an error code
    */
    void completedIntegratorStep(fmi2Boolean noSetFMUStatePriorToCurrentPoint,
                                 fmi2Boolean* enterEventMode,
                                 fmi2Boolean* terminatesSimulation);
    void setTime(fmi2Real time);
    void setStates(const fmi2Real states[]);
    void getDerivatives(fmi2Real deriv[]) const;
    void getEventIndicators(fmi2Real eventIndicators[]) const;
    /** get the current values for the states
    @param[out] states the location to store the state data states must have sufficient space
    allocated for the states
    */
    void getStates(fmi2Real states[]) const;
    /** get the nominal expected values for the states
    @param[out] nominalValues the location to store the state data states must have sufficient space
    allocated for the states
    */
    void getNominalsOfContinuousStates(fmi2Real nominalValues[]) const;
    /** set the operating state of the FMU
     */
    virtual void setMode(fmuMode mode) override;
    /** get the number of the states*/
    size_t getNumberOfStates() const { return numStates; }
    /** get the number of indicators*/
    size_t getNumberOfIndicators() const { return numIndicators; }
    /** get a pointer to the model exchange functions from the library
     */
    std::shared_ptr<const fmiModelExchangeFunctions> getModelExchangeFunctions() const
    {
        return ModelExchangeFunctions;
    }
    /** get the names of the states
    @return a vector strings containing the names of the states*/
    std::vector<std::string> getStateNames() const;

  private:
    size_t numStates = 0;  //!< the number of states in the FMU
    size_t numIndicators = 0;  //!< the number of event Indicators in the FMU
    bool hasTime = true;  //!< flag indicating that there is a time variable in the system
    std::shared_ptr<const fmiModelExchangeFunctions>
        ModelExchangeFunctions;  //!< pointer the library model exchange functions
};

/** class containing the Information for working with a FMI coSimulation object*/
class fmi2CoSimObject: public fmi2Object {
  public:
    /**constructor*/
    fmi2CoSimObject(const std::string& name,
                    fmi2Component cmp,
                    std::shared_ptr<const fmiInfo> info,
                    std::shared_ptr<const fmiCommonFunctions> comFunc,
                    std::shared_ptr<const fmiCoSimFunctions> csFunc);
    /** set the input derivatives of particular order
    @param[in] order the numerical order of the derivative to set
    @param[in] dIdt the input derivatives must be the size of the number of inputs
    */
    void setInputDerivatives(int order, const fmi2Real dIdt[]);
    /** get the output derivatives of a particular order
    @param[in] order the order of the derivatives to retrieve
    @param[out] dOdt the output derivatives
    */
    void getOutputDerivatives(int order, fmi2Real dOdt[]) const;
    /** advance a time step
    @param[in] currentCommunicationPoint the current time
    @param[in] communicationStepSize the size of the step to take
    @param[in] noSetFMUStatePriorToCurrentPoint flag to indicate that the fmu cannot rollback
    */
    void doStep(fmi2Real currentCommunicationPoint,
                fmi2Real communicationStepSize,
                fmi2Boolean noSetFMUStatePriorToCurrentPoint);
    /** cancel a pending time step*/
    void cancelStep();
    fmi2Real getLastStepTime() const;
    bool isPending();
    std::string getStatus() const;

    std::shared_ptr<const fmiCoSimFunctions> getCoSimulationFunctions() const
    {
        return CoSimFunctions;
    }
    /** set the operating state of the FMU
     */
    virtual void setMode(fmuMode mode) override;

  private:
    std::shared_ptr<const fmiCoSimFunctions> CoSimFunctions;
    bool stepPending{false};
};
