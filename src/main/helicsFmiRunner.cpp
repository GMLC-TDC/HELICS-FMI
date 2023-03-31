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
#include "helics/core/Core.hpp"
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
    app->validate_positionals()->validate_optional_arguments();

    app->add_option("--integrator",
                    integrator,
                    "the type of integrator to use(cvode, arkode, boost)")
        ->capture_default_str()
        ->transform(CLI::IsMember({"cvode", "arkode", "boost"}));

    app->add_option("inputfile,-i,--input", inputs, "specify the input files")->required();

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
    app->add_option(
           "--set",
           setParameters,
           "Specify initial values for Fmu parameters or variables as a semocolon separated list p1=34;p2=19.5")
        ->delimiter(';');
    app->add_option("--connections", connections, "Specify connections this FMU should make")
        ->delimiter(',');
    app->add_option("-L,--fmupath",
                    paths,
                    "Specify additional search paths for the fmu's or configuration files")
        ->check(CLI::ExistingPath);
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
    std::cout << "starting FMU load" << std::endl;
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
    if (inputs.empty()) {
        return errorTerminate(MISSING_FILE);
    }
    const std::string inputFile = getFilePath(inputs.front());
    if (inputFile.empty()) {
        return errorTerminate(INVALID_FILE);
    }
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
                if (fedInfo.defName.empty()) {
                    fedInfo.defName = obj->getName();
                }

                auto cr = core->getCopyofCorePointer();
                if (!cr->isOpenToNewFederates()) {
                    std::cout << "core " << cr->getIdentifier()
                              << " is moved on to state prior to fed creation"
                              << core->query("core", "current_state") << "\n";
                } else {
                    std::cout << "core " << cr->getIdentifier()
                              << " is ready for fedarate connections "
                              << core->query("core", "current_state") << "\n";
                }
                std::cout << "starting creation of cosim federate" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (!cr->isOpenToNewFederates()) {
                    std::cout << "core " << cr->getIdentifier()
                        << " is moved on to state prior to fed creation after sleep"
                        << core->query("core", "current_state") << "\n";
                } else {
                    std::cout << "core " << cr->getIdentifier()
                        << " is ready for fedarate connections after sleep "
                        << core->query("core", "current_state") << "\n";
                }
                auto fed =
                    std::make_unique<CoSimFederate>("", std::move(obj), std::move(cr), fedInfo);
                std::cout << "fed is created" << std::endl;
                cosimFeds.push_back(std::move(fed));
            } else {
                std::shared_ptr<fmi2ModelExchangeObject> obj =
                    fmi.createModelExchangeObject("obj1");
                if (!obj) {
                    std::cout << "unable to create model exchange object " << std::endl;
                    return errorTerminate(FMU_ERROR);
                }
                if (fedInfo.defName.empty()) {
                    fedInfo.defName = obj->getName();
                }
                auto fed = std::make_unique<FmiModelExchangeFederate>("",
                                                                      std::move(obj),
                                                                      core->getCopyofCorePointer(),
                                                                      fedInfo);
                meFeds.push_back(std::move(fed));
            }
        }
        catch (const std::exception& e) {
            std::cout << "error creating federate fmu: " << e.what() << std::endl;
            auto cr = core->getCopyofCorePointer();
            if (!cr) {
                std::cout << "core not valid" << std::endl;
            } else {
                if (!core->isOpenToNewFederates()) {
                    std::cout << "core " << core->getIdentifier() << " is moved on to state "
                              << core->query("core", "current_state") << "\n";
                }
                if (!core->isConnected()) {
                    std::cout << "core " << core->getIdentifier() << " is not connected\n";
                }
            }
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
        } else {
            return errorTerminate(FILE_PROCESSING_ERROR);
        }
    } else if ((ext == ".toml") || (ext == ".TOML")) {
        tomlReaderElement system(inputFile);
        if (system.isValid()) {
            try {
                const int lfile = loadFile(system);
                if (lfile != 0) {
                    errorTerminate(FILE_PROCESSING_ERROR);
                    return lfile;
                }
            }
            catch (const std::exception& e) {
                std::cout << "error running system " << e.what() << std::endl;
                return errorTerminate(FILE_PROCESSING_ERROR);
            }
        } else {
            return errorTerminate(FILE_PROCESSING_ERROR);
        }
    } else if ((ext == ".xml") || (ext == ".XML")) {
        tinyxml2ReaderElement system(inputFile);
        if (system.isValid()) {
            try {
                const int lfile = loadFile(system);
                if (lfile != 0) {
                    errorTerminate(FILE_PROCESSING_ERROR);
                    return lfile;
                }
            }
            catch (const std::exception& e) {
                std::cout << "error running system " << e.what() << std::endl;
                return errorTerminate(FILE_PROCESSING_ERROR);
            }
        } else {
            return errorTerminate(FILE_PROCESSING_ERROR);
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
    std::vector<int> paramUsed(setParameters.size(), 0);
    for (auto& csFed : cosimFeds) {
        csFed->configure(stepTime);
        int index = 0;
        for (const auto& param : setParameters) {
            auto eloc = param.find_first_of('=');
            try {
                csFed->set(param.substr(0, eloc), param.substr(eloc + 1));
                paramUsed[index] = 1;
            }
            catch (const fmiDiscardException&) {
            }
            ++index;
        }
    }
    for (auto& meFed : meFeds) {
        meFed->configure(stepTime);
        for (const auto& param : setParameters) {
            auto eloc = param.find_first_of('=');
            int index = 0;
            try {
                meFed->set(param.substr(0, eloc), param.substr(eloc + 1));
                paramUsed[index] = 1;
            }
            catch (const fmiDiscardException&) {
            }
            ++index;
        }
    }

    for (int ii = 0; ii < paramUsed.size(); ++ii) {
        if (paramUsed[ii] == 0) {
            std::cout << "WARNING: " << setParameters[ii] << " is unused" << std::endl;
        }
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
    std::cout << "error terminate with code " << errorCode << std::endl;
    if (broker) {
        broker->forceTerminate();
        broker.reset();
        if (core) {
            if (!core->waitForDisconnect(std::chrono::milliseconds(150)))
            {
                core->forceTerminate();
            }
            core.reset();
        }
    }
    else if (core)
    {
        core->forceTerminate();
        core.reset();
    }
    currentState = State::ERROR;
    returnCode = errorCode;
    return returnCode;
}

std::string FmiRunner::getFilePath(const std::string& file) const
{
    if (std::filesystem::exists(file)) {
        return file;
    }
    if (!paths.empty()) {
        for (const auto& pth : paths) {
            std::filesystem::path libraryPath(pth);
            libraryPath /= file;
            if (std::filesystem::exists(libraryPath)) {
                return libraryPath.string();
            }
        }
    }
    return {};
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
        auto str = getFilePath(elem.getAttributeText("fmu"));
        if (str.empty()) {
            std::cout << "unable to locate file " << elem.getAttributeText("fmu") << std::endl;
            return errorTerminate(MISSING_FILE);
        }
        fmilib->loadFMU(str);
        if (fmilib->checkFlag(fmuCapabilityFlags::coSimulationCapable)) {
            std::shared_ptr<fmi2CoSimObject> obj =
                fmilib->createCoSimulationObject(elem.getAttributeText("name"));
            if (!obj) {
                std::cout << "unable to create cosim object " << std::endl;
                return errorTerminate(FMU_ERROR);
            }
            auto nm = obj->getName();
            auto fed = std::make_unique<CoSimFederate>(nm,
                                                       std::move(obj),
                                                       core->getCopyofCorePointer(),
                                                       fedInfo);
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
            if (!obj) {
                std::cout << "unable to create model exchange object " << std::endl;
                return errorTerminate(FMU_ERROR);
            }
            auto nm = obj->getName();
            auto fed = std::make_unique<FmiModelExchangeFederate>(nm,
                                                                  std::move(obj),
                                                                  core->getCopyofCorePointer(),
                                                                  fedInfo);
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
