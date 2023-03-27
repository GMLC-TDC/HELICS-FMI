/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "helicsFmiRunner.hpp"

#include "fmi/fmi_import/fmiImport.h"
#include "formatInterpreters/jsonReaderElement.h"
#include "formatInterpreters/tinyxml2ReaderElement.h"
#include "formatInterpreters/tomlReaderElement.h"
#include "gmlc/utilities/timeStringOps.hpp"
#include "helics-fmi/helics-fmi-config.h"
#include "helics/apps/CoreApp.hpp"
#include "helics/core/helicsVersion.hpp"

#include <filesystem>
#include <iostream>
#include <thread>

namespace helicsfmi {

FmiRunner::FmiRunner()
{
    fedInfo.defName = "fmu${#}";
    fedInfo.separator = '.';
}

std::unique_ptr<CLI::App> FmiRunner::generateCLI()
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
                    integrator,
                    "the type of integrator to use(cvode, arkode, boost)")
        ->capture_default_str()
        ->transform(CLI::IsMember({"cvode", "arkode", "boost"}));

    app->add_option("inputfile,-i,--input", inputs, "specify the input files")
        ->check(CLI::ExistingFile)
        ->required();

    app->add_option("--integrator-args", integratorArgs, "arguments to pass to the integrator");

    app->add_option("--step",
                    stepTime,
                    "the step size to use (specified in seconds or as a time string (10ms)");
    app->add_option(
        "--stop",
        stopTime,
        "the time to stop the simulation (specified in seconds or as a time string (10ms)");

    app->add_option("--brokerargs",
                    brokerArgs,
                    "arguments to pass to an automatically generated broker");
    app->set_help_flag("-h,-?,--help", "print this help module");
    app->allow_extras();
    app->set_config("--config-file");

    app->add_option("--output_variables", output_variables, "Specify outputs of the FMU by name")
        ->ignore_underscore()
        ->delimiter(',');
    app->add_option("--input_variables",
                    input_variables,
                    "Specify the input variables of the FMU by name")
        ->ignore_underscore()
        ->delimiter(',');
    app->add_option("--connections", connections, "Specify connections this FMU should make")
        ->delimiter(',');

    app->add_flag("--cosim",
                  cosimFmu,
                  "specify that the fmu should run as a co-sim FMU if possible");
    app->add_flag("!--modelexchange",
                  cosimFmu,
                  "specify that the fmu should run as a model exchange FMU if possible");

    fedInfo.injectParser(app.get());

    return app;
}

int FmiRunner::parse(const std::string& cliString)
{
    currentState = State::CREATED;
    auto app = generateCLI();
    try {
        app->parse(cliString);
        return EXIT_SUCCESS;
    }
    catch (const CLI::Error& e) {
        returnCode = app->exit(e);
        if (returnCode != static_cast<int>(CLI::ExitCodes::Success)) {
            currentState = State::ERROR;
        }
        return returnCode;
    }
}

int FmiRunner::load()
{
    if (currentState >= State::LOADED) {
        return (currentState == State::ERROR) ? returnCode : EXIT_SUCCESS;
    }
    if (fedInfo.autobroker) {
        try {
            std::string args = brokerArgs;
            if (!fedInfo.brokerInitString.empty()) {
                if (!args.empty()) {
                    args.push_back(' ');
                }
                args.append(fedInfo.brokerInitString);
                fedInfo.brokerInitString.clear();
            }
            broker = std::make_unique<helics::apps::BrokerApp>(fedInfo.coreType, args);
            if (!broker->isConnected()) {
                if (!broker->connect()) {
                    std::cerr << "broker failed to connect" << std::endl;
                    return errorTerminate(BROKER_CONNECT_FAILURE);
                }
            }

            std::cout << "started autobroker with args \"" << args << "\"" << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "error generating broker :" << e.what() << std::endl;
            return errorTerminate(BROKER_CONNECT_FAILURE);
        }
    }
    fedInfo.autobroker = false;

    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, stepTime);

    if (broker) {
        fedInfo.brokerPort = -1;
        fedInfo.broker = broker->getAddress();
    }
    std::cout << "starting core with args " << helics::generateFullCoreInitString(fedInfo)
              << std::endl;
    core = std::make_unique<helics::CoreApp>(fedInfo.coreType,
                                             helics::generateFullCoreInitString(fedInfo));
    if (!core->isConnected()) {
        if (!core->connect()) {
            core.reset();
            std::cerr << "core failed to connect" << std::endl;
            return errorTerminate(CORE_CONNECT_FAILURE);
        }
    }
    fedInfo.coreName = core->getIdentifier();
    if (inputs.empty()) {
        currentState = State::ERROR;
        return -203;
    }
    const std::string &inputFile = inputs.front();
    auto ext = inputFile.substr(inputFile.find_last_of('.'));

    FmiLibrary fmi;
    if ((ext == ".fmu") || (ext == ".FMU")) {
        try {
            if (!fmi.loadFMU(inputFile)) {
                std::cout << "error loading fmu: error code=" << fmi.getErrorCode() << std::endl;
                return errorTerminate(INVALID_FMU);
            }
            if (cosimFmu && fmi.checkFlag(fmuCapabilityFlags::coSimulationCapable)) {
                std::shared_ptr<fmi2CoSimObject> obj = fmi.createCoSimulationObject("obj1");
                if (!obj) {
                    std::cout << "unable to create cosim object " << std::endl;
                    return errorTerminate(FMU_ERROR);
                }
                auto fed = std::make_unique<CoSimFederate>("", std::move(obj), fedInfo);
                cosimFeds.push_back(std::move(fed));
            } else {
                std::shared_ptr<fmi2ModelExchangeObject> obj =
                    fmi.createModelExchangeObject("obj1");
                if (!obj) {
                    std::cout << "unable to create model exchange object " << std::endl;
                    return errorTerminate(FMU_ERROR);
                }
                auto fed = std::make_unique<FmiModelExchangeFederate>(std::move(obj), fedInfo);
                meFeds.push_back(std::move(fed));
            }
        }
        catch (const std::exception& e) {
            std::cout << "error running fmu: " << e.what() << std::endl;
            return errorTerminate(FMU_ERROR);
        }
    } else if ((ext == ".json") || (ext == ".JSON")) {
        jsonReaderElement system(inputFile);
        if (system.isValid()) {
            try {
                loadFile(system);
            }
            catch (const std::exception& e) {
                std::cout << "error running system " << e.what() << std::endl;
                return errorTerminate(FILE_PROCESSING_ERROR);
            }
        } else if (broker) {
            return errorTerminate(FILE_PROCESSING_ERROR);
        }
    } else if ((ext == ".toml") || (ext == ".TOML")) {
        tomlReaderElement system(inputFile);
        if (system.isValid()) {
            try {
                loadFile(system);
            }
            catch (const std::exception& e) {
                std::cout << "error running system " << e.what() << std::endl;
                return errorTerminate(FILE_PROCESSING_ERROR);
            }
        } else if (broker) {
            broker->forceTerminate();
        }
    } else if ((ext == ".xml") || (ext == ".XML")) {
        tinyxml2ReaderElement system(inputFile);
        if (system.isValid()) {
            try {
                loadFile(system);
            }
            catch (const std::exception& e) {
                std::cout << "error running system " << e.what() << std::endl;
                return errorTerminate(FILE_PROCESSING_ERROR);
            }
        } else if (broker) {
            broker->forceTerminate();
        }
    }
    currentState = State::LOADED;
    return 0;
}

