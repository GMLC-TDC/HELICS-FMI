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

void runSystem(readerElement& elem, helics::FederateInfo& fi);

int main(int argc, char* argv[])
{
    std::ifstream infile;
    CLI::App app{"HELICS-FMI for loading and executing FMU's with HELICS", "helics-fmi"};
    app.add_flag_function(
        "-v,--version",
        [](size_t) {
            std::cout << "HELICS VERSION " << helics::versionString << '\n';
            std::cout << "HELICS_FMI_VERSION " << HELICS_FMI_VERSION_STRING << '\n';
            throw(CLI::Success());
        },
        "specify the versions of helics and helics-fmi");
    app.validate_positionals();
    std::string integrator{"cvode"};
    app.add_option("--integrator",
                   integrator,
                   "the type of integrator to use(cvode, arkode, boost)")
        ->capture_default_str()
        ->transform(CLI::IsMember({"cvode", "arkode", "boost"}));
    auto input_group = app.add_option_group("input files")->required();
    std::string inputFile;
    input_group->add_option("inputfile", inputFile, "specify the input files")
        ->check(CLI::ExistingFile);
    std::vector<std::string> inputs;
    input_group->add_option("-i,--input", inputs, "specify the input files")
        ->check(CLI::ExistingFile);
    std::string integratorArgs;
    app.add_option("--integrator-args", integratorArgs, "arguments to pass to the integrator");

    std::string stepTimeString;
    std::string stopTimeString;

    app.add_option("--step",
                   stepTimeString,
                   "the step size to use (specified in seconds or as a time string (10ms)");
    app.add_option(
        "--stop",
        stopTimeString,
        "the time to stop the simulation (specified in seconds or as a time string (10ms)");

    std::string brokerArgs;
    app.add_option("--brokerargs",
                   brokerArgs,
                   "arguments to pass to an automatically generated broker");
    app.set_help_flag("-h,-?,--help", "print this help module");
    app.allow_extras();
    app.set_config("--config-file");
    std::vector<std::string> output_variables;
    std::vector<std::string> input_variables;
    std::vector<std::string> connections;

    app.add_option("--output_variables", output_variables, "Specify outputs of the FMU by name")
        ->ignore_underscore()
        ->delimiter(',');
    app.add_option("--input_variables",
                   input_variables,
                   "Specify the input variables of the FMU by name")
        ->ignore_underscore()
        ->delimiter(',');
    app.add_option("--connections", input_variables, "Specify connections this FMU should make")
        ->delimiter(',');

    bool cosimFmu{true};
    app.add_flag("--cosim",cosimFmu,"specify that the fmu should run as a co-sim FMU if possible");
    app.add_flag("!--modelexchange",cosimFmu,"specify that the fmu should run as a model exchange FMU if possible");
    try {
        app.parse(argc, argv);
    }
    catch (const CLI::CallForHelp& e) {
        auto ret = app.exit(e);
        helics::FederateInfo fi(argc, argv);
        return ret;
    }
    catch (const CLI::ParseError& e) {
        return app.exit(e);
    }
    helics::Time stepTime = helics::Time::minVal();
    if (!stepTimeString.empty()) {
        stepTime = gmlc::utilities::loadTimeFromString<helics::Time>(stepTimeString);
    }
    helics::Time stopTime = helics::Time::minVal();
    if (!stopTimeString.empty()) {
        stopTime = gmlc::utilities::loadTimeFromString<helics::Time>(stopTimeString);
    }

    helics::FederateInfo fi;
    // set the default core type to be local
    fi.coreType = helics::CoreType::INPROC;
    fi.defName = "fmu${#}";
    auto remArgs = app.remaining_for_passthrough();
    fi.separator = '.';
    fi.loadInfoFromArgs(remArgs);
    //this chunk of code is to ease errors on extra args
    if (!remArgs.empty()) {
        app.allow_extras(false);
        try {
            app.parse(remArgs);
        }
        catch (const CLI::ParseError& e) {
            return app.exit(e);
        }
    }
    std::unique_ptr<helics::apps::BrokerApp> broker;
    if (fi.autobroker) {
        try
        {
            broker = std::make_unique<helics::apps::BrokerApp>(fi.coreType, brokerArgs);
        }
        catch (const  std::exception& e)
        {
            std::cerr<<"error generator broker :"<<e.what()<<std::endl;
            return -101;
        }
    }
    fi.autobroker = false;
    if (inputFile.empty()) {
        inputFile = inputs.front();
    }

    auto ext = inputFile.substr(inputFile.find_last_of('.'));

    FmiLibrary fmi;
    if ((ext == ".fmu") || (ext == ".FMU")) {
        try
        {
            fmi.loadFMU(inputFile);
            if (cosimFmu && fmi.checkFlag(fmuCapabilityFlags::coSimulationCapable)) {
                std::shared_ptr<fmi2CoSimObject> obj = fmi.createCoSimulationObject("obj1");
                auto fed = std::make_unique<FmiCoSimFederate>("", obj, fi);
                fed->configure(stepTime);
                fed->run(stopTime);
            } else {
                std::shared_ptr<fmi2ModelExchangeObject> obj = fmi.createModelExchangeObject("obj1");
                auto fed = std::make_unique<FmiModelExchangeFederate>(obj, fi);
                fed->configure(stepTime);
                fed->run(stopTime);
            }
        }
        catch (const std::exception& e)
        {
            if (broker)
            {
                broker->forceTerminate();
            }
            std::cout<<"error running system "<<e.what()<<std::endl;
            return -101;
        }
    } else if ((ext == ".json") || (ext == ".JSON")) {
        jsonReaderElement system(inputFile);
        if (system.isValid()) {
            try {
                runSystem(system, fi);
            }
            catch (const std::exception& e)
            {
                if (broker)
                {
                    broker->forceTerminate();
                }
                
                std::cout<<"error running system "<<e.what()<<std::endl;
                return -101;
            }
        } else if (broker) {
            broker->forceTerminate();
        }
    } else if ((ext == ".toml") || (ext == ".TOML")) {
        tomlReaderElement system(inputFile);
        if (system.isValid()) {
            try {
                runSystem(system, fi);
            }
            catch (const std::exception& e)
            {
                if (broker)
                {
                    broker->forceTerminate();
                }
                std::cout<<"error running system "<<e.what()<<std::endl;
                return -101;
            }
        } else if (broker) {
            broker->forceTerminate();
        }
    } else if ((ext == ".xml") || (ext == ".XML")) {
        tinyxml2ReaderElement system(inputFile);
        if (system.isValid()) {
            try {
                runSystem(system, fi);
            }
            catch (const std::exception& e)
            {
                if (broker)
                {
                    broker->forceTerminate();
                }
                std::cout<<"error running system "<<e.what()<<std::endl;
                return -101;
            }
        } else if (broker) {
            broker->forceTerminate();
        }
    }
    if (broker)
    {
        broker->waitForDisconnect();
    }
    return 0;
}

