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
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <utility>

namespace helicsfmi {
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
CoSimFederate::CoSimFederate(const std::string& name,
                             const std::string& fmu,
                             const helics::FederateInfo& fedInfo):
    fed(name, fedInfo)
{
    auto fmi = std::make_shared<FmiLibrary>();
    if (!fmi->loadFMU(fmu)) {
        throw(Error("CoSimFederate", "unable to load FMU", -101));
    }

    cs = fmi->createCoSimulationObject(name);
    loadFMUInformation();
}

CoSimFederate::CoSimFederate(const std::string& name,
                             std::shared_ptr<fmi2CoSimObject> obj,
                             const helics::FederateInfo& fedInfo)
try : fed(name, fedInfo), cs(std::move(obj)) {
    loadFMUInformation();
}
catch (const std::exception& e) {
    std::cerr << "error in constructor of federate:" << e.what() << std::endl;
    throw;
}

CoSimFederate::CoSimFederate(const std::string& name,
                             helics::CoreApp& core,
                             const std::string& fmu,
                             const helics::FederateInfo& fedInfo):
    fed(name, core, fedInfo)
{
    auto fmi = std::make_shared<FmiLibrary>();
    if (!fmi->loadFMU(fmu)) {
        throw(Error("CoSimFederate", "unable to load FMU", -101));
    }

    cs = fmi->createCoSimulationObject(name);
    loadFMUInformation();
}

CoSimFederate::CoSimFederate(const std::string& name,
                             std::shared_ptr<fmi2CoSimObject> obj,
                             helics::CoreApp& core,
                             const helics::FederateInfo& fedInfo)
try : fed(name, core, fedInfo), cs(std::move(obj)) {
    loadFMUInformation();
}
catch (const std::exception& e) {
    std::cerr << "error in constructor of federate2:" << e.what() << std::endl;
    throw;
}

void CoSimFederate::loadFMUInformation()
{
    if (cs) {
        cs->getLogger()->setLoggerCallback(
            [this](std::string_view category, std::string_view message) {
                fed.logMessage(fmiCategory2HelicsLogLevel(category), message);
            });
        input_list = cs->getInputNames();
        output_list = cs->getOutputNames();
    }
}
void CoSimFederate::configure(helics::Time step, helics::Time startTime)
{
    timeBias = startTime;
    logLevel = fed.getIntegerProperty(HELICS_PROPERTY_INT_LOG_LEVEL);
    for (const auto& input : input_list) {
        const auto& inputInfo = cs->addInputVariable(input);
        if (inputInfo.index >= 0) {
            auto iType = helicsfmi::getHelicsType(inputInfo.type);
            inputs.emplace_back(&fed, input, iType);
            LOG_INTERFACES(fmt::format("created input {}", inputs.back().getName()));
        } else {
            fed.logWarningMessage(input + " is not a recognized input");
        }
    }

    for (const auto& output : output_list) {
        const auto& outputInfo = cs->addOutputVariable(output);
        if (outputInfo.index >= 0) {
            auto iType = helicsfmi::getHelicsType(outputInfo.type);
            pubs.emplace_back(&fed, output, iType);
            LOG_INTERFACES(fmt::format("created publication {}", pubs.back().getName()));
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
    LOG_SUMMARY(fmt::format("\n  co sim federate:\n\t{} inputs\n\t{} publications\n\tstep size={}",
                            inputs.size(),
                            pubs.size(),
                            static_cast<double>(stepTime)));
}

void CoSimFederate::setInputs(std::vector<std::string> input_names)
{
    input_list = std::move(input_names);
}
void CoSimFederate::setOutputs(std::vector<std::string> output_names)
{
    output_list = std::move(output_names);
}
void CoSimFederate::setConnections(std::vector<std::string> conn)
{
    connections = std::move(conn);
}

void CoSimFederate::addInput(const std::string& input_name)
{
    input_list.push_back(input_name);
}

void CoSimFederate::addOutput(const std::string& output_name)
{
    output_list.push_back(output_name);
}

void CoSimFederate::addConnection(const std::string& conn)
{
    connections.push_back(conn);
}

void CoSimFederate::setOutputCapture(bool capture, const std::string& outputFile)
{
    if (!outputFile.empty()) {
        outputCaptureFile = outputFile;
    }
    captureOutput = capture;
}

void CoSimFederate::runCommand(const std::string& command)
{
    auto cvec = gmlc::utilities::stringOps::splitlineQuotes(
        command, " ,;:", "\"'`", gmlc::utilities::stringOps::delimiter_compression::on);
    if (cvec[0] == "set") {
        LOG_DATA_MESSAGES(fmt::format("set command {}={}", cvec[1], cvec[2]));
        auto val =
            gmlc::utilities::numeric_conversionComplete<double>(cvec[2], helics::invalidDouble);
        if (val != helics::invalidDouble) {
            if (std::round(val) == val) {
                set(cvec[1], static_cast<int64_t>(val));
                return;
            }
            set(cvec[1], val);
            return;
        }
        cs->set(cvec[1], cvec[2]);
        return;
    }
}

double CoSimFederate::initialize(double stop, std::ofstream& ofile)
{
    const auto& def = cs->fmuInformation().getExperiment();

    if (stop <= helics::timeZero) {
        stop = def.stopTime;
    }
    if (stop <= helics::timeZero) {
        stop = 30.0;
    }

    cs->setupExperiment(
        false, 0, static_cast<double>(timeBias), true, static_cast<double>(timeBias + stop));
    auto cmd = fed.getCommand();
    while (!cmd.first.empty()) {
        runCommand(cmd.first);
        cmd = fed.getCommand();
    }
    fed.enterInitializingMode();
    cs->setMode(FmuMode::INITIALIZATION);
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
    LOG_TIMING("initializing");
    return stop;
}

bool CoSimFederate::setFlag(const std::string& flag, bool val)
{
    if (cs->setFlag(flag, val)) {
        return true;
    }
    const int param = helics::getFlagIndex(flag);
    if (param != HELICS_INVALID_OPTION_INDEX) {
        fed.setFlagOption(param, val);
        return true;
    }
    return false;
}

void CoSimFederate::logMessage(int helicsLogLevel, std::string_view message)
{
    fed.logMessage(helicsLogLevel, message);
}

void CoSimFederate::run(helics::Time stop)
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
    cs->setMode(FmuMode::STEP);

    helics::Time currentTime = helics::timeZero;
    while (currentTime + timeBias + stepTime <= stop) {
        try {
            cs->doStep(static_cast<double>(currentTime + timeBias),
                       static_cast<double>(stepTime),
                       fmi2True);
        }
        catch (const fmiException& fe) {
            fed.localError(56, fe.what());
            break;
        }
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
}  // namespace helicsfmi
