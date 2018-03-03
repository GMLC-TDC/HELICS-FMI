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

#include "FmiCoSimFederate.hpp"
#include "fmi_import/fmiObjects.h"

FmiCoSimFederate::FmiCoSimFederate(std::shared_ptr<fmi2CoSimObject> obj, const helics::FederateInfo &fi):cs(std::move(obj)),fed(fi)
{
    auto inputs = obj->getInputNames();
    for (auto input : inputs)
    {
        subs.emplace_back(&fed, input);
    }
     
    auto outputs = obj->getOutputNames();
    for (auto output : outputs)
    {
        pubs.emplace_back(&fed, output, helics::helics_type_t::helicsDouble);
    }
}
