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

#include <memory>
#include "helics/application_api/ValueFederate.hpp"
#include "fmi_import/fmiImport.h"
#include "fmi_import/fmiObjects.h"

#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Inputs.hpp"

class OdeSolverBase;

/** class defining a modelExchange federate*/
class FmiModelExchangeFederate
{
public:
    FmiModelExchangeFederate(std::shared_ptr<fmi2ModelExchangeObject> obj, const helics::FederateInfo &fi);
    ~FmiModelExchangeFederate();

	void run(helics::Time step, helics::Time stop);
  private:
    helics::ValueFederate fed;  //!< the federate
    std::shared_ptr<fmi2ModelExchangeObject> me;   //!< the model exchange object

    std::vector<helics::Publication> pubs; //!< known publications
    std::vector<helics::Input> inputs; //!< known subscriptions
    double stepSize = 0.01; //!< the default step size of the simulation
    std::unique_ptr<OdeSolverBase> solver;
};

