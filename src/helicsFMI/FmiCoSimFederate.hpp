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

    std::vector<helics::Publication> pubs;  //!< known publications
    std::vector<helics::Input> inputs;  //!< known inputs
  public:
    FmiCoSimFederate (std::shared_ptr<fmi2CoSimObject> obj, const helics::FederateInfo &fi);
    void run (helics::Time step, helics::Time stop);
};