int FmiRunner::run(helics::Time stop)
{
    if (currentState == State::ERROR) {
        return returnCode;
    }
    if (currentState < State::INITIALIZED) {
        const int ret = initialize();
        if (ret != 0) {
            return ret;
        }
    }
    // load each of the fmu's into its own thread
    std::vector<std::thread> threads(cosimFeds.size() + meFeds.size());
    for (size_t ii = 0; ii < cosimFeds.size(); ++ii) {
        auto* tfed = cosimFeds[ii].get();
        threads[ii] = std::thread([tfed, stop]() { tfed->run(stop); });
    }
    for (size_t jj = 0; jj < meFeds.size(); ++jj) {
        auto* tfed = meFeds[jj].get();
        threads[jj + cosimFeds.size()] = std::thread([tfed, stop]() { tfed->run(stop); });
    }
    for (auto& thread : threads) {
        thread.join();
    }
    if (core) {
        core->forceTerminate();
    }
    currentState = State::RUNNING;
    return 0;
}

std::future<int> FmiRunner::runAsync(helics::Time stop)
{
    return std::async(std::launch::async, [this, stop]() { return run(stop); });
}

int FmiRunner::initialize()
{
    if (currentState < State::LOADED) {
        const int ret = load();
        if (ret != 0) {
            return ret;
        }
    }
    if (currentState >= State::INITIALIZED) {
        return 0;
    }
    for (auto& csFed : cosimFeds) {
        csFed->configure(stepTime);
    }
    for (auto& meFed : meFeds) {
        meFed->configure(stepTime);
    }
    currentState = State::INITIALIZED;
    return 0;
}

int FmiRunner::close()
{
    cosimFeds.clear();
    meFeds.clear();
    if (broker) {
        broker->waitForDisconnect();
    }
    if (core) {
        core->waitForDisconnect();
    }
    currentState = State::CLOSED;
    return 0;
}

int FmiRunner::errorTerminate(int errorCode)
{
    if (broker) {
        broker->forceTerminate();
        broker.reset();
    }
    if (core) {
        core->forceTerminate();
        core.reset();
    }
    currentState = State::ERROR;
    returnCode = errorCode;
    return returnCode;
}

int FmiRunner::loadFile(readerElement& elem)
{
    if (elem.hasAttribute("stoptime")) {
        stopTime = elem.getAttributeValue("stoptime");
    }
    elem.moveToFirstChild("fmus");

    std::vector<std::unique_ptr<FmiLibrary>> fmis;
    while (elem.isValid()) {
        auto fmilib = std::make_unique<FmiLibrary>();
        auto str = elem.getAttributeText("fmu");
        fmilib->loadFMU(str);
        if (fmilib->checkFlag(fmuCapabilityFlags::coSimulationCapable)) {
            std::shared_ptr<fmi2CoSimObject> obj =
                fmilib->createCoSimulationObject(elem.getAttributeText("name"));
            auto fed = std::make_unique<CoSimFederate>(obj->getName(), std::move(obj), fedInfo);
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
            cosimFeds.push_back(std::move(fed));
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
            meFeds.push_back(std::move(fed));
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
        core->dataLink(str1, str2);
    }
    elem.moveToParent();
    return 0;
}

}  // namespace helicsfmi
