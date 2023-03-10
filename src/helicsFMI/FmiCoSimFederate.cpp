/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "FmiCoSimFederate.hpp"

#include "FmiHelics.hpp"
#include "fmi/fmi_import/fmiObjects.h"
#include "gmlc/utilities/stringConversion.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <utility>

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
FmiCoSimFederate::FmiCoSimFederate(const std::string& name,
                                   const std::string& fmu,
                                   const helics::FederateInfo& fedInfo):
    fed(name, fedInfo)
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
                                   const helics::FederateInfo& fedInfo)
try : fed(name, fedInfo), cs(std::move(obj)) {
    if (cs) {
        input_list = cs->getInputNames();
        output_list = cs->getOutputNames();
    }
}
catch (const std::exception& e) {
    std::cout << "error in constructor of federate:" << e.what() << std::endl;
    throw;
}

void FmiCoSimFederate::configure(helics::Time step, helics::Time startTime)
{
    timeBias = startTime;
    for (const auto& input : input_list) {
        const auto& inputInfo = cs->addInputVariable(input);
        if (inputInfo.index >= 0) {
            auto iType = helicsfmi::getHelicsType(inputInfo.type);
            inputs.emplace_back(&fed, input, iType);
        } else {
            fed.logWarningMessage(input + " is not a recognized input");
        }
    }

    for (const auto& output : output_list) {
        const auto& outputInfo = cs->addOutputVariable(output);
        if (outputInfo.index >= 0) {
            auto iType = helicsfmi::getHelicsType(outputInfo.type);
            pubs.emplace_back(&fed, output, iType);
        } else {
            fed.logWarningMessage(output + " is not a recognized output");
        }
    }

    const auto& def = cs->fmuInformation().getExperiment();

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

bool FmiCoSimFederate::setDouble(const std::string& parameter, double value)
{
    cs->set(parameter, value);
    return true;
}

bool FmiCoSimFederate::setInteger(const std::string& parameter, int64_t value)
{
    cs->set(parameter, value);
    return true;
}

void FmiCoSimFederate::runCommand(const std::string& command)
{
    auto cvec = gmlc::utilities::stringOps::splitlineQuotes(
        command, " ,;:", "\"'`", gmlc::utilities::stringOps::delimiter_compression::on);
    if (cvec[0] == "set") {
        auto val =
            gmlc::utilities::numeric_conversionComplete<double>(cvec[2], helics::invalidDouble);
        if (val != helics::invalidDouble) {
            if (std::round(val) == val) {
                setInteger(cvec[1], static_cast<int64_t>(val));
                return;
            }
            setDouble(cvec[1], val);
            return;
        }
        cs->set(cvec[1], cvec[2]);
        return;
    }
}

double FmiCoSimFederate::initialize(double stop, std::ofstream& ofile)
{
    const auto& def = cs->fmuInformation().getExperiment();

    if (stop <= helics::timeZero) {
        stop = def.stopTime;
    }
    if (stop <= helics::timeZero) {
        stop = 30.0;
    }

    cs->setupExperiment(
        fmi2False, 0, static_cast<double>(timeBias), 1, static_cast<double>(timeBias + stop));
    auto cmd = fed.getCommand();
    while (!cmd.first.empty()) {
        runCommand(cmd.first);
        cmd = fed.getCommand();
    }
    fed.enterInitializingMode();
    cs->setMode(fmuMode::initializationMode);
    if (captureOutput) {
        ofile << "time,";
        for (auto& pub : pubs) {
            ofile << pub.getName() << ",";
        }
        ofile << std::endl;
    }
    if (!pubs.empty()) {
        for (std::size_t ii = 0; ii < pubs.size(); ++ii) {
            helicsfmi::publishOutput(pubs[ii], cs.get(), ii);
        }
    }
    if (!inputs.empty()) {
        for (std::size_t ii = 0; ii < inputs.size(); ++ii) {
            helicsfmi::setDefault(inputs[ii], cs.get(), ii);
        }
    }
    return stop;
}

void FmiCoSimFederate::run(helics::Time stop)
{
    std::ofstream ofile;

    if (captureOutput) {
        ofile.open(outputCaptureFile);
    }

    stop = initialize(stop, ofile);

    auto result = fed.enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);
    if (result == helics::IterationResult::ITERATING) {
        if (!inputs.empty()) {
            for (std::size_t ii = 0; ii < inputs.size(); ++ii) {
                helicsfmi::grabInput(inputs[ii], cs.get(), ii);
            }
        }
        fed.enterExecutingMode();
    }
    cs->setMode(fmuMode::stepMode);

    helics::Time currentTime = helics::timeZero;
    while (currentTime + timeBias <= stop) {
        cs->doStep(static_cast<double>(currentTime + timeBias),
                   static_cast<double>(stepTime),
                   fmi2True);
        currentTime = fed.requestNextStep();
        if (!pubs.empty()) {
            // get the values to publish
            for (std::size_t ii = 0; ii < pubs.size(); ++ii) {
                helicsfmi::publishOutput(pubs[ii], cs.get(), ii);
            }
        }
        if (!inputs.empty()) {
            // load the inputs
            for (std::size_t ii = 0; ii < inputs.size(); ++ii) {
                helicsfmi::grabInput(inputs[ii], cs.get(), ii);
            }
        }
        /* if (captureOutput) {
             ofile << static_cast<double>(currentTime) << ",";
             for (auto& out : outputs) {
                 ofile << out << ",";
             }
             ofile << std::endl;
         }
         */
    }
    fed.finalize();
}
