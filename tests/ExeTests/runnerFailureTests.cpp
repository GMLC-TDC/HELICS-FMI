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

static const std::string inputFile = std::string(FMI_REFERENCE_DIR) + "Resource.fmu";

TEST(runnerTests, flagCheckFail)
{
    FmiRunner runner;
    int ret = runner.parse(std::string("--autobroker --set unknown=45.6 ") + inputFile);
    EXPECT_EQ(ret, 0);
    ret = runner.load();
    EXPECT_EQ(ret, 0);
    std::cout << "running" << std::endl;
    ret = runner.run();
    EXPECT_NE(ret, 0);
    std::cout << "run failed, now closing" << std::endl;
    runner.close();
    std::cout << "closed now cleanup" << std::endl;
    helics::cleanupHelicsLibrary();
    std::cout << "finished" << std::endl;
}

TEST(runnerTests, flagCheckPass)
{
    FmiRunner runner;
    int ret = runner.parse(
        std::string("--autobroker --set unknown=45.6 --flags=-exception_on_discard ") + inputFile);
    EXPECT_EQ(ret, 0);
    ret = runner.load();
    EXPECT_EQ(ret, 0);
    ret = runner.run();
    EXPECT_EQ(ret, 0);
    runner.close();
    helics::cleanupHelicsLibrary();
}

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
