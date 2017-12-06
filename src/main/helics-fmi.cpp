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
#include "helics/ValueFederate.h"
#include "helics/Publications.hpp"
#include "helics/Subscriptions.hpp"
namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

void argumentParser(int argc, const char *const *argv, po::variables_map &vm_map);

int main(int argc, char *argv[])
{
    std::ifstream infile;
    po::variables_map vm;
    argumentParser(argc, argv, vm);
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
        ("broker,b", po::value<std::string>(), "address of the broker to connect")
        ("name,n", po::value<std::string>(), "name of the fmi federate")
        ("core,c", po::value<std::string>(), "type of the core to connect to")
        ("stop", po::value<double>(), "the time to stop recording")
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
        std::cout << helics::getHelicsVersionString() << '\n';
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
    // check to make sure we have some input file or the capture is specified
    if ((vm_map.count("input") == 0) && (vm_map.count("capture") == 0) && (vm_map.count("tags") == 0))
    {
        std::cerr << " no input file, tags, or captures specified\n";
        std::cerr << visible << '\n';
        return;
    }
}
