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

#pragma once

#include "fmi/fmi_import/fmiImport.h"
#include "fmi/fmi_import/fmiObjects.h"
#include "helics/ValueFederates.hpp"
#include <memory>

/** class defining a co-simulation federate*/
class FmiCoSimFederate
{
  private:
    helics::ValueFederate fed;  //!< the federate
    std::shared_ptr<fmi2CoSimObject> cs;  //!< the co-simulation object
    std::vector<std::string> input_list;
    std::vector<std::string> output_list;
    std::vector<std::string> connections;
    std::vector<helics::Publication> pubs;  //!< known publications
    std::vector<helics::Input> inputs;  //!< known inputs
    helics::Time stepTime;  //!< the step time for the Federate
    helics::Time timeBias = helics::timeZero;  //!< time shift for the federate
  public:
    FmiCoSimFederate(std::shared_ptr<fmi2CoSimObject> obj, const helics::FederateInfo &fi);
    /** configure the federate using the specified inputs and outputs*/
    void configure(helics::Time step, helics::Time start = helics::timeZero);
    /** set a string list of inputs*/
    void setInputs(std::vector<std::string> input_names);
    /** set a string list of outputs*/
    void setOutputs(std::vector<std::string> output_names);
    /** set a list of connections*/
    void setConnections(std::vector<std::string> conn);
    /** add an input*/
    void addInput(const std::string &input_name);
    /** add an output*/
    void addOutput(const std::string &output_name);
    /** add a connection*/
    void addConnection(const std::string &conn);
    /** run the cosimulation*/
    void run(helics::Time stop);
};
