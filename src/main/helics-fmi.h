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

#ifndef HELICS_FMI_HEADER_H_
#define HELICS_FMI_HEADER_H_
#pragma once

#include "helics/application_api/ValueFederate.hpp"
#include "fmi_import/fmiImport.h"
#include "fmi_import/fmiObjects.h"

#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"

/** class defining a cosimulation federate*/
class fmiCoSimFederate
{
public:
    helics::ValueFederate fed;  //!< the federate
    fmi2CoSimObject *obj;   //!< the co-simulation object

    std::vector<helics::Publication> pubs; //!< known publications
    std::vector<helics::Subscription> subs; //!< known subscriptions
};

std::unique_ptr<fmiCoSimFederate> createFmiValueFederate(fmi2CoSimObject *obj, helics::FederateInfo &fi);

#endif HELICS_FMI_HEADER_H_