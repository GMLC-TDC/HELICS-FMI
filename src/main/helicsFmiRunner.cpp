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
#include "helics-fmi/helics-fmi-config.h"
#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/helicsCLI11.hpp"
#include "helics/core/helicsVersion.hpp"
#include "helicsFMI/FmiCoSimFederate.hpp"
#include "helicsFMI/FmiModelExchangeFederate.hpp"

#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <thread>

namespace helicsfmi {

#define LOG_ERROR(message) runnerLog(HELICS_LOG_LEVEL_ERROR, message)
#define LOG_WARNING(message) runnerLog(HELICS_LOG_LEVEL_WARNING, message)

#define LOG_SUMMARY(message)                                                                       \
    if (logLevel >= HELICS_LOG_LEVEL_SUMMARY) {                                                    \
        runnerLog(HELICS_LOG_LEVEL_SUMMARY, message);                                              \
    }

#define LOG_CONNECTIONS(message)                                                                   \
    if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {                                                \
        runnerLog(HELICS_LOG_LEVEL_CONNECTIONS, message);                                          \
    }

#define LOG_INTERFACES(message)                                                                    \
    if (logLevel >= HELICS_LOG_LEVEL_INTERFACES) {                                                 \
        runnerLog(HELICS_LOG_LEVEL_INTERFACES, message);                                           \
    }

#define LOG_TIMING(message)                                                                        \
    if (logLevel >= HELICS_LOG_LEVEL_TIMING) {                                                     \
        runnerLog(HELICS_LOG_LEVEL_TIMING, message);                                               \
    }
#define LOG_DATA_MESSAGES(message)                                                                 \
    if (logLevel >= HELICS_LOG_LEVEL_DATA) {                                                       \
        runnerLog(HELICS_LOG_LEVEL_DATA, message);                                                 \
    }
#define LOG_DEBUG_MESSAGES(message)                                                                \
    if (logLevel >= HELICS_LOG_LEVEL_DEBUG) {                                                      \
        runnerLog(HELICS_LOG_LEVEL_DEBUG, message);                                                \
    }

#define LOG_TRACE(message)                                                                         \
    if (logLevel >= HELICS_LOG_LEVEL_TRACE) {                                                      \
        runnerLog(HELICS_LOG_LEVEL_TRACE, message);                                                \
    }

FmiRunner::FmiRunner()
{
    fedInfo.defName = "fmu${#}";
    fedInfo.separator = '.';
}

FmiRunner::~FmiRunner() = default;

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
                    "the step size to use, specified in milliseconds or as a time string (10s)");
    auto* stopOpt = app->add_option(
        "--stop",
        stopTime,
        "the time to stop the simulation, specified in milliseconds or as a time string (10s)");

    CLI::deprecate_option(stopOpt, "please use '--stoptime' instead");

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
    app->add_option("--extractpath", extractPath, "the file location in which to extract the FMU")
        ->check(CLI::ExistingDirectory);
    app->add_flag("--cosim",
                  cosimFmu,
                  "specify that the fmu should run as a co-sim FMU if possible");
    app->add_flag("!--modelexchange",
                  cosimFmu,
                  "specify that the fmu should run as a model exchange FMU if possible");
    app->add_option(
           "--flags",
           flags,
           "comma separated list of flags that will be passed to the cosimulation object, or the fmi object")
        ->delimiter(',');
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
    if (inputs.empty()) {
        return errorTerminate(MISSING_FILE);
    }
    const std::string inputFile = getFilePath(inputs.front());
    if (inputFile.empty()) {
        return errorTerminate(INVALID_FILE);
    }
    auto ext = inputFile.substr(inputFile.find_last_of('.'));
    try {
        if ((ext == ".json") || (ext == ".JSON")) {
            fedInfo.loadInfoFromJson(inputFile);
        } else if ((ext == ".toml") || (ext == ".TOML")) {
            fedInfo.loadInfoFromToml(inputFile);
        }
    }
    catch (const helics::HelicsException& e) {
        LOG_ERROR(fmt::format("error loading federateInfo from file :{}", e.what()));
        return errorTerminate(FILE_PROCESSING_ERROR);
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
            broker = std::make_unique<helics::BrokerApp>(fedInfo.coreType, args);
            if (!broker->isConnected()) {
                if (!broker->connect()) {
                    LOG_ERROR("broker failed to connect");
                    return errorTerminate(BROKER_CONNECT_FAILURE);
                }
            }
            LOG_SUMMARY(fmt::format("started autobroker with args \"{}\"", args));
        }
        catch (const std::exception& e) {
            LOG_ERROR(fmt::format("error generating broker :{}", e.what()));
            return errorTerminate(BROKER_CONNECT_FAILURE);
        }
    }
    fedInfo.autobroker = false;

    if (broker) {
        fedInfo.brokerPort = -1;
        fedInfo.broker = broker->getAddress();
    }
    LOG_SUMMARY(
        fmt::format("starting core with args {}", helics::generateFullCoreInitString(fedInfo)));

    core = std::make_unique<helics::CoreApp>(fedInfo.coreType,
                                             helics::generateFullCoreInitString(fedInfo));
    if (!core->isConnected()) {
        if (!core->connect()) {
            core.reset();
            LOG_ERROR("core failed to connect");
            return errorTerminate(CORE_CONNECT_FAILURE);
        }
    }
    crptr = core->getCopyofCorePointer();

    fedInfo.coreName = core->getIdentifier();
    FmiLibrary fmi;
    if ((ext == ".fmu") || (ext == ".FMU")) {
        if (stepTime > helics::timeZero) {
            fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, stepTime);
        } else {
            stepTime = fedInfo.checkTimeProperty(HELICS_PROPERTY_TIME_PERIOD, 0.001);
            if (stepTime == 0.001) {
                fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, stepTime);
            }
        }

        if (stopTime > helics::timeZero) {
            fedInfo.setProperty(HELICS_PROPERTY_TIME_STOPTIME, stopTime);
        } else {
            stopTime = fedInfo.checkTimeProperty(HELICS_PROPERTY_TIME_STOPTIME, 30.0);
            if (stopTime == 30.0) {
                fedInfo.setProperty(HELICS_PROPERTY_TIME_STOPTIME, stopTime);
            }
        }
        try {
            if (!fmi.loadFMU(inputFile, extractPath)) {
                LOG_ERROR(fmt::format("error loading fmu: error code={}", fmi.getErrorCode()));
                return errorTerminate(INVALID_FMU);
            }
            if (cosimFmu && fmi.checkFlag(fmuCapabilityFlags::coSimulationCapable)) {
                std::shared_ptr<fmi2CoSimObject> obj = fmi.createCoSimulationObject("obj1");
                if (!obj) {
                    LOG_ERROR("unable to create cosim object ");
                    return errorTerminate(FMU_ERROR);
                }
                if (fedInfo.defName.empty()) {
                    fedInfo.defName = obj->getName();
                }

                auto fed = std::make_unique<CoSimFederate>("", std::move(obj), fedInfo);

                cosimFeds.push_back(std::move(fed));
            } else {
                std::shared_ptr<fmi2ModelExchangeObject> obj =
                    fmi.createModelExchangeObject("obj1");
                if (!obj) {
                    LOG_ERROR("unable to create model exchange object");
                    return errorTerminate(FMU_ERROR);
                }
                if (fedInfo.defName.empty()) {
                    fedInfo.defName = obj->getName();
                }
                auto fed = std::make_unique<FmiModelExchangeFederate>("", std::move(obj), fedInfo);
                meFeds.push_back(std::move(fed));
            }
        }
        catch (const std::exception& e) {
            LOG_ERROR(fmt::format("error creating federate fmu: {}", e.what()));
            return errorTerminate(FMU_ERROR);
        }
    } else if ((ext == ".json") || (ext == ".JSON")) {
        jsonReaderElement system(inputFile);
        if (system.isValid()) {
            try {
                const int lfile = loadFile(system);
                if (lfile != 0) {
                    errorTerminate(FILE_PROCESSING_ERROR);
                    return lfile;
                }
                if (cosimFeds.size() + meFeds.size() == 1) {
                    if (!cosimFeds.empty()) {
                        cosimFeds.front()->loadFromFile(inputFile);
                    } else {
                        // meFeds.front()->loadFromFile(inputFile)
                    }
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR(fmt::format("error loading system ", e.what()));
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
                if (cosimFeds.size() + meFeds.size() == 1) {
                    if (!cosimFeds.empty()) {
                        cosimFeds.front()->loadFromFile(inputFile);
                    } else {
                        // meFeds.front()->loadFromFile(inputFile)
                    }
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR(fmt::format("error loading system ", e.what()));
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
                LOG_ERROR(fmt::format("error loading system ", e.what()));
                return errorTerminate(FILE_PROCESSING_ERROR);
            }
        } else {
            return errorTerminate(FILE_PROCESSING_ERROR);
        }
    }
    currentState = State::LOADED;
    for (const auto& flag : flags) {
        bool used{false};
        for (auto& fmu : cosimFeds) {
            if (flag.front() == '-') {
                used |= fmu->setFlag(flag.substr(1), false);
            } else {
                used |= fmu->setFlag(flag, true);
            }
        }
        for (auto& fmu : meFeds) {
            if (flag.front() == '-') {
                used |= fmu->setFlag(flag.substr(1), false);
            } else {
                used |= fmu->setFlag(flag, true);
            }
        }
        if (!used) {
            LOG_WARNING(fmt::format("flag {} was not recognized ", flag));
        }
    }
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
    if (stop < helics::timeZero) {
        stop = stopTime;
    }
    if (stop < stepTime) {
        LOG_WARNING(fmt::format("stoptime ({}) < steptime ({}), please check values ",
                                static_cast<double>(stop),
                                static_cast<double>(stepTime)));
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
                return DISCARDED_PARAMETER_ERROR;
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
                return DISCARDED_PARAMETER_ERROR;
            }
            ++index;
        }
    }

    for (std::size_t ii = 0; ii < paramUsed.size(); ++ii) {
        if (paramUsed[ii] == 0) {
            LOG_WARNING(fmt::format("parameter ({}) is unused ", setParameters[ii]));
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
        crptr.reset();
        core.reset();
    }
    currentState = State::CLOSED;
    return 0;
}

