/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "fmiImport.h"

#include "fmiObjects.h"
#include "gmlc/utilities/stringOps.h"
#include "helics-fmi/helics-fmi-config.h"
#include "utilities/zipUtilities.h"

#include <algorithm>
#include <boost/dll/import.hpp>
#include <boost/dll/shared_library.hpp>
#include <cstdarg>
#include <iostream>
#include <map>
#include <sstream>

using path = std::filesystem::path;

fmiBaseFunctions::fmiBaseFunctions(const std::shared_ptr<boost::dll::shared_library>& slib)
{
    fmi2GetVersion = slib->get<fmi2GetVersionTYPE>("fmi2GetVersion");
    fmi2GetTypesPlatform = slib->get<fmi2GetTypesPlatformTYPE>("fmi2GetTypesPlatform");
    fmi2Instantiate = slib->get<fmi2InstantiateTYPE>("fmi2Instantiate");
}

fmiCommonFunctions::fmiCommonFunctions(std::shared_ptr<boost::dll::shared_library> slib):
    lib(std::move(slib))
{
    // commonFunctions = std::make_shared<fmiCommonFunctions> ();

    fmi2SetDebugLogging = lib->get<fmi2SetDebugLoggingTYPE>("fmi2SetDebugLogging");

    /* Creation and destruction of FMU instances and setting debug status */
    if (lib->has("fmi2FreeInstance")) {
        fmi2FreeInstance = lib->get<fmi2FreeInstanceTYPE>("fmi2FreeInstance");
    }

    /* Enter and exit initialization mode, terminate and reset */
    fmi2SetupExperiment = lib->get<fmi2SetupExperimentTYPE>("fmi2SetupExperiment");
    fmi2EnterInitializationMode =
        lib->get<fmi2EnterInitializationModeTYPE>("fmi2EnterInitializationMode");
    fmi2ExitInitializationMode =
        lib->get<fmi2ExitInitializationModeTYPE>("fmi2ExitInitializationMode");
    fmi2Terminate = lib->get<fmi2TerminateTYPE>("fmi2Terminate");
    fmi2Reset = lib->get<fmi2ResetTYPE>("fmi2Reset");

    /* Getting and setting variable values */
    fmi2GetReal = lib->get<fmi2GetRealTYPE>("fmi2GetReal");
    fmi2GetInteger = lib->get<fmi2GetIntegerTYPE>("fmi2GetInteger");
    fmi2GetBoolean = lib->get<fmi2GetBooleanTYPE>("fmi2GetBoolean");
    fmi2GetString = lib->get<fmi2GetStringTYPE>("fmi2GetString");

    fmi2SetReal = lib->get<fmi2SetRealTYPE>("fmi2SetReal");
    fmi2SetInteger = lib->get<fmi2SetIntegerTYPE>("fmi2SetInteger");
    fmi2SetBoolean = lib->get<fmi2SetBooleanTYPE>("fmi2SetBoolean");
    fmi2SetString = lib->get<fmi2SetStringTYPE>("fmi2SetString");

    /* Getting and setting the internal FMU state */
    fmi2GetFMUstate = lib->get<fmi2GetFMUstateTYPE>("fmi2GetFMUstate");
    fmi2SetFMUstate = lib->get<fmi2SetFMUstateTYPE>("fmi2SetFMUstate");
    fmi2FreeFMUstate = lib->get<fmi2FreeFMUstateTYPE>("fmi2FreeFMUstate");
    fmi2SerializedFMUstateSize =
        lib->get<fmi2SerializedFMUstateSizeTYPE>("fmi2SerializedFMUstateSize");
    fmi2SerializeFMUstate = lib->get<fmi2SerializeFMUstateTYPE>("fmi2SerializeFMUstate");
    fmi2DeSerializeFMUstate = lib->get<fmi2DeSerializeFMUstateTYPE>("fmi2DeSerializeFMUstate");

    /* Getting partial derivatives */
    fmi2GetDirectionalDerivative =
        lib->get<fmi2GetDirectionalDerivativeTYPE>("fmi2GetDirectionalDerivative");
}

