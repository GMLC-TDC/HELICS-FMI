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

#include "FmiModelExchangeFederate.hpp"
#include "fmi/fmi_import/fmiObjects.h"
#include "solvers/solverInterface.h"

FmiModelExchangeFederate::FmiModelExchangeFederate(std::shared_ptr<fmi2ModelExchangeObject> obj,
                                                   const helics::FederateInfo &fi)
    : me(std::move(obj)), fed(std::string(), fi)
{
    if (me)
    {
        input_list = me->getInputNames();
        output_list = me->getOutputNames();
    }
}

FmiModelExchangeFederate::~FmiModelExchangeFederate() = default;

void FmiModelExchangeFederate::configure(helics::Time step, helics::Time startTime)
{
    timeBias = startTime;
    for (auto input : input_list)
    {
        inputs.emplace_back(&fed, input);
    }

    for (auto output : output_list)
    {
        pubs.emplace_back(&fed, output, helics::data_type::helics_double);
    }

    auto &def = me->fmuInformation().getExperiment();

    if (step <= helics::timeZero)
    {
        step = def.stepSize;
    }
    if (step <= helics::timeZero)
    {
        auto tstep = fed.getTimeProperty(helics_property_time_period);
        step = (tstep > helics::timeEpsilon) ? tstep : 0.2;
    }
    fed.setProperty(helics_property_time_period, step);
    stepTime = step;
}

void FmiModelExchangeFederate::setInputs(std::vector<std::string> input_names)
{
    input_list = std::move(input_names);
}
void FmiModelExchangeFederate::setOutputs(std::vector<std::string> output_names)
{
    output_list = std::move(output_names);
}
void FmiModelExchangeFederate::setConnections(std::vector<std::string> conn) { connections = std::move(conn); }

void FmiModelExchangeFederate::addInput(const std::string &input_name) { input_list.push_back(input_name); }

void FmiModelExchangeFederate::addOutput(const std::string &output_name) { output_list.push_back(output_name); }

void FmiModelExchangeFederate::addConnection(const std::string &conn) { connections.push_back(conn); }

void FmiModelExchangeFederate::run(helics::Time stop)
{
    auto &def = me->fmuInformation().getExperiment();

    if (stop <= helics::timeZero)
    {
        stop = def.stopTime;
    }
    if (stop <= helics::timeZero)
    {
        stop = 30.0;
    }

    fed.enterInitializingMode();
    me->setMode(fmuMode::initializationMode);
    std::vector<fmi2Real> outputs(pubs.size());
    std::vector<fmi2Real> inp(inputs.size());
    me->getOutputs(outputs.data());
    for (size_t ii = 0; ii < pubs.size(); ++ii)
    {
        pubs[ii].publish(outputs[ii]);
    }
    me->getCurrentInputs(inp.data());
    for (size_t ii = 0; ii < inputs.size(); ++ii)
    {
        inputs[ii].setDefault(inp[ii]);
    }
    auto result = fed.enterExecutingMode(helics::iteration_request::iterate_if_needed);
    if (result == helics::iteration_result::iterating)
    {
        for (size_t ii = 0; ii < inputs.size(); ++ii)
        {
            inp[ii] = inputs[ii].getValue<fmi2Real>();
        }
        me->setInputs(inp.data());
        fed.enterExecutingMode();
    }
    me->setMode(fmuMode::continuousTimeMode);

    helics::Time currentTime = helics::timeZero;
    while (currentTime <= stop)
    {
        // me->cs->doStep(static_cast<double>(currentTime), static_cast<double>(stepTime), true);
        currentTime = fed.requestNextStep();
        // get the values to publish
        me->getOutputs(outputs.data());
        for (size_t ii = 0; ii < pubs.size(); ++ii)
        {
            pubs[ii].publish(outputs[ii]);
        }
        // load the inputs
        for (size_t ii = 0; ii < inputs.size(); ++ii)
        {
            inp[ii] = inputs[ii].getValue<fmi2Real>();
        }
        me->setInputs(inp.data());
    }
    fed.finalize();
}

solver_index_type FmiModelExchangeFederate::jacobianSize(const griddyn::solverMode &sMode) const { return 0; }

void FmiModelExchangeFederate::guessCurrentValue(double time,
                                                 double state[],
                                                 double dstate_dt[],
                                                 const griddyn::solverMode &sMode)
{
}

int FmiModelExchangeFederate::residualFunction(double time,
                                               const double state[],
                                               const double dstate_dt[],
                                               double resid[],
                                               const griddyn::solverMode &sMode) noexcept

{
    return 0;
}

int FmiModelExchangeFederate::derivativeFunction(double time,
                                                 const double state[],
                                                 double dstate_dt[],
                                                 const griddyn::solverMode &sMode) noexcept
{
    return 0;
}

int FmiModelExchangeFederate::algUpdateFunction(double time,
                                                const double state[],
                                                double update[],
                                                const griddyn::solverMode &sMode,
                                                double alpha) noexcept
{
    return 0;
}

int FmiModelExchangeFederate::jacobianFunction(double time,
                                               const double state[],
                                               const double dstate_dt[],
                                               matrixData<double> &md,
                                               double cj,
                                               const griddyn::solverMode &sMode) noexcept
{
    return 0;
}

int FmiModelExchangeFederate::rootFindingFunction(double time,
                                                  const double state[],
                                                  const double dstate_dt[],
                                                  double roots[],
                                                  const griddyn::solverMode &sMode) noexcept
{
    return 0;
}

int FmiModelExchangeFederate::dynAlgebraicSolve(double time,
                                                const double diffState[],
                                                const double deriv[],
                                                const griddyn::solverMode &sMode) noexcept
{
    return 0;
}
