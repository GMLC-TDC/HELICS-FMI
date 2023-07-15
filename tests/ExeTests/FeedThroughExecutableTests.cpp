/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/
#include "FmiCoSimFederate.hpp"
#include "helics/application_api/helicsTypes.hpp"
#include "helics/application_api/queryFunctions.hpp"
#include "helicsFmiRunner.hpp"

#include "gtest/gtest.h"
#include <filesystem>
#include <fmt/format.h>
#include <future>
#include <thread>

static const std::string fmuName = "Feedthrough";
static const std::string inputFile = std::string(FMI_REFERENCE_DIR) + fmuName + ".fmu";

using helicsfmi::FmiRunner;

TEST(feedthrough, check)
{
    FmiRunner runner;
    runner.parse(
        std::string(
            "--autobroker --coretype=zmq --step=0.1s --stoptime=1.0s --name=fthru  --brokerargs=\"-f2 --name=ft1broker\" ") +
        inputFile);
    int ret = runner.load();
    ASSERT_EQ(ret, 0);
    ret = runner.initialize();
    ASSERT_EQ(ret, 0);

    auto result = runner.runAsync();

    const helics::FederateInfo fedInfo(helics::CoreType::ZMQ);

    helics::ValueFederate vFed("fed1", fedInfo);

    vFed.enterInitializingModeIterative();

    auto qres = helics::vectorizeQueryResult(vFed.query("root", "publications"));

    EXPECT_EQ(qres.size(), 5U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);
    auto& sub3 = vFed.registerSubscription(qres[2]);
    sub2.setDefault(-20.0);
    auto& sub4 = vFed.registerSubscription(qres[3]);
    sub2.setDefault(-20.0);

    qres = helics::vectorizeQueryResult(vFed.query("fthru", "inputs"));

    EXPECT_EQ(qres.size(), 5U);
    auto& pub1 = vFed.registerPublication<double>("");
    auto& pub2 = vFed.registerPublication<double>("");
    auto& pub3 = vFed.registerPublication<int>("");
    auto& pub4 = vFed.registerPublication<bool>("");

    pub1.addInputTarget(qres[0]);
    pub2.addInputTarget(qres[1]);
    pub3.addInputTarget(qres[2]);
    pub4.addInputTarget(qres[3]);

    vFed.enterInitializingMode();

    pub1.publish(13.56);
    pub2.publish(18.58);
    pub3.publish(998);
    pub4.publish(true);

    vFed.enterExecutingMode();

    auto time = vFed.requestTime(2.0);
    EXPECT_LT(time, 2.0);

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    auto val3 = sub3.getValue<int>();
    auto val4 = sub4.getValue<bool>();

    // make sure it is not default
    EXPECT_DOUBLE_EQ(val, 13.56);
    EXPECT_DOUBLE_EQ(val2, 18.58);
    EXPECT_EQ(val3, 998);
    EXPECT_TRUE(val4);

    vFed.finalize();
    result.get();
    runner.close();
}

