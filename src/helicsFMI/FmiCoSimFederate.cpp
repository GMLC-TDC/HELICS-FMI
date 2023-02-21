/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "FmiCoSimFederate.hpp"

#include "fmi/fmi_import/fmiObjects.h"

#include <algorithm>
#include <fstream>
#include <utility>

FmiCoSimFederate::FmiCoSimFederate(const std::string& name,
                                   const std::string& fmu,
                                   const helics::FederateInfo& fi):
    fed(name, fi)
{
    auto fmi = std::make_shared<FmiLibrary>();
    fmi->loadFMU(fmu);

    cs = fmi->createCoSimulationObject(name);
    if (cs) {
        input_list = cs->getInputNames();
        output_list = cs->getOutputNames();
    }
}

FmiCoSimFederate::FmiCoSimFederate(const std::string& name,
                                   std::shared_ptr<fmi2CoSimObject> obj,
                                   const helics::FederateInfo& fi):
    fed(name, fi),
    cs(std::move(obj))
{
    fed = helics::ValueFederate(name, fi);
    if (cs) {
        input_list = cs->getInputNames();
        output_list = cs->getOutputNames();
    }
}

void FmiCoSimFederate::configure(helics::Time step, helics::Time startTime)
{
    timeBias = startTime;
    for (auto input : input_list) {
        inputs.emplace_back(&fed, input);
    }

    for (auto output : output_list) {
        pubs.emplace_back(&fed, output, helics::DataType::HELICS_DOUBLE);
    }

    auto& def = cs->fmuInformation().getExperiment();

    if (step <= helics::timeZero) {
        step = def.stepSize;
    }
    if (step <= helics::timeZero) {
        auto tstep = fed.getTimeProperty(HELICS_PROPERTY_TIME_PERIOD);
        step = (tstep > helics::timeEpsilon) ? tstep : helics::Time(0.2);
    }
    fed.setProperty(HELICS_PROPERTY_TIME_PERIOD, step);
    stepTime = step;
}

void FmiCoSimFederate::setInputs(std::vector<std::string> input_names)
{
    input_list = std::move(input_names);
}
void FmiCoSimFederate::setOutputs(std::vector<std::string> output_names)
{
    output_list = std::move(output_names);
}
void FmiCoSimFederate::setConnections(std::vector<std::string> conn)
{
    connections = std::move(conn);
}

void FmiCoSimFederate::addInput(const std::string& input_name)
{
    input_list.push_back(input_name);
}

void FmiCoSimFederate::addOutput(const std::string& output_name)
{
    output_list.push_back(output_name);
}

void FmiCoSimFederate::addConnection(const std::string& conn)
{
    connections.push_back(conn);
}

void FmiCoSimFederate::setOutputCapture(bool capture, const std::string& outputFile)
{
    if (!outputFile.empty()) {
        outputCaptureFile = outputFile;
    }
    captureOutput = capture;
}

void FmiCoSimFederate::run(helics::Time stop)
{
    auto& def = cs->fmuInformation().getExperiment();

    if (stop <= helics::timeZero) {
        stop = def.stopTime;
    }
    if (stop <= helics::timeZero) {
        stop = 30.0;
    }

    std::ofstream ofile;

    if (captureOutput) {
        ofile.open(outputCaptureFile);
    }
    fed.enterInitializingMode();
    cs->setupExperiment(
        false, 0, static_cast<double>(timeBias), 1, static_cast<double>(timeBias + stop));
    cs->setMode(fmuMode::initializationMode);
    std::vector<fmi2Real> outputs(pubs.size());
    if (captureOutput) {
        ofile << "time,";
        for (auto& pub : pubs) {
            ofile << pub.getName() << ",";
        }
        ofile << std::endl;
    }
    std::vector<fmi2Real> inp(inputs.size());
    if (!pubs.empty()) {
        cs->getOutputs(outputs.data());
        for (size_t ii = 0; ii < pubs.size(); ++ii) {
            pubs[ii].publish(outputs[ii]);
        }
    }
    if (!inp.empty()) {
        cs->getCurrentInputs(inp.data());
        for (size_t ii = 0; ii < inputs.size(); ++ii) {
            inputs[ii].setDefault(inp[ii]);
        }
    }
    auto result = fed.enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);
    if (result == helics::IterationResult::ITERATING) {
        if (!inputs.empty()) {
            for (size_t ii = 0; ii < inputs.size(); ++ii) {
                inp[ii] = inputs[ii].getValue<fmi2Real>();
            }
            cs->setInputs(inp.data());
        }
        fed.enterExecutingMode();
    }
    cs->setMode(fmuMode::stepMode);

    helics::Time currentTime = helics::timeZero;
    while (currentTime + timeBias <= stop) {
        cs->doStep(static_cast<double>(currentTime + timeBias),
                   static_cast<double>(stepTime),
                   true);
        currentTime = fed.requestNextStep();
        if (!outputs.empty()) {
            // get the values to publish
            cs->getOutputs(outputs.data());
            for (size_t ii = 0; ii < pubs.size(); ++ii) {
                pubs[ii].publish(outputs[ii]);
            }
        }
        if (!inputs.empty()) {
            // load the inputs
            for (size_t ii = 0; ii < inputs.size(); ++ii) {
                inp[ii] = inputs[ii].getValue<fmi2Real>();
            }
            cs->setInputs(inp.data());
        }
        if (captureOutput) {
            ofile << static_cast<double>(currentTime) << ",";
            for (auto& out : outputs) {
                ofile << out << ",";
            }
            ofile << std::endl;
        }
    }
    fed.finalize();
}
