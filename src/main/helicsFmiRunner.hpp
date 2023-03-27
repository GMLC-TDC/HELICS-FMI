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
    bool cosimFmu{true};
    helics::FederateInfo fedInfo;
    std::unique_ptr<helics::apps::BrokerApp> broker;
    std::unique_ptr<helics::apps::CoreApp> core;
    std::vector<std::unique_ptr<CoSimFederate>> cosimFeds;
    std::vector<std::unique_ptr<FmiModelExchangeFederate>> meFeds;
    enum class State { CREATED, LOADED, INITIALIZED, RUNNING, CLOSED, ERROR };
    State currentState{State::CREATED};
    int returnCode{EXIT_SUCCESS};

  public:
    enum ExitCodes : int {
        BrokerConnectFailure = 32,
        CoreConnectFailure = 33,
        MissingFile = 34,
        InvalidFile = 45,
        FileProcessingError = 47,
        InvalidFmu = 55,
        FmuError = 56,
        IncorrectFmu = 57,
        CallNotAllowedInCurrentState = 83
    };

  public:
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
};

}  // namespace helicsfmi