fmiModelExchangeFunctions::fmiModelExchangeFunctions(
    std::shared_ptr<boost::dll::shared_library> slib):
    lib(std::move(slib))
{
    fmi2EnterEventMode = lib->get<fmi2EnterEventModeTYPE>("fmi2EnterEventMode");
    fmi2NewDiscreteStates = lib->get<fmi2NewDiscreteStatesTYPE>("fmi2NewDiscreteStates");
    fmi2EnterContinuousTimeMode =
        lib->get<fmi2EnterContinuousTimeModeTYPE>("fmi2EnterContinuousTimeMode");
    fmi2CompletedIntegratorStep =
        lib->get<fmi2CompletedIntegratorStepTYPE>("fmi2CompletedIntegratorStep");

    /* Providing independent variables and re-initialization of caching */
    fmi2SetTime = lib->get<fmi2SetTimeTYPE>("fmi2SetTime");
    fmi2SetContinuousStates = lib->get<fmi2SetContinuousStatesTYPE>("fmi2SetContinuousStates");

    /* Evaluation of the model equations */
    fmi2GetDerivatives = lib->get<fmi2GetDerivativesTYPE>("fmi2GetDerivatives");
    fmi2GetEventIndicators = lib->get<fmi2GetEventIndicatorsTYPE>("fmi2GetEventIndicators");
    fmi2GetContinuousStates = lib->get<fmi2GetContinuousStatesTYPE>("fmi2GetContinuousStates");
    fmi2GetNominalsOfContinuousStates =
        lib->get<fmi2GetNominalsOfContinuousStatesTYPE>("fmi2GetNominalsOfContinuousStates");
}

fmiCoSimFunctions::fmiCoSimFunctions(std::shared_ptr<boost::dll::shared_library> slib):
    lib(std::move(slib))
{
    fmi2SetRealInputDerivatives =
        lib->get<fmi2SetRealInputDerivativesTYPE>("fmi2SetRealInputDerivatives");
    fmi2GetRealOutputDerivatives =
        lib->get<fmi2GetRealOutputDerivativesTYPE>("fmi2GetRealOutputDerivatives");

    fmi2DoStep = lib->get<fmi2DoStepTYPE>("fmi2DoStep");
    fmi2CancelStep = lib->get<fmi2CancelStepTYPE>("fmi2CancelStep");

    /* Inquire slave status */
    fmi2GetStatus = lib->get<fmi2GetStatusTYPE>("fmi2GetStatus");
    fmi2GetRealStatus = lib->get<fmi2GetRealStatusTYPE>("fmi2GetRealStatus");
    fmi2GetIntegerStatus = lib->get<fmi2GetIntegerStatusTYPE>("fmi2GetIntegerStatus");
    fmi2GetBooleanStatus = lib->get<fmi2GetBooleanStatusTYPE>("fmi2GetBooleanStatus");
    fmi2GetStringStatus = lib->get<fmi2GetStringStatusTYPE>("fmi2GetStringStatus");
}

FmiLibrary::FmiLibrary(): logger(std::make_shared<FmiLogger>())
{
    information = std::make_shared<FmiInfo>();
}

FmiLibrary::FmiLibrary(const std::string& fmuPath): FmiLibrary()
{
    loadFMU(fmuPath);
}

FmiLibrary::FmiLibrary(const std::string& fmuPath, const std::string& extractPath):
    extractDirectory(extractPath), fmuName(fmuPath)
{
    information = std::make_shared<FmiInfo>();
    if (!exists(extractDirectory)) {
        create_directories(extractDirectory);
    }
    loadInformation();
}

FmiLibrary::~FmiLibrary()
{
    if (deleteDirectory && extracted) {
        try {
            std::filesystem::remove_all(extractDirectory);
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "unable to remove directory " << extractDirectory << " :" << e.what()
                      << std::endl;
        }
    }
}

void FmiLibrary::close()
{
    soMeLoaded = false;
    soCoSimLoaded = false;
    lib = nullptr;
}

bool FmiLibrary::checkFlag(fmuCapabilityFlags flag) const
{
    return information->checkFlag(flag);
}

bool FmiLibrary::isSoLoaded(fmu_type type) const
{
    switch (type) {
        case fmu_type::modelExchange:
            return soMeLoaded;
        case fmu_type::cosimulation:
            return soCoSimLoaded;
        default:
            return (soMeLoaded || soCoSimLoaded);
    }
}

bool FmiLibrary::loadFMU(const std::string& fmuPath)
{
    const path ipath(fmuPath);
    if (is_directory(ipath)) {
        extractDirectory = ipath;
    } else {
        fmuName = ipath;
        auto status = std::filesystem::status(fmuName.parent_path());

        // Check if the directory is read-only
        if ((status.permissions() & std::filesystem::perms::all) == std::filesystem::perms::none) {
            if (!std::filesystem::exists(fmuName.parent_path() / fmuName.stem())) {
                // if we are in a read only directory and the path doesn't exist then extract to
                // temp directory
                extractDirectory = std::filesystem::temp_directory_path() / fmuName.stem();
            }
        } else {
            extractDirectory = fmuName.parent_path() / fmuName.stem();
        }
    }
    return loadInformation();
}

