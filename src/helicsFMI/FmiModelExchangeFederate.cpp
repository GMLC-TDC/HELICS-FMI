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

#include "FmiModelExchangeFederate.hpp"
#include "fmi_import/fmiObjects.h"
#include "OdeSolverBase.hpp"

FmiModelExchangeFederate::FmiModelExchangeFederate(std::shared_ptr<fmi2ModelExchangeObject> obj, const helics::FederateInfo &fi):me(std::move(obj)), fed(std::string(),fi)
{
	if (me)
	{
        auto inputNames = me->getInputNames ();
        for (auto input : inputNames)
        {
            inputs.emplace_back (&fed, input);
        }

        auto outputs = me->getOutputNames ();
        for (auto output : outputs)
        {
            pubs.emplace_back (&fed, output, helics::helics_type_t::helicsDouble);
        }
	}
    
}


FmiModelExchangeFederate::~FmiModelExchangeFederate() = default;

void FmiModelExchangeFederate::run (helics::Time step, helics::Time stop)
{
		
}