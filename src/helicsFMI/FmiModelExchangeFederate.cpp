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

void FmiModelExchangeFederate::configure(helics::Time step)
{
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

void FmiModelExchangeFederate::run(helics::Time stop) {}