bool FmiLibrary::loadFMU(const std::string& fmuPath, const std::string& extractLoc)
{
    extractDirectory = extractLoc;
    fmuName = fmuPath;
    return loadInformation();
}

int FmiLibrary::getCounts(fmiVariableType countType) const
{
    int cnt = invalidCount;
    switch (countType) {
        case fmiVariableType::meObject:
            cnt = mecount;
            break;
        case fmiVariableType::csObject:
            cnt = cosimcount;
            break;
        default:
            return information->getCounts(countType);
    }

    return cnt;
}

bool FmiLibrary::loadInformation()
{
    auto xmlfile = extractDirectory / "modelDescription.xml";
    if (!exists(xmlfile)) {
        auto res = extract();
        if (res != 0) {
            return false;
        }
    }
    const int res = information->loadFile(xmlfile.string());
    if (res != 0) {
        errorCode = res;
        return false;
    }
    xmlLoaded = true;

    // load the resources directory location if it exists
    if (exists(extractDirectory / "resources")) {
        resourceDir = extractDirectory / "resources";
        if (resourceDir.is_relative()) {
            resourceDir = std::filesystem::absolute(resourceDir);
        }
    } else {
        resourceDir = "";
    }
    return true;
}

std::string FmiLibrary::getTypes() const
{
    if (isSoLoaded()) {
        return {baseFunctions.fmi2GetTypesPlatform()};
    }
    return "";
}

std::string FmiLibrary::getVersion() const
{
    if (isSoLoaded()) {
        return {baseFunctions.fmi2GetVersion()};
    }
    return "";
}

int FmiLibrary::extract()
{
    const int ret = utilities::unzip(fmuName.string(), extractDirectory.string());
    if (ret != 0) {
        errorCode = ret;
    }
    extracted = true;
    return ret;
}

std::unique_ptr<fmi2ModelExchangeObject>
    FmiLibrary::createModelExchangeObject(const std::string& name)
{
    if (!isSoLoaded()) {
        loadSharedLibrary(fmu_type::modelExchange);
    }
    if (soMeLoaded) {
        if (!callbacks) {
            makeCallbackFunctions();
        }
        auto* comp =
            baseFunctions.fmi2Instantiate(name.c_str(),
                                          fmi2ModelExchange,
                                          information->getString("guid").c_str(),
                                          (R"raw(file:///)raw" + resourceDir.string()).c_str(),
                                          reinterpret_cast<fmi2CallbackFunctions*>(callbacks.get()),
                                          fmi2False,
                                          fmi2False);
        auto meobj = std::make_unique<fmi2ModelExchangeObject>(
            name, comp, information, commonFunctions, ModelExchangeFunctions);
        meobj->setLogger(logger);
        ++mecount;
        return meobj;
    }

    return nullptr;
}

std::unique_ptr<fmi2CoSimObject> FmiLibrary::createCoSimulationObject(const std::string& name)
{
    if (!isSoLoaded()) {
        loadSharedLibrary(fmu_type::cosimulation);
    }
    if (soCoSimLoaded) {
        if (!callbacks) {
            makeCallbackFunctions();
        }
        auto* comp =
            baseFunctions.fmi2Instantiate(name.c_str(),
                                          fmi2CoSimulation,
                                          information->getString("guid").c_str(),
                                          (R"raw(file:///)raw" + resourceDir.string()).c_str(),
                                          reinterpret_cast<fmi2CallbackFunctions*>(callbacks.get()),
                                          fmi2False,
                                          fmi2False);
        auto csobj = std::make_unique<fmi2CoSimObject>(
            name, comp, information, commonFunctions, CoSimFunctions);
        csobj->setLogger(logger);
        ++cosimcount;
        return csobj;
    }
    return nullptr;
}

