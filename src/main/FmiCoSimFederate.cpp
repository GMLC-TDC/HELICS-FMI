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

FmiCoSimFederate::FmiCoSimFederate (std::shared_ptr<fmi2CoSimObject> obj, const helics::FederateInfo &fi)
    : cs (std::move (obj)), fed (std::string (), fi)
{
    if (cs)
    {
        auto inputNames = cs->getInputNames ();
        for (auto input : inputNames)
        {
            inputs.emplace_back (&fed, input);
        }

        auto outputs = cs->getOutputNames ();
        for (auto output : outputs)
        {
            pubs.emplace_back (&fed, output, helics::helics_type_t::helicsDouble);
        }
    }
}

void FmiCoSimFederate::run ()
{
    fed.enterInitializingMode ();
    cs->setMode (fmuMode::initializationMode);
    std::vector<fmi2Real> outputs (pubs.size ());
    std::vector<fmi2Real> inp (inputs.size ());
    cs->getOutputs (outputs.data ());
    for (int ii = 0; ii < pubs.size (); ++ii)
    {
        pubs[ii].publish (outputs[ii]);
    }

    auto result = fed.enterExecutingMode (helics::iteration_request::iterate_if_needed);
    if (result == helics::iteration_result::iterating)
    {
        for (int ii = 0; ii < inputs.size (); ++ii)
        {
            inp[ii] = inputs[ii].getValue<fmi2Real> ();
        }
        cs->setInputs (inp.data ());
        fed.enterExecutingMode ();
    }
    cs->setMode (fmuMode::stepMode);
	
	auto &def = cs->fmuInformation().getExperiment();
	
}
