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

#include "CLI11/CLI11.hpp"
#include "fmi/fmi_import/fmiImport.h"
#include "formatInterpreters/jsonReaderElement.h"
#include "helics-fmi/helics-fmi-config.h"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/helicsVersion.hpp"
#include "helicsFMI/FmiCoSimFederate.hpp"
#include "helicsFMI/FmiModelExchangeFederate.hpp"
#include "utilities/argParser.h"
#include <iostream>
#include <boost/filesystem.hpp>

namespace filesystem = boost::filesystem;

void runSystem (readerElement &elem);

static const utilities::ArgDescriptors fmiArgs{
  {"integrator", "the type of integrator to use (cvode,arkode,boost)"},
  {"step", "the step size to use (specified in seconds or as a time string (10ms)"},
  {"stop", "the stop time to use (specified in seconds or as a time string (10ms)"},
  {"integrator-args", "arguments to pass to the integrator"}};

int main (int argc, char *argv[])
{
    std::ifstream infile;
    CLI::App app{"HELICS-FMI for loading and executing FMU's with HELICS"};
    app.add_flag_function ("-v,--version",
                           [](size_t) {
                               std::cout << "HELICS VERSION " << helics::versionString << '\n';
                               std::cout << "HELICS_FMI_VERSION " << HELICS_FMI_VERSION_STRING << '\n';
                               throw (CLI::Success ());
                           },
                           "specify the versions of helics and helics-fmi");

    std::string integrator;
    app.add_option ("--integrator", integrator, "the type of integrator to use(cvode, arkode, boost)");
    std::vector<std::string> inputs;
    app.add_option ("input,--input", inputs, "specify the input files")->check (CLI::ExistingFile);
    std::string integratorArgs;
    app.add_option ("--integrator-args", integratorArgs, "arguments to pass to the integrator");

    std::string stepTimeString;
    std::string stopTimeString;

    app.add_option ("--step", stepTimeString,
                    "the step size to use (specified in seconds or as a time string (10ms)");
    app.add_option ("--step", stopTimeString,
                    "the step size to use (specified in seconds or as a time string (10ms)");

    app.set_help_flag ("-h,-?,--help", "print this help module");
    app.allow_extras ();
    app.set_config ("--config-file");
    CLI11_PARSE (app, argc, argv);

    helics::Time stepTime = helics::loadTimeFromString (stepTimeString);
    helics::Time stopTime = helics::loadTimeFromString (stopTimeString);

    // check to make sure we have some input file or the capture is specified
    if (inputs.empty ())
    {
        std::cerr << "no input file specified\n";
        return (-3);
    }

    std::string filename = inputs.front ();

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
            fed->run (helics::timeZero, helics::timeZero);
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
        jsonReaderElement system (filename);
        if (system.isValid ())
        {
            runSystem (system);
        }
    }
    else if ((ext == ".toml") || (ext == ".TOML"))
    {
    }
    else if ((ext == ".xml") || (ext == ".XML"))
    {
    }
}

void runSystem (readerElement &elem)
{
    elem.moveToFirstChild ("fmus");
    std::vector<std::unique_ptr<FmiCoSimFederate>> feds;
    auto core = helics::CoreFactory::create (helics::core_type::TEST, "--name=fmu_core --autobroker");
    while (elem.isValid ())
    {
        fmiLibrary fmi;
        helics::FederateInfo fi;
        fi.defName = "fmi";
        fi.coreName = "fmu_core";
        auto str = elem.getAttributeText ("fmu");
        fmi.loadFMU (str);
        std::shared_ptr<fmi2CoSimObject> obj = fmi.createCoSimulationObject (elem.getAttributeText ("name"));
        auto fed = std::make_unique<FmiCoSimFederate> (obj, fi);
        feds.push_back (std::move (fed));
        elem.moveToNextSibling ("fmus");
    }
    elem.moveToParent ();
    elem.moveToFirstChild ("connections");
    while (elem.isValid ())
    {
        auto str1 = elem.getFirstAttribute ().getText ();
        auto str2 = elem.getNextAttribute ().getText ();
        elem.moveToNextSibling ();
        core->dataLink (str1, str2);
    }
    feds.clear ();
    core->disconnect ();
}
