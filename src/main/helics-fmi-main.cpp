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

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <vector>
#include "helics/helics.hpp"
#include "helics-fmi-config.h"
#include "fmi_import/fmiImport.h"
#include "helics/core/helicsVersion.hpp"
#include "helics-fmi.h"
#include "utilities/argParser.h"

namespace filesystem = boost::filesystem;

static const utilities::ArgDescriptors fmiArgs{
    {"stop", "the time to stop the fmi" },
{"integrator", "the type of integrator to use (cvode,arkode,boost" },
{"integrator-args", "arguments to pass to the integrator" }
};

int main(int argc, char *argv[])
{
    std::ifstream infile;
    utilities::variable_map vm;
    auto res=argumentParser(argc, argv, vm,fmiArgs,"input");
    if (res == utilities::helpReturn)
    {
        helics::FederateInfo().loadInfoFromArgs(argc, argv);
        return 0;
    }
    else if (res == utilities::versionReturn)
    {
        std::cout << "HELICS VERSION " << helics::helicsVersionString() << '\n';
        std::cout << "HELICS_FMI_VERSION " << HELICS_FMI_MAJOR << "." << HELICS_FMI_MINOR << "." << HELICS_FMI_PATCH << " (" << HELICS_FMI_DATE << ")\n";
        return (0);
    }
    // check to make sure we have some input file or the capture is specified
    if (vm.count("input") == 0)
    {
        std::cerr << "no input file specified\n";
        return (-3);
    }

    std::string filename = vm["input"].as<std::string>();
    if (!filesystem::exists(filename))
    {
        std::cerr << "input file " << filename << " does not exist\n";
        return (-2);
    }
    auto ext = filesystem::path(filename).extension().string();

    fmiLibrary fmi;
    if ((ext == ".fmu")||(ext==".FMU"))
    {
        fmi.loadFMU(filename);
        auto obj = fmi.createCoSimulationObject("obj1");
        if (!obj)
        {
            std::cerr << "helics-fmi only support co-simulation at this time\n";
        }
        helics::FederateInfo fi("fmi");
        fi.loadInfoFromArgs(argc, argv);
        auto fed = createFmiValueFederate(obj.get(),fi );

    }
    else if (ext == ".json")
    {

    }
    else if (ext == ".xml")
    {

    }
}