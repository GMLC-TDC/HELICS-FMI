/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "exeTestHelper.h"
#include "helics-fmi/helics-fmi-config.h"
#include "helicsFmiRunner.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

static const std::string inputDir = std::string(TEST_DIR) + "/";


TEST(exeTests, nonExistingFile)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runAsync(std::string("--autobroker ") + inputDir + "nonExistent.fmu");
    EXPECT_NE(out.get(), 0);
}

TEST(exeTests, invalidZip)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runAsync(std::string("--autobroker ") + inputDir + "dummy.fmu");
    EXPECT_NE(out.get(), 0);
}

TEST(exeTests, invalidFMU)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runAsync(std::string("--autobroker ") + inputDir + "validZip.fmu");
    EXPECT_NE(out.get(), 0);
}

TEST(exeTests, missingSO)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runAsync(std::string("--autobroker ") + inputDir + "missingSO.fmu");
    EXPECT_NE(out.get(), 0);
}
