/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "fmi/fmi_import/fmiImport.h"
#include "formatInterpreters/jsonReaderElement.h"
#include "formatInterpreters/tinyxml2ReaderElement.h"
#include "formatInterpreters/tomlReaderElement.h"
#include "gmlc/utilities/timeStringOps.hpp"
#include "helics-fmi/helics-fmi-config.h"
#include "helics/application_api/timeOperations.hpp"
#include "helics/apps/BrokerApp.hpp"
#include "helics/apps/CoreApp.hpp"
#include "helics/core/helicsVersion.hpp"
#include "helics/external/CLI11/CLI11.hpp"
#include "helicsFMI/FmiCoSimFederate.hpp"
#include "helicsFMI/FmiModelExchangeFederate.hpp"

#include <filesystem>
#include <iostream>
#include <thread>

void runSystem(readerElement& elem, helics::FederateInfo& fedInfo);

struct exeData {
    std::string inputFile;
    std::string integrator{"cvode"};
    std::string integratorArgs;
    std::string brokerArgs;
    helics::Time stepTime = helics::Time::minVal();
    helics::Time stopTime = helics::Time::minVal();
    std::vector<std::string> inputs;
    std::vector<std::string> output_variables;
    std::vector<std::string> input_variables;
    std::vector<std::string> connections;
    bool cosimFmu{true};
};

std::unique_ptr<CLI::App> generateCLI(exeData& data);

//NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, char* argv[])
{
    exeData configData;
    auto app = generateCLI(configData);
    try {
        app->parse(argc, argv);
    }
    catch (const CLI::CallForHelp& e) {
        auto ret = app->exit(e);
        // this is to trigger the help
        [[maybe_unused]] const helics::FederateInfo fedInfo(argc, argv);
        return ret;
    }
    catch (const CLI::ParseError& e) {
        return app->exit(e);
    }

    helics::FederateInfo fedInfo;
    // set the default core type to be local
    fedInfo.coreType = helics::CoreType::INPROC;
    fedInfo.defName = "fmu${#}";

    auto remArgs = app->remaining_for_passthrough();
    fedInfo.separator = '.';
    try
    {
        fedInfo.loadInfoFromArgs(remArgs);
    }
    catch(const CLI::ParseError& e) {
       return app->exit(e);
    }
    // this chunk of code is to ease errors on extra args
    if (!remArgs.empty()) {
        app->allow_extras(false);
        try {
            app->parse(remArgs);
        }
        catch (const CLI::ParseError& e) {
            return app->exit(e);
        }
    }
    std::unique_ptr<helics::apps::BrokerApp> broker;
    if (fedInfo.autobroker) {
        try {
            broker =
                std::make_unique<helics::apps::BrokerApp>(fedInfo.coreType, configData.brokerArgs);
            if (!broker->isConnected()) {
                broker->connect();
            }
        }
        catch (const std::exception& e) {
            std::cerr << "error generator broker :" << e.what() << std::endl;
            return -101;
        }
    }
    fedInfo.autobroker = false;
    if (configData.inputFile.empty()) {
        configData.inputFile = configData.inputs.front();
    }
    fedInfo.forceNewCore = true;
    auto ext = configData.inputFile.substr(configData.inputFile.find_last_of('.'));

    FmiLibrary fmi;
    if ((ext == ".fmu") || (ext == ".FMU")) {
        try {
            fmi.loadFMU(configData.inputFile);
            if (configData.cosimFmu && fmi.checkFlag(fmuCapabilityFlags::coSimulationCapable)) {
                std::shared_ptr<fmi2CoSimObject> obj = fmi.createCoSimulationObject("obj1");
                auto fed = std::make_unique<FmiCoSimFederate>("", std::move(obj), fedInfo);
                fed->configure(configData.stepTime);
                fed->run(configData.stopTime);
            } else {
                std::shared_ptr<fmi2ModelExchangeObject> obj =
                    fmi.createModelExchangeObject("obj1");
                auto fed = std::make_unique<FmiModelExchangeFederate>(std::move(obj), fedInfo);
                fed->configure(configData.stepTime);
                fed->run(configData.stopTime);
            }
        }
        catch (const std::exception& e) {
            if (broker) {
                broker->forceTerminate();
            }
            std::cout << "error running fmu: " << e.what() << std::endl;
            return -101;
        }
    } else if ((ext == ".json") || (ext == ".JSON")) {
        jsonReaderElement system(configData.inputFile);
        if (system.isValid()) {
            try {
                runSystem(system, fedInfo);
            }
            catch (const std::exception& e) {
                if (broker) {
                    broker->forceTerminate();
                }

                std::cout << "error running system " << e.what() << std::endl;
                return -101;
            }
        } else if (broker) {
            broker->forceTerminate();
        }
    } else if ((ext == ".toml") || (ext == ".TOML")) {
        tomlReaderElement system(configData.inputFile);
        if (system.isValid()) {
            try {
                runSystem(system, fedInfo);
            }
            catch (const std::exception& e) {
                if (broker) {
                    broker->forceTerminate();
                }
                std::cout << "error running system " << e.what() << std::endl;
                return -101;
            }
        } else if (broker) {
            broker->forceTerminate();
        }
    } else if ((ext == ".xml") || (ext == ".XML")) {
        tinyxml2ReaderElement system(configData.inputFile);
        if (system.isValid()) {
            try {
                runSystem(system, fedInfo);
            }
            catch (const std::exception& e) {
                if (broker) {
                    broker->forceTerminate();
                }
                std::cout << "error running system " << e.what() << std::endl;
                return -101;
            }
        } else if (broker) {
            broker->forceTerminate();
        }
    }
    if (broker) {
        broker->waitForDisconnect();
    }
    return 0;
}