int FmiRunner::errorTerminate(int errorCode)
{
    LOG_ERROR(fmt::format("error terminate with code {}", errorCode));
    if (broker) {
        broker->forceTerminate();
        broker.reset();
        if (core) {
            if (!core->waitForDisconnect(std::chrono::milliseconds(150))) {
                core->forceTerminate();
            }
            crptr.reset();
            core.reset();
        }
    } else if (core) {
        core->forceTerminate();
        crptr.reset();
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

void FmiRunner::runnerLog(int loggingLevel, std::string_view message)
{
    if (loggingLevel > logLevel) {
        return;
    }
    if (core) {
        crptr->logMessage(helics::gLocalCoreId, logLevel, message);
    } else if (broker) {
        broker->sendCommand("broker", fmt::format("log {}", message));
    } else if (logLevel <= HELICS_LOG_LEVEL_WARNING) {
        std::cerr << message << std::endl;
    } else {
        std::cout << message << '\n';
    }
}

int FmiRunner::loadFile(readerElement& elem)
{
    if (stopTime == helics::Time::minVal() && elem.hasAttribute("stop")) {
        stopTime = loadTimeFromString(elem.getAttributeText("stop"), time_units::s);
    }
    if (stepTime == 1.0 && elem.hasAttribute("step")) {
        stepTime = loadTimeFromString(elem.getAttributeText("step"), time_units::s);
    }
    if (stepTime > helics::timeZero) {
        fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, stepTime);
    } else {
        stepTime = fedInfo.checkTimeProperty(HELICS_PROPERTY_TIME_PERIOD, 0.001);
        if (stepTime == 0.001) {
            fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, stepTime);
        }
    }

    if (stopTime > helics::timeZero) {
        fedInfo.setProperty(HELICS_PROPERTY_TIME_STOPTIME, stopTime);
    } else {
        stopTime = fedInfo.checkTimeProperty(HELICS_PROPERTY_TIME_STOPTIME, 30.0);
        if (stopTime == 30.0) {
            fedInfo.setProperty(HELICS_PROPERTY_TIME_STOPTIME, stopTime);
        }
    }
    if (elem.hasAttribute("extractpath")) {
        extractPath = elem.getAttributeText("extractpath");
    }
    elem.moveToFirstChild("fmus");

    std::vector<std::unique_ptr<FmiLibrary>> fmis;
    while (elem.isValid()) {
        auto fmilib = std::make_unique<FmiLibrary>();
        auto str = getFilePath(elem.getAttributeText("fmu"));
        if (str.empty()) {
            LOG_ERROR(fmt::format("unable to locate file {}", elem.getAttributeText("fmu")));
            return errorTerminate(MISSING_FILE);
        }
        fmilib->loadFMU(str, extractPath);
        if (fmilib->checkFlag(fmuCapabilityFlags::coSimulationCapable)) {
            std::shared_ptr<fmi2CoSimObject> obj =
                fmilib->createCoSimulationObject(elem.getAttributeText("name"));
            if (!obj) {
                LOG_ERROR("unable to create cosim object");
                return errorTerminate(FMU_ERROR);
            }
            auto name = obj->getName();

            std::unique_ptr<CoSimFederate> fed;
            if (elem.hasAttribute("config")) {
                auto cfile = elem.getAttributeText("config");
                auto localFedInfo = fedInfo;
                fed = std::make_unique<CoSimFederate>(name, std::move(obj), localFedInfo, cfile);
            } else {
                fed = std::make_unique<CoSimFederate>(name, std::move(obj), fedInfo);
            }
            elem.moveToFirstChild("parameters");
            while (elem.isValid()) {
                auto attr = elem.getFirstAttribute();
                if (attr.getName() == "field") {
                    const std::string& str1 = attr.getText();
                    if (!str1.empty()) {
                        double val = elem.getAttributeValue("value");
                        if (val != readerNullVal) {
                            fed->set(str1, val);
                        } else {
                            fed->set(str1, elem.getAttributeText("value"));
                        }
                    }
                } else {
                    while (attr.isValid()) {
                        const std::string& str1 = attr.getName();
                        if (!str1.empty()) {
                            double val = attr.getValue();
                            if (val != readerNullVal) {
                                fed->set(str1, val);
                            } else {
                                fed->set(str1, attr.getText());
                            }
                        }
                        attr = elem.getNextAttribute();
                    }
                }

                elem.moveToNextSibling("parameters");
            }
            elem.moveToParent();
            helics::Time localStepTime{stepTime};
            if (elem.hasAttribute("steptime")) {
                localStepTime =
                    loadTimeFromString(elem.getAttributeText("steptime"), time_units::s);
            }
            if (elem.hasAttribute("starttime")) {
                fed->configure(localStepTime,
                               loadTimeFromString(elem.getAttributeText("starttime"),
                                                  time_units::s));
            } else {
                fed->configure(localStepTime);
            }
            cosimFeds.push_back(std::move(fed));
        } else {
            std::shared_ptr<fmi2ModelExchangeObject> obj =
                fmilib->createModelExchangeObject(elem.getAttributeText("name"));
            if (!obj) {
                LOG_ERROR("unable to create model exchange object ");
                return errorTerminate(FMU_ERROR);
            }
            auto name = obj->getName();
            auto fed = std::make_unique<FmiModelExchangeFederate>(name, std::move(obj), fedInfo);
            elem.moveToFirstChild("parameters");
            while (elem.isValid()) {
                auto str1 = elem.getFirstAttribute().getText();
                auto str2 = elem.getNextAttribute().getText();
                elem.moveToNextSibling("parameters");
                fed->set(str1, str2);
            }
            elem.moveToParent();
            elem.moveToFirstChild("parameters");
            elem.moveToFirstChild();
            while (elem.isValid()) {
                auto str1 = elem.getName();
                auto str2 = elem.getText();
                elem.moveToNextSibling();
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
