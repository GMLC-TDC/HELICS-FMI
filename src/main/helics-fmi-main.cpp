/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/external/CLI11/CLI11.hpp"
#include "helicsFmiRunner.hpp"

#include <filesystem>
#include <iostream>
#include <thread>

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, char* argv[])
{
    helicsfmi::FmiRunner runner;
    auto app = runner.generateCLI();
    CLI11_PARSE(*app, argc, argv);
    int ret = runner.load();
    if (ret != 0) {
        return ret;
    }

    ret = runner.initialize();
    if (ret != 0) {
        return ret;
    }
    ret = runner.run();
    if (ret != 0) {
        return ret;
    }
    ret = runner.close();
    return ret;
}
