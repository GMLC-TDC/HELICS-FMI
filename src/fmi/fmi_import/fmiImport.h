/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "../FMI2/fmi2FunctionTypes.h"
#include "fmiInfo.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <string>

class readerElement;

class fmi2functions;

namespace boost {
namespace dll {
    class shared_library;
}
}  // namespace boost

/** an editable version of the fmi callback structure */
typedef struct {
    fmi2CallbackLogger logger;
    fmi2CallbackAllocateMemory allocateMemory;
    fmi2CallbackFreeMemory freeMemory;
    fmi2StepFinished stepFinished;
    fmi2ComponentEnvironment componentEnvironment;
} fmi2CallbackFunctions_nc;

/**container class that do not require an fmi2Component*/
class fmiBaseFunctions {
  public:
    std::function<fmi2GetTypesPlatformTYPE> fmi2GetTypesPlatform;
    std::function<fmi2GetVersionTYPE> fmi2GetVersion;
    std::function<fmi2InstantiateTYPE> fmi2Instantiate;

    fmiBaseFunctions() = default;
    fmiBaseFunctions(const std::shared_ptr<boost::dll::shared_library>& lib);
};

/**container class for functions that are common to all FMU's*/
class fmiCommonFunctions {
  public:
    std::function<fmi2SetDebugLoggingTYPE> fmi2SetDebugLogging;

    /* Creation and destruction of FMU instances and setting debug status */

    std::function<fmi2FreeInstanceTYPE> fmi2FreeInstance;

    /* Enter and exit initialization mode, terminate and reset */
    std::function<fmi2SetupExperimentTYPE> fmi2SetupExperiment;
    std::function<fmi2EnterInitializationModeTYPE> fmi2EnterInitializationMode;
    std::function<fmi2ExitInitializationModeTYPE> fmi2ExitInitializationMode;
    std::function<fmi2TerminateTYPE> fmi2Terminate;
    std::function<fmi2ResetTYPE> fmi2Reset;

    /* Getting and setting variable values */
    std::function<fmi2GetRealTYPE> fmi2GetReal;
    std::function<fmi2GetIntegerTYPE> fmi2GetInteger;
    std::function<fmi2GetBooleanTYPE> fmi2GetBoolean;
    std::function<fmi2GetStringTYPE> fmi2GetString;

    std::function<fmi2SetRealTYPE> fmi2SetReal;
    std::function<fmi2SetIntegerTYPE> fmi2SetInteger;
    std::function<fmi2SetBooleanTYPE> fmi2SetBoolean;
    std::function<fmi2SetStringTYPE> fmi2SetString;

    /* Getting and setting the internal FMU state */
    std::function<fmi2GetFMUstateTYPE> fmi2GetFMUstate;
    std::function<fmi2SetFMUstateTYPE> fmi2SetFMUstate;
    std::function<fmi2FreeFMUstateTYPE> fmi2FreeFMUstate;
    std::function<fmi2SerializedFMUstateSizeTYPE> fmi2SerializedFMUstateSize;
    std::function<fmi2SerializeFMUstateTYPE> fmi2SerializeFMUstate;
    std::function<fmi2DeSerializeFMUstateTYPE> fmi2DeSerializeFMUstate;

    /* Getting partial derivatives */
    std::function<fmi2GetDirectionalDerivativeTYPE> fmi2GetDirectionalDerivative;
    std::shared_ptr<boost::dll::shared_library> lib;

    fmiCommonFunctions() = default;
    fmiCommonFunctions(std::shared_ptr<boost::dll::shared_library> lib);
};

/**container class for functions that are specific to model exchange*/
class fmiModelExchangeFunctions {
  public:
    std::function<fmi2EnterEventModeTYPE> fmi2EnterEventMode;
    std::function<fmi2NewDiscreteStatesTYPE> fmi2NewDiscreteStates;
    std::function<fmi2EnterContinuousTimeModeTYPE> fmi2EnterContinuousTimeMode;
    std::function<fmi2CompletedIntegratorStepTYPE> fmi2CompletedIntegratorStep;

    /* Providing independent variables and re-initialization of caching */
    std::function<fmi2SetTimeTYPE> fmi2SetTime;
    std::function<fmi2SetContinuousStatesTYPE> fmi2SetContinuousStates;

    /* Evaluation of the model equations */
    std::function<fmi2GetDerivativesTYPE> fmi2GetDerivatives;
    std::function<fmi2GetEventIndicatorsTYPE> fmi2GetEventIndicators;
    std::function<fmi2GetContinuousStatesTYPE> fmi2GetContinuousStates;
    std::function<fmi2GetNominalsOfContinuousStatesTYPE> fmi2GetNominalsOfContinuousStates;

    std::shared_ptr<boost::dll::shared_library> lib;

    fmiModelExchangeFunctions() = default;
    fmiModelExchangeFunctions(std::shared_ptr<boost::dll::shared_library> lib);
};

/**container class for functions that are specific to coSimuluation*/
class fmiCoSimFunctions {
  public:
    std::function<fmi2SetRealInputDerivativesTYPE> fmi2SetRealInputDerivatives;
    std::function<fmi2GetRealOutputDerivativesTYPE> fmi2GetRealOutputDerivatives;