void FmiLibrary::loadSharedLibrary(fmu_type type)
{
    if (isSoLoaded()) {
        return;
    }
    auto sopath = findSoPath(type);
    bool loaded = false;
    if (!sopath.empty()) {
        lib = std::make_shared<boost::dll::shared_library>(sopath);
        if (lib->is_loaded()) {
            loaded = true;
        }
    }
    if (loaded) {
        baseFunctions = fmiBaseFunctions(lib);
        commonFunctions = std::make_shared<fmiCommonFunctions>(lib);
        // Only load one or the other
        if (checkFlag(modelExchangeCapable) && type._value != fmu_type::cosimulation) {
            ModelExchangeFunctions = std::make_shared<fmiModelExchangeFunctions>(lib);
            soMeLoaded = true;
            soCoSimLoaded = false;
        } else if (checkFlag(coSimulationCapable)) {
            CoSimFunctions = std::make_shared<fmiCoSimFunctions>(lib);
            soCoSimLoaded = true;
            soMeLoaded = false;
        }
    }
}

path FmiLibrary::findSoPath(fmu_type type)
{
    path sopath = extractDirectory / "binaries";
    path sopathDebug = sopath;
    std::string identifier;
    switch (type) {
        case fmu_type::unknown:
        default:
            if (checkFlag(modelExchangeCapable))  // give priority to model Exchange
            {
                identifier = information->getString("meidentifier");
            } else if (checkFlag(coSimulationCapable)) {
                identifier = information->getString("cosimidentifier");
            } else {
                return path{};
            }
            break;
        case fmu_type::cosimulation:
            if (checkFlag(coSimulationCapable)) {
                identifier = information->getString("cosimidentifier");
            } else {
                return path{};
            }
            break;
        case fmu_type::modelExchange:
            if (checkFlag(modelExchangeCapable)) {
                identifier = information->getString("meidentifier");
            } else {
                return path{};
            }
            break;
    }
    if constexpr (sizeof(void*) == 8) {
#ifdef _WIN32
        sopath /= "win64";
        sopathDebug = sopath / (identifier + "d.dll");
        sopath /= identifier + ".dll";
#else
#    ifdef MACOS
        sopath /= "darwin64";
        sopathDebug = sopath / (identifier + "d.dylib");
        sopath /= identifier + ".dylib";
#    else
        sopath /= "linux64";
        sopathDebug = sopath / (identifier + "d.so");
        sopath /= identifier + ".so";
#    endif
#endif

    } else {
#ifdef _WIN32
        sopath /= "win32";
        sopathDebug = sopath / (identifier + "d.dll");
        sopath /= identifier + ".dll";
#else
#    ifdef MACOS
        sopath /= "darwin32";
        sopathDebug = sopath / (identifier + "d.dylib");
        sopath /= identifier + ".dylib";
#    else
        sopath /= "linux32";
        sopathDebug = sopath / (identifier + "d.so");
        sopath /= identifier + ".so";
#    endif
#endif
    }

    if (exists(sopath)) {
        return sopath;
    }
    if (exists(sopathDebug)) {
        return sopathDebug;
    }

    return path{};
}

void FmiLibrary::makeCallbackFunctions()
{
    callbacks = std::make_shared<fmi2CallbackFunctions_nc>();
    callbacks->allocateMemory = &calloc;
    callbacks->freeMemory = &free;
    callbacks->logger = &loggerFunc;
    callbacks->componentEnvironment = static_cast<void*>(logger.get());
}

void FmiLogger::logMessage(std::string_view category, std::string_view message) const
{
    if (loggerCallback) {
        loggerCallback(category, message);
    } else {
        std::cout << message << std::endl;
    }
}

void FmiLibrary::logMessage(std::string_view message) const
{
    if (logger && logger->check()) {
        logger->logMessage("", message);
    }
}

static constexpr std::size_t cStringBufferSize{1024};

void loggerFunc(fmi2ComponentEnvironment compEnv,
                [[maybe_unused]] fmi2String instanceName,
                [[maybe_unused]] fmi2Status status,
                [[maybe_unused]] fmi2String category,
                fmi2String message,
                ...)
{
    std::string temp;
    temp.resize(cStringBufferSize);
    va_list arglist;
    va_start(arglist, message);
    auto stringSize = vsnprintf(temp.data(), cStringBufferSize, message, arglist);
    va_end(arglist);
    temp.resize(std::min(static_cast<std::size_t>(stringSize), cStringBufferSize));

    auto* logger = reinterpret_cast<FmiLogger*>(compEnv);
    if (logger != nullptr && logger->check()) {
        std::stringstream logstream;
        logstream << instanceName << "(" << status << "):" << temp;
        logger->logMessage(category, logstream.str());
    } else {
        std::cout << instanceName << "(" << status << "):" << temp << std::endl;
    }
}
