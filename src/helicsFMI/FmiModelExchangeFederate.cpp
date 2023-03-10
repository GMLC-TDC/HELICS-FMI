/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "FmiModelExchangeFederate.hpp"

#include "FmiHelics.hpp"
#include "fmi/fmi_import/fmiObjects.h"
#include "solvers/solverInterface.h"

#include <utility>

FmiModelExchangeFederate::FmiModelExchangeFederate(std::shared_ptr<fmi2ModelExchangeObject> obj,
                                                   const helics::FederateInfo& fi):
    fed(std::string(), fi),
    me(std::move(obj))

{
    if (me) {
        input_list = me->getInputNames();
        output_list = me->getOutputNames();
    }
}

FmiModelExchangeFederate::~FmiModelExchangeFederate() = default;

void FmiModelExchangeFederate::configure(helics::Time step, helics::Time startTime)
{
    timeBias = startTime;
    for (auto input : input_list) {
        inputs.emplace_back(&fed, input);
    }

    for (auto output : output_list) {
        pubs.emplace_back(&fed, output, helics::DataType::HELICS_DOUBLE);
    }

    auto& def = me->fmuInformation().getExperiment();

    if (step <= helics::timeZero) {
        step = def.stepSize;
    }
    if (step <= helics::timeZero) {
        auto tstep = fed.getTimeProperty(HELICS_PROPERTY_TIME_PERIOD);
        step = (tstep > helics::timeEpsilon) ? tstep : helics::Time(0.2);
    }
    fed.setProperty(HELICS_PROPERTY_TIME_PERIOD, step);
    stepTime = step;
    solver = griddyn::makeSolver("cvode", "cvode");
}

void FmiModelExchangeFederate::setInputs(std::vector<std::string> input_names)
{
    input_list = std::move(input_names);
}
void FmiModelExchangeFederate::setOutputs(std::vector<std::string> output_names)
{
    output_list = std::move(output_names);
}
void FmiModelExchangeFederate::setConnections(std::vector<std::string> conn)
{
    connections = std::move(conn);
}

void FmiModelExchangeFederate::addInput(const std::string& input_name)
{
    input_list.push_back(input_name);
}

void FmiModelExchangeFederate::addOutput(const std::string& output_name)
{
    output_list.push_back(output_name);
}

void FmiModelExchangeFederate::addConnection(const std::string& conn)
{
    connections.push_back(conn);
}

void FmiModelExchangeFederate::run(helics::Time stop)
{
    auto& def = me->fmuInformation().getExperiment();

    if (stop <= helics::timeZero) {
        stop = def.stopTime;
    }
    if (stop <= helics::timeZero) {
        stop = 30.0;
    }

    fed.enterInitializingMode();
    me->setMode(fmuMode::initializationMode);

    if (!pubs.empty()) {
        for (std::size_t ii = 0; ii < pubs.size(); ++ii) {
            helicsfmi::publishOutput(pubs[ii], me.get(), ii);
        }
    }
    if (!inputs.empty()) {
        for (std::size_t ii = 0; ii < inputs.size(); ++ii) {
            helicsfmi::setDefault(inputs[ii], me.get(), ii);
        }
    }
    auto result = fed.enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);
    if (result == helics::IterationResult::ITERATING) {
        if (!inputs.empty()) {
            for (std::size_t ii = 0; ii < inputs.size(); ++ii) {
                helicsfmi::grabInput(inputs[ii], me.get(), ii);
            }
        }
        fed.enterExecutingMode();
    }
    me->setMode(fmuMode::continuousTimeMode);

    helics::Time currentTime = helics::timeZero;
    while (currentTime <= stop) {
        // me->doStep(static_cast<double>(currentTime), static_cast<double>(stepTime), true);
        double timeReturn;
        solver->solve(static_cast<double>(currentTime), timeReturn);
        currentTime = fed.requestNextStep();
        // get the values to publish
        if (!pubs.empty()) {
            // get the values to publish
            for (std::size_t ii = 0; ii < pubs.size(); ++ii) {
                helicsfmi::publishOutput(pubs[ii], me.get(), ii);
            }
        }
        if (!inputs.empty()) {
            // load the inputs
            for (std::size_t ii = 0; ii < inputs.size(); ++ii) {
                helicsfmi::grabInput(inputs[ii], me.get(), ii);
            }
        }
    }
    fed.finalize();
}

solver_index_type
    FmiModelExchangeFederate::jacobianSize([[maybe_unused]] const griddyn::solverMode& sMode) const
{
    return 0;
}

void FmiModelExchangeFederate::guessCurrentValue([[maybe_unused]] double time,
                                                 double state[],
                                                 double dstate_dt[],
                                                 [[maybe_unused]] const griddyn::solverMode& sMode)
{
    if (hasDifferential(sMode)) {
        me->getStates(state);
        me->getDerivatives(dstate_dt);
    } else if (!isDynamic(sMode)) {
        me->getStates(state);
    }
}

int FmiModelExchangeFederate::residualFunction(double time,
                                               const double state[],
                                               const double dstate_dt[],
                                               double resid[],
                                               const griddyn::solverMode& sMode) noexcept

{
    if (hasDifferential(sMode)) {
        derivativeFunction(time, state, resid, sMode);
        for (index_t ii = 0; ii < solver->size(); ++ii) {
            resid[ii] -= dstate_dt[ii];
        }
    } else if (!isDynamic(sMode)) {
        derivativeFunction(time, state, resid, sMode);
    }
    return 0;
}

int FmiModelExchangeFederate::derivativeFunction(
    double time,
    const double state[],
    double dstate_dt[],
    [[maybe_unused]] const griddyn::solverMode& sMode) noexcept
{
    me->setStates(state);
    me->getDerivatives(dstate_dt);
    printf("tt=%f, state=%f deriv=%e\n", time, state[0], dstate_dt[0]);
    return 0;
}

int FmiModelExchangeFederate::algUpdateFunction([[maybe_unused]] double time,
                                                [[maybe_unused]] const double state[],
                                                [[maybe_unused]] double update[],
                                                [[maybe_unused]] const griddyn::solverMode& sMode,
                                                [[maybe_unused]] double alpha) noexcept
{
    return 0;
}

int FmiModelExchangeFederate::jacobianFunction(
    [[maybe_unused]] double time,
    [[maybe_unused]] const double state[],
    [[maybe_unused]] const double dstate_dt[],
    [[maybe_unused]] matrixData<double>& md,
    [[maybe_unused]] double cj,
    [[maybe_unused]] const griddyn::solverMode& sMode) noexcept
{
    return 0;
}

int FmiModelExchangeFederate::rootFindingFunction(
    [[maybe_unused]] double time,
    [[maybe_unused]] const double state[],
    [[maybe_unused]] const double dstate_dt[],
    [[maybe_unused]] double roots[],
    [[maybe_unused]] const griddyn::solverMode& sMode) noexcept
{
    return 0;
}

int FmiModelExchangeFederate::dynAlgebraicSolve(
    [[maybe_unused]] double time,
    [[maybe_unused]] const double diffState[],
    [[maybe_unused]] const double deriv[],
    [[maybe_unused]] const griddyn::solverMode& sMode) noexcept
{
    return 0;
}
