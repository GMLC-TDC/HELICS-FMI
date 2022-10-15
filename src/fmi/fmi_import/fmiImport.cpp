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

#include "fmiImport.h"

#include "fmiObjects.h"
#include "gmlc/utilities/stringOps.h"
#include "helics-fmi/helics-fmi-config.h"
#include "utilities/zipUtilities.h"

#include <boost/dll/import.hpp>
#include <boost/dll/shared_library.hpp>
#include <cstdarg>
#include <map>

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

    fmi2FreeInstance = lib->get<fmi2FreeInstanceTYPE>("fmi2FreeInstance");

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

fmiLibrary::fmiLibrary()
{
    information = std::make_shared<fmiInfo>();
}

fmiLibrary::fmiLibrary(const std::string& fmupath): fmiLibrary()
{
    loadFMU(fmupath);
}

fmiLibrary::fmiLibrary(const std::string& fmupath, const std::string& extractPath):
    extractDirectory(extractPath), fmuName(fmupath)
{
    information = std::make_shared<fmiInfo>();
    if (!exists(extractDirectory)) {
        create_directories(extractDirectory);
    }
    loadInformation();
}

fmiLibrary::~fmiLibrary() = default;

void fmiLibrary::close()
{
    soMeLoaded = false;
    soCoSimLoaded = false;
    lib = nullptr;
}

bool fmiLibrary::checkFlag(fmuCapabilityFlags flag) const
{
    return information->checkFlag(flag);
}

bool fmiLibrary::isSoLoaded(fmu_type type) const
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

void fmiLibrary::loadFMU(const std::string& fmupath)
{
    path ipath(fmupath);
    if (is_directory(ipath)) {
        extractDirectory = ipath;
    } else {
        fmuName = ipath;
        extractDirectory = fmuName.parent_path() / fmuName.stem();
    }
    loadInformation();
}

void fmiLibrary::loadFMU(const std::string& fmupath, const std::string& extractLoc)
{
    extractDirectory = extractLoc;
    fmuName = fmupath;
    loadInformation();
}

int fmiLibrary::getCounts(const std::string& countType) const
{
    size_t cnt = size_t(-1);
    if (countType == "meobjects") {
        cnt = mecount;
    } else if (countType == "cosimobjects") {
        cnt = cosimcount;
    } else {
        cnt = information->getCounts(countType);
    }

    if (cnt == size_t(-1)) {
        return (-1);
    }
    return static_cast<int>(cnt);
}

void fmiLibrary::loadInformation()
{
    auto xmlfile = extractDirectory / "modelDescription.xml";
    if (!exists(xmlfile)) {
        auto res = extract();
        if (res != 0) {
            return;
        }
    }
    int res = information->loadFile(xmlfile.string());
    if (res != 0) {
        error = true;
        return;
    }
    xmlLoaded = true;

    // load the resources directory location if it exists
    if (exists(extractDirectory / "resources")) {
        resourceDir = extractDirectory / "resources";
    } else {
        resourceDir = "";
    }
}

std::string fmiLibrary::getTypes()
{
    if (isSoLoaded()) {
        return std::string(baseFunctions.fmi2GetTypesPlatform());
    }
    return "";
}

std::string fmiLibrary::getVersion()
{
    if (isSoLoaded()) {
        return std::string(baseFunctions.fmi2GetVersion());
    }
    return "";
}

int fmiLibrary::extract()
{
    int ret = utilities::unzip(fmuName.string(), extractDirectory.string());
    if (ret != 0) {
        error = true;
    }
    return ret;
}

std::unique_ptr<fmi2ModelExchangeObject>
    fmiLibrary::createModelExchangeObject(const std::string& name)
{
    if (!isSoLoaded()) {
        loadSharedLibrary(fmu_type::modelExchange);
    }
    if (soMeLoaded) {
        if (!callbacks) {
            makeCallbackFunctions();
        }
        auto comp =
            baseFunctions.fmi2Instantiate(name.c_str(),
                                          fmi2ModelExchange,
                                          information->getString("guid").c_str(),
                                          (R"raw(file:///)raw" + resourceDir.string()).c_str(),
                                          reinterpret_cast<fmi2CallbackFunctions*>(callbacks.get()),
                                          fmi2False,
                                          fmi2False);
        auto meobj = std::make_unique<fmi2ModelExchangeObject>(
            name, comp, information, commonFunctions, ModelExchangeFunctions);
        ++mecount;
        return meobj;
    }

    return nullptr;
}

std::unique_ptr<fmi2CoSimObject> fmiLibrary::createCoSimulationObject(const std::string& name)
{
    if (!isSoLoaded()) {
        loadSharedLibrary(fmu_type::cosimulation);
    }
    if (soCoSimLoaded) {
        if (!callbacks) {
            makeCallbackFunctions();
        }
        auto comp =
            baseFunctions.fmi2Instantiate(name.c_str(),
                                          fmi2CoSimulation,
                                          information->getString("guid").c_str(),
                                          (R"raw(file:///)raw" + resourceDir.string()).c_str(),
                                          reinterpret_cast<fmi2CallbackFunctions*>(callbacks.get()),
                                          fmi2False,
                                          fmi2False);
        auto csobj = std::make_unique<fmi2CoSimObject>(
            name, comp, information, commonFunctions, CoSimFunctions);
        ++cosimcount;
        return csobj;
    }
    return nullptr;
}

void fmiLibrary::loadSharedLibrary(fmu_type type)
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

path fmiLibrary::findSoPath(fmu_type type)
{
    path sopath = extractDirectory / "binaries";

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
        sopath /= identifier + ".dll";
#else
#    ifdef MACOS
        sopath /= "darwin64";
        sopath /= identifier + ".dylib";
#    else
        sopath /= "linux64";
        sopath /= identifier + ".so";
#    endif
#endif

        if (exists(sopath)) {
            return sopath;
        }
#ifdef MACOS
        sopath /= "darwin64";
        sopath /= identifier + ".so";
#endif
    } else {
#ifdef _WIN32
        sopath /= "win32";
        sopath /= identifier + ".dll";
#else
#    ifdef MACOS
        sopath /= "darwin32";
        sopath /= identifier + ".dylib";
#    else
        sopath /= "linux32";
        sopath /= identifier + ".so";
#    endif
#endif
    }

    if (exists(sopath)) {
        return sopath;
    }

    return path{};
}

void fmiLibrary::makeCallbackFunctions()
{
    callbacks = std::make_shared<fmi2CallbackFunctions_nc>();
    callbacks->allocateMemory = &calloc;
    callbacks->freeMemory = &free;
    callbacks->logger = &loggerFunc;
    callbacks->componentEnvironment = static_cast<void*>(this);
}

#define STRING_BUFFER_SIZE 1000
void loggerFunc([[maybe_unused]] fmi2ComponentEnvironment compEnv,
    [[maybe_unused]] fmi2String instanceName,
    [[maybe_unused]] fmi2Status status,
    [[maybe_unused]] fmi2String category,
                fmi2String message,
                ...)
{
    std::string temp;
    temp.resize(STRING_BUFFER_SIZE);
    va_list arglist;
    va_start(arglist, message);
    auto sz = vsnprintf(&(temp[0]), STRING_BUFFER_SIZE, message, arglist);
    va_end(arglist);
    temp.resize(std::min(sz, STRING_BUFFER_SIZE));
    printf("%s\n", temp.c_str());
}
