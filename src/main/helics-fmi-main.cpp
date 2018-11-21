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

#include "helicsFMI/FmiCoSimFederate.hpp"
#include "helicsFMI/FmiModelExchangeFederate.hpp"
#include "fmi/fmi_import/fmiImport.h"
#include "helics-fmi/helics-fmi-config.h"
#include "helics/core/helicsVersion.hpp"
#include "utilities/argParser.h"
#include <iostream>
#include <boost/filesystem.hpp>

namespace filesystem = boost::filesystem;

static const utilities::ArgDescriptors fmiArgs{
  {"stop", "the time to stop the fmi"},
  {"integrator", "the type of integrator to use (cvode,arkode,boost)"},
  {"step", "the step size to use (specified in seconds or as a time string (10ms)"},
  {"stop", "the stop time to use (specified in seconds or as a time string (10ms)"},
  {"integrator-args", "arguments to pass to the integrator"}};

int main (int argc, char *argv[])
{
    std::ifstream infile;
    utilities::variable_map vm;
    auto res = argumentParser (argc, argv, vm, fmiArgs, "input");
    if (res == utilities::helpReturn)
    {
        helics::FederateInfo ().loadInfoFromArgs (argc, argv);
        return 0;
    }
    else if (res == utilities::versionReturn)
    {
        std::cout << "HELICS VERSION " << helics::versionString << '\n';
        std::cout << "HELICS_FMI_VERSION " << HELICS_FMI_VERSION_STRING << '\n';
        return (0);
    }
    // check to make sure we have some input file or the capture is specified
    if (vm.count ("input") == 0)
    {
        std::cerr << "no input file specified\n";
        return (-3);
    }

    std::string filename = vm["input"].as<std::string> ();
    if (!filesystem::exists (filename))
    {
        std::cerr << "input file " << filename << " does not exist\n";
        return (-2);
    }
    auto ext = filesystem::path (filename).extension ().string ();

    fmiLibrary fmi;
    if ((ext == ".fmu") || (ext == ".FMU"))
    {
        helics::FederateInfo fi;
        fi.defName = "fmi";
        fi.loadInfoFromArgs (argc, argv);
        fmi.loadFMU (filename);
        if (fmi.checkFlag (fmuCapabilityFlags::coSimulationCapable))
        {
            std::shared_ptr<fmi2CoSimObject> obj = fmi.createCoSimulationObject ("obj1");
            auto fed = std::make_unique<FmiCoSimFederate> (obj, fi);
            fed->run (helics::timeZero,helics::timeZero);
        }
        else
        {
            std::shared_ptr<fmi2ModelExchangeObject> obj = fmi.createModelExchangeObject ("obj1");
            auto fed = std::make_unique<FmiModelExchangeFederate> (obj, fi);
            fed->run (helics::timeZero, helics::timeZero);
        }
    }
    else if ((ext == ".json") || (ext == ".JSON"))
    {

    }
	else if ((ext == ".toml") || (ext == ".TOML"))
	{

	}
    else if ((ext == ".xml") || (ext == ".XML"))
    {

    }
}