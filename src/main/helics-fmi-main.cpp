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
#include "helics-fmi.h"
namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

void argumentParser(int argc, const char *const *argv, po::variables_map &vm_map);
void loadInfoFromArgs(helics::FederateInfo &fi, int argc, const char * const *argv);
void argumentParserFI(int argc, const char *const *argv, po::variables_map &vm_map);

int main(int argc, char *argv[])
{
    std::ifstream infile;
    po::variables_map vm;
    argumentParser(argc, argv, vm);

    // check to make sure we have some input file or the capture is specified
    if (vm.count("input") == 0)
    {
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
    if (ext == ".fmu")
    {
        fmi.loadFMU(filename);
        auto obj = fmi.createCoSimulationObject("obj1");
        if (!obj)
        {
            std::cerr << "helics-fmi only support co-simulation at this time\n";
        }
        helics::FederateInfo fi;
        loadInfoFromArgs(fi, argc, argv);
        auto fed = createFmiValueFederate(obj.get(),fi );

    }
    else if (ext == ".json")
    {

    }
    else if (ext == ".xml")
    {

    }
}

void argumentParser(int argc, const char *const *argv, po::variables_map &vm_map)
{
    po::options_description cmd_only("command line only");
    po::options_description config("configuration");
    po::options_description hidden("hidden");

    // clang-format off
    // input boost controls
    cmd_only.add_options()
        ("help,h", "produce help message")
        ("version,v", "helics version number")
        ("config-file", po::value<std::string>(), "specify a configuration file to use");


    config.add_options()
        ("stop", po::value<double>(), "the time to stop recording")
        ("integrator", po::value<std::string>(), "the type of integrator to use (cvode,arkode,boost")
        ("integrator-args", po::value<std::string>(), "arguments to pass to the integrator");

    hidden.add_options() ("input", po::value<std::string>(), "input file can be an fmu, json, or xml file");
    // clang-format on

    po::options_description cmd_line("command line options");
    po::options_description config_file("configuration file options");
    po::options_description visible("allowed options");

    cmd_line.add(cmd_only).add(config).add(hidden);
    config_file.add(config).add(hidden);
    visible.add(cmd_only).add(config);

    po::positional_options_description p;
    p.add("input", -1);

    po::variables_map cmd_vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(cmd_line).positional(p).run(), cmd_vm);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        throw (e);
    }

    po::notify(cmd_vm);

    // objects/pointers/variables/constants

    // program options control
    if (cmd_vm.count("help") > 0)
    {
        std::cout << visible << '\n';
        return;
    }

    if (cmd_vm.count("version") > 0)
    {
        std::cout << "HELICS VERSION " << helics::getHelicsVersionString() << '\n';
        std::cout << "HELICS_FMI_VERSION " << HELICS_FMI_MAJOR << "." << HELICS_FMI_MINOR << "." << HELICS_FMI_PATCH << " (" << HELICS_FMI_DATE << ")\n";
        return;
    }

    po::store(po::command_line_parser(argc, argv).options(cmd_line).positional(p).run(), vm_map);

    if (cmd_vm.count("config-file") > 0)
    {
        std::string config_file_name = cmd_vm["config-file"].as<std::string>();
        if (!filesystem::exists(config_file_name))
        {
            std::cerr << "config file " << config_file_name << " does not exist\n";
            throw (std::invalid_argument("unknown config file"));
        }
        else
        {
            std::ifstream fstr(config_file_name.c_str());
            po::store(po::parse_config_file(fstr, config_file), vm_map);
            fstr.close();
        }
    }

    po::notify(vm_map);
    if (vm_map.count("input") == 0)
    {
        std::cerr << " no input file specified exiting program\n";
        std::cerr << visible << '\n';
        return;
    }
}


void loadInfoFromArgs(helics::FederateInfo &fi, int argc, const char * const *argv)
{
    po::variables_map vm;
    argumentParser(argc, argv, vm);
    std::string name;
    if (vm.count("name") > 0)
    {
        name = vm["name"].as<std::string>();
    }
    std::string corename;
    if (vm.count("core") > 0)
    {
        fi.coreName = vm["core"].as<std::string>();
    }

    fi.coreType = helics::coreTypeFromString(corename);

    fi.coreInitString = "1";
    if (vm.count("coreinit") > 0)
    {
        fi.coreInitString.push_back(' ');
        fi.coreInitString = vm["coreinit"].as<std::string>();
    }
    if (vm.count("broker") > 0)
    {
        fi.coreInitString += " --broker=";
        fi.coreInitString += vm["broker"].as<std::string>();
    }

    if (vm.count("timedelta") > 0)
    {
        fi.timeDelta = vm["timedelta"].as<double>();
    }

    if (vm.count("period") > 0)
    {
        fi.period = vm["period"].as<double>();
    }

    if (vm.count("offset") > 0)
    {
        fi.offset = vm["offset"].as<double>();
    }
}

void argumentParserFI(int argc, const char *const *argv, po::variables_map &vm_map)
{
    po::options_description cmd_only("command line only");
    po::options_description config("configuration");
    po::options_description hidden("hidden");

    // clang-format off
    // input boost controls
    cmd_only.add_options()
        ("help,h", "produce help message")
        ("version,v", "helics version number")
        ("config-file", po::value<std::string>(), "specify a configuration file to use");


    config.add_options()
        ("broker,b", po::value<std::string>(), "address of the broker to connect")
        ("name,n", po::value<std::string>(), "name of the player federate")
        ("core,c", po::value<std::string>(), "type of the core to connect to")
        ("offset", po::value<double>(), "the offset of the time steps")
        ("period", po::value<double>(), "the period of the federate")
        ("timedelta", po::value<double>(), "the time delta of the federate")
        ("coreinit,i", po::value<std::string>(), "the core initialization string");


    hidden.add_options() ("input", po::value<std::string>(), "input file");
    // clang-format on

    po::options_description cmd_line("command line options");
    po::options_description config_file("configuration file options");
    po::options_description visible("allowed options");

    cmd_line.add(cmd_only).add(config).add(hidden);
    config_file.add(config).add(hidden);
    visible.add(cmd_only).add(config);

    po::variables_map cmd_vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(cmd_line).run(), cmd_vm);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        throw (e);
    }

    po::notify(cmd_vm);

    // objects/pointers/variables/constants

    // program options control
    if (cmd_vm.count("help") > 0)
    {
        std::cout << visible << '\n';
        return;
    }

    if (cmd_vm.count("version") > 0)
    {
        std::cout << helics::getHelicsVersionString() << '\n';
        return;
    }

    po::store(po::command_line_parser(argc, argv).options(cmd_line).run(), vm_map);

    if (cmd_vm.count("config-file") > 0)
    {
        std::string config_file_name = cmd_vm["config-file"].as<std::string>();
        if (!filesystem::exists(config_file_name))
        {
            std::cerr << "config file " << config_file_name << " does not exist\n";
            throw (std::invalid_argument("unknown config file"));
        }
        else
        {
            std::ifstream fstr(config_file_name.c_str());
            po::store(po::parse_config_file(fstr, config_file), vm_map);
            fstr.close();
        }
    }

    po::notify(vm_map);

}