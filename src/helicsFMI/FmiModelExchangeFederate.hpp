/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "fmi/fmi_import/fmiImport.h"
#include "fmi/fmi_import/fmiObjects.h"
#include "helics/application_api/Inputs.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "solvers/SolvableObject.hpp"

#include <memory>
#include <string>
#include <vector>

namespace griddyn {
class SolverInterface;
class SolverMode;
}  // namespace griddyn

/** class defining a modelExchange federate*/
class FmiModelExchangeFederate: public griddyn::SolvableObject {
  public:
    FmiModelExchangeFederate(std::shared_ptr<fmi2ModelExchangeObject> obj,
                             const helics::FederateInfo& fi);
    virtual ~FmiModelExchangeFederate();
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
    /** run the cosimulation*/
    void run(helics::Time stop);

    virtual solver_index_type jacobianSize(const griddyn::solverMode& sMode) const override;

    virtual void guessCurrentValue(double time,
                                   double state[],
                                   double dstate_dt[],
                                   const griddyn::solverMode& sMode) override;

    virtual int residualFunction(double time,
                                 const double state[],
                                 const double dstate_dt[],
                                 double resid[],
                                 const griddyn::solverMode& sMode) noexcept override;

    virtual int derivativeFunction(double time,
                                   const double state[],
                                   double dstate_dt[],
                                   const griddyn::solverMode& sMode) noexcept override;

    virtual int algUpdateFunction(double time,
                                  const double state[],
                                  double update[],
                                  const griddyn::solverMode& sMode,
                                  double alpha) noexcept override;

    virtual int jacobianFunction(double time,
                                 const double state[],
                                 const double dstate_dt[],
                                 matrixData<double>& md,
                                 double cj,
                                 const griddyn::solverMode& sMode) noexcept override;
    virtual int rootFindingFunction(double time,
                                    const double state[],
                                    const double dstate_dt[],
                                    double roots[],
                                    const griddyn::solverMode& sMode) noexcept override;

    virtual int dynAlgebraicSolve(double time,
                                  const double diffState[],
                                  const double deriv[],
                                  const griddyn::solverMode& sMode) noexcept override;

  private:
    helics::ValueFederate fed;  //!< the federate
    std::shared_ptr<fmi2ModelExchangeObject> me;  //!< the model exchange object

    std::vector<std::string> input_list;
    std::vector<std::string> output_list;
    std::vector<std::string> connections;
    helics::Time stepTime{helics::timeEpsilon};  //!< the step time for the Federate
    helics::Time timeBias{
        helics::timeZero};  //!< the starting time for the FMU with a bias shift from 0
    std::vector<helics::Publication> pubs;  //!< known publications
    std::vector<helics::Input> inputs;  //!< known subscriptions
    double stepSize{0.01};  //!< the default step size of the simulation
    std::unique_ptr<griddyn::SolverInterface> solver;
};
