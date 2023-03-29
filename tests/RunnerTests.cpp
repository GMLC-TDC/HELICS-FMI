/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics-fmi/helics-fmi-config.h"
#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/application_api/queryFunctions.hpp"
#include "helicsFmiRunner.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <future>
#include <thread>

// using ::testing::HasSubstr;

static const std::string bballFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
static const std::string ftFile = std::string(FMI_REFERENCE_DIR) + "Feedthrough.fmu";

using helicsfmi::FmiRunner;

TEST(runnerTests, singleFed)
{
    FmiRunner runner;
    runner.parse(std::string("--autobroker ") + bballFile);
    /**test that things run to completion with auto broker*/
    int ret = runner.load();
    ASSERT_EQ(ret, 0);
    ret = runner.initialize();
    ASSERT_EQ(ret, 0);
    ret = runner.run();
    ASSERT_EQ(ret, 0);
    ret = runner.close();
    EXPECT_EQ(ret, 0);
}

TEST(runnerTests, singleFedAsyncZMQ)
{
    FmiRunner runner;
    runner.parse(std::string("--autobroker --core=zmq ") + ftFile);
    /**test that things run to completion with auto broker*/
    int ret = runner.load();
    ASSERT_EQ(ret, 0);
    ret = runner.initialize();
    ASSERT_EQ(ret, 0);
    ret = runner.run();
    ASSERT_EQ(ret, 0);
    ret = runner.close();
    EXPECT_EQ(ret, 0);
}
//
TEST(runnerTests, dualFedZMQ)
{
    FmiRunner runner;
    runner.parse(
        std::string(
            "--autobroker --coretype=zmq --step=0.1 --stop=2.0 --name=ftfed --brokerargs=\"-f2 --force\" ") +
        ftFile);
    int ret = runner.load();
    ASSERT_EQ(ret, 0);
    ret = runner.initialize();
    ASSERT_EQ(ret, 0);

    auto fut = runner.runAsync();

    helics::ValueFederate vFed("fed1", "--coretype=zmq --forcenewcore");

    vFed.enterInitializingModeIterative();

    auto qres = helics::vectorizeQueryResult(vFed.query("ftfed", "publications"));

    ASSERT_EQ(qres.size(), 4U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);

    qres = helics::vectorizeQueryResult(vFed.query("ftfed", "inputs"));

    EXPECT_EQ(qres.size(), 4U);

    vFed.enterExecutingMode();
    auto time1 = vFed.requestTime(2.0);
    EXPECT_LT(time1, 2.0);

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    EXPECT_NE(val, -20.0);
    EXPECT_NE(val2, -20.0);
    vFed.finalize();
    auto str = fut.get();
    EXPECT_EQ(str, 0);
}

static const std::string inputDir = std::string(TEST_DIR) + "/";

TEST(runnerTests, nonExistingFile)
{
    FmiRunner runner;
    const int ret = runner.parse(std::string("--autobroker ") + inputDir + "nonExistent.fmu");
    EXPECT_NE(ret, 0);
    const int ret2 = runner.load();
    EXPECT_EQ(ret, ret2);
}

TEST(runnerTests, invalidZip)
{
    FmiRunner runner;
    int ret = runner.parse(std::string("--autobroker ") + inputDir + "dummy.fmu");
    EXPECT_EQ(ret, 0);
    ret = runner.load();
    EXPECT_NE(ret, 0);
    EXPECT_EQ(ret, FmiRunner::INVALID_FMU);
}

TEST(runnerTests, invalidFMU)
{
    FmiRunner runner;
    int ret = runner.parse(std::string("--autobroker ") + inputDir + "validZip.fmu");
    EXPECT_EQ(ret, 0);
    ret = runner.load();
    EXPECT_NE(ret, 0);
}

TEST(runnerTests, missingSO)
{
    FmiRunner runner;
    int ret = runner.parse(std::string("--autobroker ") + inputDir + "missingSO.fmu");
    EXPECT_EQ(ret, 0);
    ret = runner.load();
    EXPECT_NE(ret, 0);
}


TEST(runnerTests, setfield)
{
    FmiRunner runner;
    runner.parse(
        std::string(
            "--autobroker --coretype=zmq --step=0.1 --stop=2.0 --name=bbfed --set h=4 --brokerargs=\"-f2 --force\" ") +
        bballFile);
    int ret = runner.load();
    ASSERT_EQ(ret, 0);
    ret = runner.initialize();
    ASSERT_EQ(ret, 0);

    auto fut = runner.runAsync();

    helics::ValueFederate vFed("fed1", "--coretype=zmq --forcenewcore");

    vFed.enterInitializingModeIterative();

    auto qres = helics::vectorizeQueryResult(vFed.query("bbfed", "publications"));

    ASSERT_EQ(qres.size(), 2U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);


    vFed.enterExecutingMode();
    auto time1 = vFed.requestTime(2.0);
    EXPECT_LT(time1, 2.0);

    auto val = sub1.getValue<double>();
    EXPECT_LT(val, 4.0);
    EXPECT_GT(val, 3.5);
    vFed.finalize();
    auto str = fut.get();
    EXPECT_EQ(str, 0);
}


TEST(runnerTests, setfield2)
{
    FmiRunner runner;
    runner.parse(
        std::string(
            "--autobroker --coretype=zmq --step=0.1 --stop=2.0 --name=bbfed --set h=5;v=2 --brokerargs=\"-f2 --force\" ") +
        bballFile);
    int ret = runner.load();
    ASSERT_EQ(ret, 0);
    ret = runner.initialize();
    ASSERT_EQ(ret, 0);

    auto fut = runner.runAsync();

    helics::ValueFederate vFed("fed1", "--coretype=zmq --forcenewcore");

    vFed.enterInitializingModeIterative();

    auto qres = helics::vectorizeQueryResult(vFed.query("bbfed", "publications"));

    ASSERT_EQ(qres.size(), 2U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);


    vFed.enterExecutingMode();
    auto time1 = vFed.requestTime(2.0);
    EXPECT_LT(time1, 2.0);

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    EXPECT_GT(val, 5.0);
    EXPECT_LT(val2, 2.0);
    vFed.finalize();
    auto str = fut.get();
    EXPECT_EQ(str, 0);
}