std::unique_ptr<CLI::App> generateCLI(exeData& data)
{
    auto app = std::make_unique<CLI::App>("HELICS-FMI for loading and executing FMU's with HELICS",
                                          "helics-fmi");
    app->add_flag_function(
        "-v,--version",
        [](size_t) {
            std::cout << "HELICS VERSION " << helics::versionString << '\n';
            std::cout << "HELICS_FMI_VERSION " << HELICS_FMI_VERSION_STRING << '\n';
            throw(CLI::Success());
        },
        "specify the versions of helics and helics-fmi");
    app->validate_positionals();

    app->add_option("--integrator",
                    data.integrator,
                    "the type of integrator to use(cvode, arkode, boost)")
        ->capture_default_str()
        ->transform(CLI::IsMember({"cvode", "arkode", "boost"}));
    auto* input_group = app->add_option_group("input files")->required();

    input_group->add_option("inputfile", data.inputFile, "specify the input files")
        ->check(CLI::ExistingFile);
    input_group->add_option("-i,--input", data.inputs, "specify the input files")
        ->check(CLI::ExistingFile);
    app->add_option("--integrator-args",
                    data.integratorArgs,
                    "arguments to pass to the integrator");

    app->add_option("--step",
                    data.stepTime,
                    "the step size to use (specified in seconds or as a time string (10ms)");
    app->add_option(
        "--stop",
        data.stopTime,
        "the time to stop the simulation (specified in seconds or as a time string (10ms)");

    app->add_option("--brokerargs",
                    data.brokerArgs,
                    "arguments to pass to an automatically generated broker");
    app->set_help_flag("-h,-?,--help", "print this help module");
    app->allow_extras();
    app->set_config("--config-file");

    app->add_option("--output_variables",
                    data.output_variables,
                    "Specify outputs of the FMU by name")
        ->ignore_underscore()
        ->delimiter(',');
    app->add_option("--input_variables",
                    data.input_variables,
                    "Specify the input variables of the FMU by name")
        ->ignore_underscore()
        ->delimiter(',');
    app->add_option("--connections", data.connections, "Specify connections this FMU should make")
        ->delimiter(',');

    app->add_flag("--cosim",
                  data.cosimFmu,
                  "specify that the fmu should run as a co-sim FMU if possible");
    app->add_flag("!--modelexchange",
                  data.cosimFmu,
                  "specify that the fmu should run as a model exchange FMU if possible");
    return app;
}