TEST(feedthrough, CmdLineConnections)
{
    FmiRunner runner;
    runner.parse(
        std::string(
            "--autobroker --coretype=zmq --step=0.1s --stoptime=1.0s --name=fthru  --connections pub0,fthru.Float64_continuous_input,pub1,fthru.Float64_discrete_input --connections pub2 --connections=fthru.Int32_input --connections=pub3,fthru.Boolean_input --brokerargs=\"-f2 --name=ftconnbroker\" ") +
        inputFile);
    int ret = runner.load();
    ASSERT_EQ(ret, 0);
    ret = runner.initialize();
    ASSERT_EQ(ret, 0);

    auto result = runner.runAsync();

    helics::FederateInfo fedInfo(helics::CoreType::ZMQ);
    fedInfo.broker = "ftconnbroker";

    helics::ValueFederate vFed("fed1", fedInfo);

    vFed.enterInitializingModeIterative();

    EXPECT_EQ(vFed.getCurrentMode(),helics::Federate::Modes::STARTUP);
    auto qres = helics::vectorizeQueryResult(vFed.query("root", "publications"));

    EXPECT_EQ(qres.size(), 5U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);
    auto& sub3 = vFed.registerSubscription(qres[2]);
    sub2.setDefault(-20.0);
    auto& sub4 = vFed.registerSubscription(qres[3]);
    sub2.setDefault(-20.0);

    EXPECT_EQ(qres.size(), 5U);
    auto& pub1 = vFed.registerGlobalPublication<double>("pub0");
    auto& pub2 = vFed.registerGlobalPublication<double>("pub1");
    auto& pub3 = vFed.registerGlobalPublication<int>("pub2");
    auto& pub4 = vFed.registerGlobalPublication<bool>("pub3");

    vFed.enterInitializingMode();
    EXPECT_EQ(vFed.getCurrentMode(),helics::Federate::Modes::INITIALIZING);
    try
    {
        pub1.publish(13.56);
        pub2.publish(18.58);
        pub3.publish(998);
        pub4.publish(true);
    }
    catch (...)
    {
        EXPECT_TRUE(false)<<"Got error in publish";
    }

    vFed.enterExecutingMode();
    EXPECT_EQ(vFed.getCurrentMode(),helics::Federate::Modes::EXECUTING);
    auto time = vFed.requestTime(2.0);
    EXPECT_LT(time, 2.0);

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    auto val3 = sub3.getValue<int>();
    auto val4 = sub4.getValue<bool>();

    // make sure it is not default
    EXPECT_DOUBLE_EQ(val, 13.56);
    EXPECT_DOUBLE_EQ(val2, 18.58);
    EXPECT_EQ(val3, 998);
    EXPECT_TRUE(val4);

    vFed.finalize();
    result.get();
    runner.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

static constexpr const char* connectionFiles[] = {"example_connections1.json",
                                                  "example_connections1.toml",
                                                  "example_connections2.json",
                                                  "example_connections2.toml"};

class ConnectionFileTests: public ::testing::TestWithParam<const char*> {};

TEST_P(ConnectionFileTests, connections)
{
    static int index{0};
    std::string cfile = std::string(TEST_DIR) + "/" + GetParam();
    FmiRunner runner;
    runner.parse(fmt::format(
        "--autobroker --coretype=zmq --step=0.1s --stoptime=1.0s --name=fthru  --connections {} --brokerargs=\"-f2 --name=ftfbroker{}\" {}",
        cfile,
        index,
        inputFile));
    int ret = runner.load();
    ASSERT_EQ(ret, 0);
    ret = runner.initialize();
    ASSERT_EQ(ret, 0);

    auto result = runner.runAsync();

    helics::FederateInfo fedInfo(helics::CoreType::ZMQ);
    fedInfo.broker = fmt::format("ftfbroker{}", index);
    ++index;
    helics::ValueFederate vFed("fed1", fedInfo);

    vFed.enterInitializingModeIterative();

    auto qres = helics::vectorizeQueryResult(vFed.query("root", "publications"));

    EXPECT_EQ(qres.size(), 5U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);
    auto& sub3 = vFed.registerSubscription(qres[2]);
    sub2.setDefault(-20.0);
    auto& sub4 = vFed.registerSubscription(qres[3]);
    sub2.setDefault(-20.0);

    auto& pub1 = vFed.registerGlobalPublication<double>("pub0");
    auto& pub2 = vFed.registerGlobalPublication<double>("pub1");
    auto& pub3 = vFed.registerGlobalPublication<int>("pub2");
    auto& pub4 = vFed.registerGlobalPublication<bool>("pub3");

    vFed.enterInitializingMode();
    EXPECT_EQ(vFed.getCurrentMode(),helics::Federate::Modes::INITIALIZING);
    try
    {
        pub1.publish(13.56);
        pub2.publish(18.58);
        pub3.publish(998);
        pub4.publish(true);
    }
    catch (...)
    {
        EXPECT_TRUE(false) << "Got error in publish";
    }
    vFed.enterExecutingMode();
    EXPECT_EQ(vFed.getCurrentMode(),helics::Federate::Modes::EXECUTING);

    auto time = vFed.requestTime(2.0);
    EXPECT_LT(time, 2.0);

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    auto val3 = sub3.getValue<int>();
    auto val4 = sub4.getValue<bool>();

    // make sure it is not default
    EXPECT_DOUBLE_EQ(val, 13.56);
    EXPECT_DOUBLE_EQ(val2, 18.58);
    EXPECT_EQ(val3, 998);
    EXPECT_TRUE(val4);

    vFed.finalize();
    result.get();
    runner.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

INSTANTIATE_TEST_SUITE_P(feedthrough, ConnectionFileTests, ::testing::ValuesIn(connectionFiles));

TEST(feedthrough, connnectionInFmuFile)
{
    static const std::string testFile = std::string(TEST_DIR) + "ft_connections.json";
    // helics::cleanupHelicsLibrary();
    FmiRunner runner;
    runner.parse(fmt::format("--fmupath={} {}", FMI_REFERENCE_DIR, testFile));

    int ret = runner.load();
    ASSERT_EQ(ret, 0);
    ret = runner.initialize();
    ASSERT_EQ(ret, 0);

    auto result = runner.runAsync();

    helics::FederateInfo fedInfo(helics::CoreType::ZMQ);
    fedInfo.broker = "ft5broker";

    helics::ValueFederate vFed("fed1", fedInfo);

    vFed.enterInitializingModeIterative();

    auto qres = helics::vectorizeQueryResult(vFed.query("root", "publications"));

    EXPECT_EQ(qres.size(), 5U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);
    auto& sub3 = vFed.registerSubscription(qres[2]);
    sub2.setDefault(-20.0);
    auto& sub4 = vFed.registerSubscription(qres[3]);
    sub2.setDefault(-20.0);

    auto& pub1 = vFed.registerGlobalPublication<double>("pub0");
    auto& pub2 = vFed.registerGlobalPublication<double>("pub1");
    auto& pub3 = vFed.registerGlobalPublication<int>("pub2");
    auto& pub4 = vFed.registerGlobalPublication<bool>("pub3");

    vFed.enterInitializingMode();
    EXPECT_EQ(vFed.getCurrentMode(),helics::Federate::Modes::INITIALIZING);
    try
    {
        pub1.publish(13.56);
        pub2.publish(18.58);
        pub3.publish(998);
        pub4.publish(true);
    }
    catch (...)
    {
        EXPECT_TRUE(false) << "Got error in publish";
    }
    vFed.enterExecutingMode();
    EXPECT_EQ(vFed.getCurrentMode(),helics::Federate::Modes::EXECUTING);
    auto time = vFed.requestTime(2.0);
    EXPECT_LT(time, 2.0);

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    auto val3 = sub3.getValue<int>();
    auto val4 = sub4.getValue<bool>();

    // make sure it is not default
    EXPECT_DOUBLE_EQ(val, 13.56);
    EXPECT_DOUBLE_EQ(val2, 18.58);
    EXPECT_EQ(val3, 998);
    EXPECT_TRUE(val4);

    vFed.finalize();
    result.get();
    runner.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}