void runSystem(readerElement& elem, helics::FederateInfo& fi)
{
    helics::Time stopTime = helics::timeZero;
    fi.coreName = "fmu_core";
    if (elem.hasAttribute("stoptime")) {
        double sval = elem.getAttributeValue("stoptime");
        stopTime = sval;
    }
    elem.moveToFirstChild("fmus");
    std::vector<std::unique_ptr<FmiCoSimFederate>> feds_cs;
    std::vector<std::unique_ptr<FmiModelExchangeFederate>> feds_me;
    auto core =
        helics::CoreApp(fi.coreType, "--name=fmu_core " + helics::generateFullCoreInitString(fi));
    std::vector<std::unique_ptr<FmiLibrary>> fmis;
    while (elem.isValid()) {
        auto fmilib = std::make_unique<FmiLibrary>();
        auto str = elem.getAttributeText("fmu");
        fmilib->loadFMU(str);
        if (fmilib->checkFlag(fmuCapabilityFlags::coSimulationCapable)) {
            std::shared_ptr<fmi2CoSimObject> obj =
                fmilib->createCoSimulationObject(elem.getAttributeText("name"));
            auto fed = std::make_unique<FmiCoSimFederate>(obj->getName(), obj, fi);
            elem.moveToFirstChild("parameters");
            while (elem.isValid()) {
                auto str1 = elem.getFirstAttribute().getText();
                auto attr = elem.getNextAttribute();

                elem.moveToNextSibling("parameters");
                double val = attr.getValue();
                if (val != readerNullVal) {
                    obj->set(str1, val);
                } else {
                    const auto& str2 = attr.getText();
                    obj->set(str1, str2);
                }
            }
            elem.moveToParent();
            if (elem.hasAttribute("starttime")) {
                double sval = elem.getAttributeValue("starttime");
                fed->configure(1.0, sval);
            } else {
                fed->configure(1.0);
            }
            feds_cs.push_back(std::move(fed));
        } else {
            std::shared_ptr<fmi2ModelExchangeObject> obj =
                fmilib->createModelExchangeObject(elem.getAttributeText("name"));
            auto fed = std::make_unique<FmiModelExchangeFederate>(obj, fi);
            elem.moveToFirstChild("parameters");
            while (elem.isValid()) {
                auto str1 = elem.getFirstAttribute().getText();
                auto str2 = elem.getNextAttribute().getText();
                elem.moveToNextSibling("parameters");
                obj->set(str1, str2);
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
        auto tfed = feds_cs[ii].get();
        threads[ii] = std::thread([tfed, stopTime]() { tfed->run(stopTime); });
    }
    for (size_t jj = 0; jj < feds_me.size(); ++jj) {
        auto tfed = feds_me[jj].get();
        threads[jj + feds_cs.size()] = std::thread([tfed, stopTime]() { tfed->run(stopTime); });
    }
    for (auto& thread : threads) {
        thread.join();
    }
    feds_cs.clear();
    feds_me.clear();
    core.forceTerminate();
}
