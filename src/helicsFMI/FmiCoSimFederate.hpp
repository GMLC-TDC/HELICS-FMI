/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "fmi/fmi_import/fmiImport.h"
#include "fmi/fmi_import/fmiObjects.h"
#include "helics/ValueFederates.hpp"



#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics
{
    class CoreApp;
}

namespace helicsfmi {
/** class defining a co-simulation federate*/
class CoSimFederate {
  private:
    helics::ValueFederate fed;  //!< the federate
    std::shared_ptr<fmi2CoSimObject> cs;  //!< the co-simulation object
    std::vector<std::string> input_list;
    std::vector<std::string> output_list;
    std::vector<std::string> connections;
    std::vector<helics::Publication> pubs;  //!< known publications
    std::vector<helics::Input> inputs;  //!< known inputs
    helics::Time stepTime{helics::timeEpsilon};  //!< the step time for the Federate
    helics::Time timeBias{helics::timeZero};  //!< time shift for the federate
    std::string outputCaptureFile;
    bool captureOutput{false};

  public:
    CoSimFederate(const std::string& name,
                  const std::string& fmu,
                  const helics::FederateInfo& fedInfo);
    CoSimFederate(const std::string& name,
                  std::shared_ptr<fmi2CoSimObject> obj,
                  const helics::FederateInfo& fedInfo);
    CoSimFederate(const std::string& name,
                  helics::CoreApp & cr,
                  const std::string& fmu,
                  const helics::FederateInfo& fedInfo);
    CoSimFederate(const std::string& name,
                  std::shared_ptr<fmi2CoSimObject> obj,
                  helics::CoreApp & cr,
                  const helics::FederateInfo& fedInfo);
    /** configure the federate using the specified inputs and outputs*/
    void configure(helics::Time step, helics::Time start = helics::timeZero);
    /** set a string list of inputs*/
    void setInputs(std::vector<std::string> input_names);
    /** set a string list of outputs*/
    void setOutputs(std::vector<std::string> output_names);
    /** set a list of connections*/
    void setConnections(std::vector<std::string> conn);
    /** add an input*/
    void addInput(const std::string& input_name);
    /** add an output*/
    void addOutput(const std::string& output_name);
    /** add a connection*/
    void addConnection(const std::string& conn);
    /** set the capture file*/
    void setOutputCapture(bool capture = true, const std::string& outputFile = "");
    /** run a command on the cosim object*/
    void runCommand(const std::string& command);
    /** set a parameter*/
    template<typename... Args>
    void set(Args&&... args)
    {
        cs->set(std::forward<Args>(args)...);
    }
    /** run the cosimulation*/
    void run(helics::Time stop);
    /** get the underlying HELICS federate*/
    helics::ValueFederate* operator->() { return &fed; }

  private:
    double initialize(double stop, std::ofstream& ofile);
};

}  // namespace helicsfmi
