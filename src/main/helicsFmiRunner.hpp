/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "helics/application_api/FederateInfo.hpp"
#include "helics/application_api/timeOperations.hpp"
#include "helics/apps/BrokerApp.hpp"
#include "helics/apps/CoreApp.hpp"
#include "helics/external/CLI11/CLI11.hpp"
#include "helicsFMI/FmiCoSimFederate.hpp"
#include "helicsFMI/FmiModelExchangeFederate.hpp"

#include <future>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helicsfmi {

class FmiRunner {
  private:
    std::string integrator{"cvode"};
    std::string integratorArgs;
    std::string brokerArgs;
    helics::Time stepTime{0.001};
    helics::Time stopTime = helics::Time::minVal();
    std::vector<std::string> inputs;
    std::vector<std::string> output_variables;
    std::vector<std::string> input_variables;
    std::vector<std::string> connections;
    //!< paths to find the fmu or other files
    std::vector<std::string> paths;
    bool cosimFmu{true};
    helics::FederateInfo fedInfo;
    std::unique_ptr<helics::apps::BrokerApp> broker;
    std::unique_ptr<helics::apps::CoreApp> core;
    std::vector<std::unique_ptr<CoSimFederate>> cosimFeds;
    std::vector<std::unique_ptr<FmiModelExchangeFederate>> meFeds;
    std::vector<std::string> setParameters;
    enum class State { CREATED, LOADED, INITIALIZED, RUNNING, CLOSED, ERROR };
    State currentState{State::CREATED};
    int returnCode{EXIT_SUCCESS};

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
        CALL_NOT_ALLOWED_IN_CURRENT_STATE = 83
    };

    FmiRunner();
    std::unique_ptr<CLI::App> generateCLI();
    /** parse a string input*/
    int parse(const std::string& cliString);
    int load();
    int initialize();
    int run(helics::Time stop = helics::initializationTime);
    std::future<int> runAsync(helics::Time stop = helics::initializationTime);

    int close();

  private:
    int loadFile(readerElement& elem);
    int errorTerminate(int errorCode);
    /// @brief  find the full path for a file name
    /// @param file the filename
    /// @return a string with the full file path
    std::string getFilePath(const std::string& file) const;
};

}  // namespace helicsfmi
