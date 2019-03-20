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
#include "formatInterpreters/tinyxml2ReaderElement.h"
#include "formatInterpreters/tomlReaderElement.h"
#include "helics-fmi/helics-fmi-config.h"
#include "helics/apps/BrokerApp.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/helicsVersion.hpp"
#include "helicsFMI/FmiCoSimFederate.hpp"
#include "helicsFMI/FmiModelExchangeFederate.hpp"
#include <iostream>

void runSystem(readerElement &elem, helics::FederateInfo &fi);

int main(int argc, char *argv[])
{
    std::ifstream infile;
    CLI::App app{"HELICS-FMI for loading and executing FMU's with HELICS", "helics-fmi"};
    app.add_flag_function(
      "-v,--version",
      [](size_t) {
          std::cout << "HELICS VERSION " << helics::versionString << '\n';
          std::cout << "HELICS_FMI_VERSION " << HELICS_FMI_VERSION_STRING << '\n';
          throw(CLI::Success());
      },
      "specify the versions of helics and helics-fmi");

    std::string integrator{"cvode"};
    app.add_option("--integrator", integrator, "the type of integrator to use(cvode, arkode, boost)", true)
      ->transform(CLI::IsMember({"cvode", "arkode", "boost"}));
    std::vector<std::string> inputs;
    app.add_option("input,--input", inputs, "specify the input files")->check(CLI::ExistingFile);
    std::string integratorArgs;
    app.add_option("--integrator-args", integratorArgs, "arguments to pass to the integrator");

    std::string stepTimeString;
    std::string stopTimeString;

    app.add_option("--step", stepTimeString,
                   "the step size to use (specified in seconds or as a time string (10ms)");
    app.add_option("--stop", stopTimeString,
                   "the time to stop the simulation (specified in seconds or as a time string (10ms)");

    std::string brokerArgs;
    app.add_option("--brokerargs", brokerArgs, "arguments to pass to an automatically generated broker");
    app.set_help_flag("-h,-?,--help", "print this help module");
    app.allow_extras();
    app.set_config("--config-file");
    std::vector<std::string> output_variables;
    std::vector<std::string> input_variables;
    std::vector<std::string> connections;

    app.add_option("--output_variables", output_variables, "Specify outputs of the FMU by name")
      ->ignore_underscore()
      ->delimiter(',');
    app.add_option("--input_variables", input_variables, "Specify the input variables of the FMU by name")
      ->ignore_underscore()
      ->delimiter(',');
    app.add_option("--connections", input_variables, "Specify connections this FMU should make")->delimiter(',');

    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::CallForHelp &e)
    {
        auto ret = app.exit(e);
        helics::FederateInfo fi(argc, argv);
        return ret;
    }
    catch (const CLI::ParseError &e)
    {
        return app.exit(e);
    }

    helics::Time stepTime = helics::loadTimeFromString(stepTimeString);
    helics::Time stopTime = helics::loadTimeFromString(stopTimeString);

    helics::FederateInfo fi;
    // set the default core type to be local
    fi.coreType = helics::core_type::TEST;
    fi.defName = "fmi";
    fi.loadInfoFromArgs(argc, argv);

    std::unique_ptr<helics::apps::BrokerApp> broker;
    if (fi.autobroker)
    {
        broker = std::make_unique<helics::apps::BrokerApp>(fi.coreType, brokerArgs);
        fi.autobroker = false;
    }

    std::string filename = inputs.front();

    auto ext = filename.substr(filename.find_last_of('.'));

    fmiLibrary fmi;
    if ((ext == ".fmu") || (ext == ".FMU"))
    {
        fmi.loadFMU(filename);
        if (fmi.checkFlag(fmuCapabilityFlags::coSimulationCapable))
        {
            std::shared_ptr<fmi2CoSimObject> obj = fmi.createCoSimulationObject("obj1");
            auto fed = std::make_unique<FmiCoSimFederate>(obj, fi);
            fed->configure(stepTime);
            fed->run(stopTime);
        }
        else
        {
            std::shared_ptr<fmi2ModelExchangeObject> obj = fmi.createModelExchangeObject("obj1");
            auto fed = std::make_unique<FmiModelExchangeFederate>(obj, fi);
            fed->configure(stepTime);
            fed->run(stopTime);
        }
    }
    else if ((ext == ".json") || (ext == ".JSON"))
    {
        jsonReaderElement system(filename);
        if (system.isValid())
        {
            runSystem(system, fi);
        }
    }
    else if ((ext == ".toml") || (ext == ".TOML"))
    {
        tomlReaderElement system(filename);
        if (system.isValid())
        {
            runSystem(system, fi);
        }
    }
    else if ((ext == ".xml") || (ext == ".XML"))
    {
        tinyxml2ReaderElement system(filename);
        if (system.isValid())
        {
            runSystem(system, fi);
        }
    }
}

void runSystem(readerElement &elem, helics::FederateInfo &fi)
{
    fi.coreName = "fmu_core";
    elem.moveToFirstChild("fmus");
    std::vector<std::unique_ptr<FmiCoSimFederate>> feds;

    auto core =
      helics::CoreFactory::create(fi.coreType, "--name=fmu_core " + helics::generateFullCoreInitString(fi));
    while (elem.isValid())
    {
        fmiLibrary fmi;
        auto str = elem.getAttributeText("fmu");
        fmi.loadFMU(str);
        std::shared_ptr<fmi2CoSimObject> obj = fmi.createCoSimulationObject(elem.getAttributeText("name"));
        auto fed = std::make_unique<FmiCoSimFederate>(obj, fi);
        feds.push_back(std::move(fed));
        elem.moveToNextSibling("fmus");
    }
    elem.moveToParent();
    elem.moveToFirstChild("connections");
    while (elem.isValid())
    {
        auto str1 = elem.getFirstAttribute().getText();
        auto str2 = elem.getNextAttribute().getText();
        elem.moveToNextSibling();
        core->dataLink(str1, str2);
    }

    feds.clear();
    core->disconnect();
}
