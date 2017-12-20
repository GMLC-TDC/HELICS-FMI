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

#include "helics-fmi.h"
#include "fmi_import/fmiObjects.h"

std::unique_ptr<fmiCoSimFederate> createFmiValueFederate(fmi2CoSimObject *obj, helics::FederateInfo &fi)
{
    if (obj == nullptr)
    {
        return nullptr;
    }
    auto fed = std::make_unique<fmiCoSimFederate>();
    fed->obj = obj;
    fed->fed = helics::ValueFederate(fi);

    auto inputs = obj->getInputNames();
    for (auto input : inputs)
    {
        fed->subs.emplace_back(&(fed->fed), input);
    }

    auto outputs = obj->getOutputNames();
    for (auto output : outputs)
    {
        fed->pubs.emplace_back(&(fed->fed), output, helics::helicsType_t::helicsDouble);
    }
    return fed;
}
