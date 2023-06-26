/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "helics/application_api/FederateInfo.hpp"
#include "helics/application_api/timeOperations.hpp"

#include <future>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Forward declarations
namespace CLI {
class App;
}

namespace helics {
class CoreApp;
class BrokerApp;
class Core;
}  // namespace helics

class readerElement;

namespace helicsfmi {

class CoSimFederate;
class FmiModelExchangeFederate;

/// @brief  main runner class for helics-fmi
class FmiRunner {
  private:
    std::string integrator{"cvode"};
    std::string integratorArgs;
    std::string brokerArgs;
    helics::Time stepTime{1.0};
    helics::Time stopTime{helics::Time::minVal()};
    std::vector<std::string> inputs;
    std::vector<std::string> output_variables;
    std::vector<std::string> input_variables;
    std::vector<std::string> connections;
    //!< paths to find the fmu or other files
    std::vector<std::string> paths;
    std::string extractPath;
    bool cosimFmu{true};
    helics::FederateInfo fedInfo;
    std::unique_ptr<helics::BrokerApp> broker;
    std::unique_ptr<helics::CoreApp> core;
    std::vector<std::unique_ptr<CoSimFederate>> cosimFeds;
    std::vector<std::unique_ptr<FmiModelExchangeFederate>> meFeds;
    std::vector<std::string> setParameters;
    std::vector<std::string> flags;
    enum class State { CREATED, LOADED, INITIALIZED, RUNNING, CLOSED, ERROR };
    State currentState{State::CREATED};
    int returnCode{EXIT_SUCCESS};
    int logLevel{HELICS_LOG_LEVEL_SUMMARY};
    std::shared_ptr<helics::Core> crptr;

  public:
    enum ExitCodes : int {
        BROKER_CONNECT_FAILURE = 32,
        CORE_CONNECT_FAILURE = 33,
        MISSING_FILE = 34,
        INVALID_FILE = 45,
        FILE_PROCESSING_ERROR = 47,
        INVALID_FMU = 55,
        FMU_ERROR = 56,
        INCORRECT_FMU = 57,
        DISCARDED_PARAMETER_ERROR = 62,
        CALL_NOT_ALLOWED_IN_CURRENT_STATE = 83
    };

    FmiRunner();
    ~FmiRunner();
    std::unique_ptr<CLI::App> generateCLI();
    /** parse a string input*/
    int parse(const std::string& cliString);
    int load();
    int initialize();
    int run(helics::Time stop = helics::initializationTime);
    std::future<int> runAsync(helics::Time stop = helics::initializationTime);

    int close();

  private:
    void runnerLog(int loggingLevel, std::string_view message);
    int loadFile(readerElement& elem);
    int errorTerminate(int errorCode);
    /// @brief  find the full path for a file name
    /// @param file the filename
    /// @return a string with the full file path
    [[nodiscard]] std::string getFilePath(const std::string& file) const;
};

}  // namespace helicsfmi
