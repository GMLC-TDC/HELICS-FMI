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

#include "FmiCoSimFederate.hpp"
#include "fmi/fmi_import/fmiObjects.h"
#include <algorithm>

FmiCoSimFederate::FmiCoSimFederate(std::shared_ptr<fmi2CoSimObject> obj, const helics::FederateInfo &fi)
    : cs(std::move(obj)), fed(obj->getName(), fi)
{
    if (cs)
    {
        input_list = cs->getInputNames();
        output_list = cs->getOutputNames();
    }
}

void FmiCoSimFederate::configure(helics::Time step, helics::Time startTime)
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

    auto &def = cs->fmuInformation().getExperiment();

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

void FmiCoSimFederate::setInputs(std::vector<std::string> input_names) { input_list = std::move(input_names); }
void FmiCoSimFederate::setOutputs(std::vector<std::string> output_names) { output_list = std::move(output_names); }
void FmiCoSimFederate::setConnections(std::vector<std::string> conn) { connections = std::move(conn); }

void FmiCoSimFederate::addInput(const std::string &input_name) { input_list.push_back(input_name); }

void FmiCoSimFederate::addOutput(const std::string &output_name) { output_list.push_back(output_name); }

void FmiCoSimFederate::addConnection(const std::string &conn) { connections.push_back(conn); }

void FmiCoSimFederate::run(helics::Time stop)
{
    auto &def = cs->fmuInformation().getExperiment();

    if (stop <= helics::timeZero)
    {
        stop = def.stopTime;
    }
    if (stop <= helics::timeZero)
    {
        stop = 30.0;
    }

    fed.enterInitializingMode();
    cs->setMode(fmuMode::initializationMode);
    std::vector<fmi2Real> outputs(pubs.size());
    std::vector<fmi2Real> inp(inputs.size());
    cs->getOutputs(outputs.data());
    for (size_t ii = 0; ii < pubs.size(); ++ii)
    {
        pubs[ii].publish(outputs[ii]);
    }
    cs->getCurrentInputs(inp.data());
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
        cs->setInputs(inp.data());
        fed.enterExecutingMode();
    }
    cs->setMode(fmuMode::stepMode);

    helics::Time currentTime = helics::timeZero;
    while (currentTime <= stop)
    {
        cs->doStep(static_cast<double>(currentTime), static_cast<double>(stepTime), true);
        currentTime = fed.requestNextStep();
        // get the values to publish
        cs->getOutputs(outputs.data());
        for (size_t ii = 0; ii < pubs.size(); ++ii)
        {
            pubs[ii].publish(outputs[ii]);
        }
        // load the inputs
        for (size_t ii = 0; ii < inputs.size(); ++ii)
        {
            inp[ii] = inputs[ii].getValue<fmi2Real>();
        }
        cs->setInputs(inp.data());
    }
    fed.finalize();
}