void runSystem(readerElement& elem, helics::FederateInfo& fedInfo)
{
    helics::Time stopTime = helics::timeZero;
    fedInfo.coreName = "fmu_core";
    if (elem.hasAttribute("stoptime")) {
        stopTime = elem.getAttributeValue("stoptime");
    }
    elem.moveToFirstChild("fmus");
    std::vector<std::unique_ptr<FmiCoSimFederate>> feds_cs;
    std::vector<std::unique_ptr<FmiModelExchangeFederate>> feds_me;
    auto core = helics::CoreApp(fedInfo.coreType,
                                "--name=fmu_core " + helics::generateFullCoreInitString(fedInfo));
    std::vector<std::unique_ptr<FmiLibrary>> fmis;
    while (elem.isValid()) {
        auto fmilib = std::make_unique<FmiLibrary>();
        auto str = elem.getAttributeText("fmu");
        fmilib->loadFMU(str);
        if (fmilib->checkFlag(fmuCapabilityFlags::coSimulationCapable)) {
            std::shared_ptr<fmi2CoSimObject> obj =
                fmilib->createCoSimulationObject(elem.getAttributeText("name"));
            auto fed = std::make_unique<FmiCoSimFederate>(obj->getName(), std::move(obj), fedInfo);
            elem.moveToFirstChild("parameters");
            while (elem.isValid()) {
                const auto& str1 = elem.getFirstAttribute().getText();
                auto attr = elem.getNextAttribute();

                elem.moveToNextSibling("parameters");
                const double val = attr.getValue();
                if (val != readerNullVal) {
                    fed->set(str1, val);
                } else {
                    fed->set(str1, attr.getText());
                }
            }
            elem.moveToParent();
            if (elem.hasAttribute("starttime")) {
                fed->configure(1.0, elem.getAttributeValue("starttime"));
            } else {
                fed->configure(1.0);
            }
            feds_cs.push_back(std::move(fed));
        } else {
            std::shared_ptr<fmi2ModelExchangeObject> obj =
                fmilib->createModelExchangeObject(elem.getAttributeText("name"));
            auto fed = std::make_unique<FmiModelExchangeFederate>(std::move(obj), fedInfo);
            elem.moveToFirstChild("parameters");
            while (elem.isValid()) {
                auto str1 = elem.getFirstAttribute().getText();
                auto str2 = elem.getNextAttribute().getText();
                elem.moveToNextSibling("parameters");
                fed->set(str1, str2);
            }
            elem.moveToParent();
            fed->configure(1.0);
            feds_me.push_back(std::move(fed));
        }
        fmis.push_back(std::move(fmilib));
        elem.moveToNextSibling("fmus");
    }
    elem.moveToParent();
    elem.moveToFirstChild("connections");
    while (elem.isValid()) {
        auto str1 = elem.getFirstAttribute().getText();
        auto str2 = elem.getNextAttribute().getText();
        elem.moveToNextSibling("connections");
        core.dataLink(str1, str2);
    }
    elem.moveToParent();
    // load each of the fmu's into its own thread
    std::vector<std::thread> threads(feds_cs.size() + feds_me.size());
    for (size_t ii = 0; ii < feds_cs.size(); ++ii) {
        auto* tfed = feds_cs[ii].get();
        threads[ii] = std::thread([tfed, stopTime]() { tfed->run(stopTime); });
    }
    for (size_t jj = 0; jj < feds_me.size(); ++jj) {
        auto* tfed = feds_me[jj].get();
        threads[jj + feds_cs.size()] = std::thread([tfed, stopTime]() { tfed->run(stopTime); });
    }
    for (auto& thread : threads) {
        thread.join();
    }
    feds_cs.clear();
    feds_me.clear();
    core.forceTerminate();
}
