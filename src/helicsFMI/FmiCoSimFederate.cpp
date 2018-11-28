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
#include "fmi/fmi_import/fmiObjects.h"
#include <algorithm>

FmiCoSimFederate::FmiCoSimFederate (std::shared_ptr<fmi2CoSimObject> obj, const helics::FederateInfo &fi)
    : cs (std::move (obj)), fed (obj->getName (), fi)
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

void FmiCoSimFederate::run (helics::Time step, helics::Time stop)
{
    auto &def = cs->fmuInformation ().getExperiment ();

    if (stop <= helics::timeZero)
    {
        stop = def.stopTime;
    }
    if (stop <= helics::timeZero)
    {
        stop = 30.0;
    }

    if (step <= helics::timeZero)
    {
        step = def.stepSize;
    }
    if (step <= helics::timeZero)
    {
        auto tstep = fed.getTimeProperty (helics_property_time_period);
        if (tstep > helics::timeEpsilon)
        {
            step = tstep;
        }
        else
        {
            step = std::min (0.2, static_cast<double> (stop) / 100.0);
        }
    }
    fed.setTimeProperty (helics_property_time_period, step);

    fed.enterInitializingMode ();
    cs->setMode (fmuMode::initializationMode);
    std::vector<fmi2Real> outputs (pubs.size ());
    std::vector<fmi2Real> inp (inputs.size ());
    cs->getOutputs (outputs.data ());
    for (size_t ii = 0; ii < pubs.size (); ++ii)
    {
        pubs[ii].publish (outputs[ii]);
    }
    cs->getCurrentInputs (inp.data ());
    for (size_t ii = 0; ii < inputs.size (); ++ii)
    {
        inputs[ii].setDefault (inp[ii]);
    }
    auto result = fed.enterExecutingMode (helics::iteration_request::iterate_if_needed);
    if (result == helics::iteration_result::iterating)
    {
        for (size_t ii = 0; ii < inputs.size (); ++ii)
        {
            inp[ii] = inputs[ii].getValue<fmi2Real> ();
        }
        cs->setInputs (inp.data ());
        fed.enterExecutingMode ();
    }
    cs->setMode (fmuMode::stepMode);

    helics::Time currentTime = helics::timeZero;
    while (currentTime <= stop)
    {
        cs->doStep (static_cast<double> (currentTime), static_cast<double> (step), true);
        currentTime = fed.requestNextStep ();
        // get the values to publish
        cs->getOutputs (outputs.data ());
        for (size_t ii = 0; ii < pubs.size (); ++ii)
        {
            pubs[ii].publish (outputs[ii]);
        }
        // load the inputs
        for (size_t ii = 0; ii < inputs.size (); ++ii)
        {
            inp[ii] = inputs[ii].getValue<fmi2Real> ();
        }
        cs->setInputs (inp.data ());
    }
    fed.finalize ();
}
