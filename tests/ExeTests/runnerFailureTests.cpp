/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics-fmi/helics-fmi-config.h"
#include "helicsFmiRunner.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

static const std::string inputDir = std::string(TEST_DIR) + "/";

using helicsfmi::FmiRunner;

TEST(runnerTests, NoFile)
{
    FmiRunner runner;
    const int ret = runner.parse("--autobroker ");
    EXPECT_NE(ret, 0);
    runner.close();
    helics::cleanupHelicsLibrary();
}

TEST(runnerTests, nonExistingFile)
{
    FmiRunner runner;
    const int ret = runner.parse(std::string("--autobroker ") + inputDir + "nonExistent.fmu");
    EXPECT_EQ(ret, 0);
    const int ret2 = runner.load();
    EXPECT_NE(ret, ret2);
    runner.close();
    helics::cleanupHelicsLibrary();
}

TEST(runnerTests, invalidZip)
{
    FmiRunner runner;
    int ret = runner.parse(std::string("--autobroker ") + inputDir + "dummy.fmu");
    EXPECT_EQ(ret, 0);
    ret = runner.load();
    EXPECT_NE(ret, 0);
    EXPECT_EQ(ret, FmiRunner::INVALID_FMU);
    runner.close();
    helics::cleanupHelicsLibrary();
}

TEST(runnerTests, invalidFMU)
{
    FmiRunner runner;
    int ret = runner.parse(std::string("--autobroker ") + inputDir + "validZip.fmu");
    EXPECT_EQ(ret, 0);
    ret = runner.load();
    EXPECT_NE(ret, 0);
    runner.close();
    helics::cleanupHelicsLibrary();
}

TEST(runnerTests, missingSO)
{
    FmiRunner runner;
    int ret = runner.parse(std::string("--autobroker ") + inputDir + "missingSO.fmu");
    EXPECT_EQ(ret, 0);
    ret = runner.load();
    EXPECT_NE(ret, 0);
    runner.close();
    helics::cleanupHelicsLibrary();
}