    std::function<fmi2DoStepTYPE> fmi2DoStep;
    std::function<fmi2CancelStepTYPE> fmi2CancelStep;

    /* Inquire slave status */
    std::function<fmi2GetStatusTYPE> fmi2GetStatus;
    std::function<fmi2GetRealStatusTYPE> fmi2GetRealStatus;
    std::function<fmi2GetIntegerStatusTYPE> fmi2GetIntegerStatus;
    std::function<fmi2GetBooleanStatusTYPE> fmi2GetBooleanStatus;
    std::function<fmi2GetStringStatusTYPE> fmi2GetStringStatus;

    std::shared_ptr<boost::dll::shared_library> lib;

    fmiCoSimFunctions() = default;
    fmiCoSimFunctions(std::shared_ptr<boost::dll::shared_library> lib);
};

// forward declarations NOTE:: may move around later
class fmi2ModelExchangeObject;
class fmi2CoSimObject;

/** @brief class for loading an fmu file information
 *@details class extracts and FMU if needed then searches for the xml file and loads the information
 */
class FmiLibrary {
  public:
    FmiLibrary();
    ~FmiLibrary();
    /** construct an fmilibrary object from the fmu path
    @param[in] fmuPath the path to the fmu object
    */
    explicit FmiLibrary(const std::string& fmuPath);
    /** construct an fmilibrary object from the fmu path
    @param[in] fmuPath the path to the fmu
    @param[in] extractLoc the folder location to extract the FMU
    */
    FmiLibrary(const std::string& fmuPath, const std::string& extractLoc);
    /** check if the xml file for the fmu has been loaded
    @return true if loaded false if not*/
    bool isXmlLoaded() const { return xmlLoaded; }
    /** check if the shared object file for the fmu has been loaded
    @return true if loaded false if not*/
    bool isSoLoaded(fmu_type type = fmu_type::unknown) const;
    /** load the FMU from the fmu path
    @param[in] fmuPath the file location of the fmu
    @return true if the fmu was loaded*/
    bool loadFMU(const std::string& fmuPath);
    /** load the FMU from the fmu path
    @param[in] fmuPath the file location of the fmu
    @param[in] extractLoc the path to extract the fmu to
    @return true if the fmu was loaded false otherwise
    */
    bool loadFMU(const std::string& fmuPath, const std::string& extractLoc);
    std::shared_ptr<fmiInfo> getInfo() const { return information; }
    void close();
    const std::string& getName() const { return modelName; }
    int getCounts(fmiVariableType countType) const;
    void loadSharedLibrary(fmu_type type = fmu_type::unknown);

    bool checkFlag(fmuCapabilityFlags flag) const;

    std::unique_ptr<fmi2ModelExchangeObject> createModelExchangeObject(const std::string& name);
    std::unique_ptr<fmi2CoSimObject> createCoSimulationObject(const std::string& name);
    std::string getTypes();
    std::string getVersion();
    /** remove the FMU extraction directory on close*/
    void deleteFMUdirectory(bool deleteDir = true) { deleteDirectory = deleteDir; }
    /** get the current status of the delete Directory modifier*/
    bool getDeleteFMUDirectory() const { return deleteDirectory; }
    /** get the latest error code*/
    int getErrorCode() const { return errorCode; }

  private:  // private functions
    bool loadInformation();
    int extract();

    std::filesystem::path findSoPath(fmu_type type = fmu_type::unknown);

    void makeCallbackFunctions();

  private:  // private Variables
    std::filesystem::path extractDirectory;  //!< the path to the extracted directory
    std::filesystem::path fmuName;  //!< the path to the FMU file itself
    std::filesystem::path resourceDir;  //!< the path to the resource Directory

    bool xmlLoaded = false;  //!< flag indicating that the FMU information has been loaded
    bool soMeLoaded =
        false;  //!< flag indicating that the Shared Library has been loaded for modelExchange
    bool soCoSimLoaded{
        false};  //!< flag indicating that the shared library has been loaded for CoSimulation;
    std::string modelName;  //!< the name of the model
    int mecount = 0;  //!< counter for the number of created model exchange objects
    int cosimcount = 0;  //!< counter for the number of created co-simulation objects
    std::shared_ptr<fmiInfo>
        information;  //!< an object containing information derived from the FMU XML file

    int errorCode{0};  //!< code indicating error source
    bool deleteDirectory{
        false};  //!< indicator that on close the fmiInfoshould delete the directory
    bool extracted{false};  //!< set to true if the FMU was extracted
    std::shared_ptr<boost::dll::shared_library> lib;
    std::shared_ptr<fmi2CallbackFunctions_nc> callbacks;
    fmiBaseFunctions baseFunctions;
    std::shared_ptr<fmiCommonFunctions> commonFunctions;
    std::shared_ptr<fmiModelExchangeFunctions> ModelExchangeFunctions;
    std::shared_ptr<fmiCoSimFunctions> CoSimFunctions;
};

/** logging function to capture log messages
@details converts to a string then calls a component specific logging function
@param[in] compEnv the environment of the logger
@param[in] instanceName the name of the object
@param[in] status the status of the message
@param[in] message the string to add to the log
*/
void loggerFunc(fmi2ComponentEnvironment compEnv,
                fmi2String instanceName,
                fmi2Status status,
                fmi2String category,
                fmi2String message,
                ...);
