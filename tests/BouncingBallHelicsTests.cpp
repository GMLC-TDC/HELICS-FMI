/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "FmiCoSimFederate.hpp"
#include "helics/application_api/queryFunctions.hpp"

#include "gtest/gtest.h"
#include <filesystem>
#include <future>

static const std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";

using helicsfmi::CoSimFederate;

TEST(bouncingBall, simpleRun)
{
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<CoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<CoSimFederate>("bball", inputFile, fedInfo));
    csFed->setOutputCapture(true, "testOut.csv");
    csFed->configure(0.1, 0.0);
    csFed->run(2.0);

    EXPECT_TRUE(std::filesystem::exists("testOut.csv"));

    std::filesystem::remove("testOut.csv");
}

TEST(bouncingBall, checkPubsOutput)
{
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<CoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<CoSimFederate>("bball", inputFile, fedInfo));

    fedInfo.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fedInfo);
    csFed->configure(0.1, 0.0);

    auto result = std::async(std::launch::async, [csFed]() { csFed->run(2.0); });

    vFed.enterInitializingModeIterative();

    auto qres = helics::vectorizeQueryResult(vFed.query("root", "publications"));

    EXPECT_EQ(qres.size(), 2U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);
    vFed.enterExecutingMode();
    auto time = vFed.requestTime(2.0);
    EXPECT_LT(time, 2.0);

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    EXPECT_NE(val, -20.0);
    EXPECT_NE(val2, -20.0);
    vFed.finalize();
    result.get();
}

TEST(bouncingBall, checkPubsExtra)
{
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<CoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<CoSimFederate>("bball", inputFile, fedInfo));

    fedInfo.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fedInfo);
    csFed->addOutput("der(h)");
    csFed->configure(0.1, 0.0);

    auto result = std::async(std::launch::async, [csFed]() { csFed->run(2.0); });

    vFed.enterInitializingModeIterative();

    auto qres = helics::vectorizeQueryResult(vFed.query("root", "publications"));

    EXPECT_EQ(qres.size(), 3U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);
    auto& sub3 = vFed.registerSubscription(qres[2]);
    sub2.setDefault(-20.0);
    vFed.enterExecutingMode();
    auto time = vFed.requestTime(2.0);
    EXPECT_LT(time, 2.0);

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    auto val3 = sub3.getValue<double>();
    // make sure it is not default
    EXPECT_NE(val, -20.0);
    EXPECT_NE(val2, -20.0);
    EXPECT_NE(val3, -20.0);
    // make sure it is not 0
    EXPECT_NE(val, 0.0);
    EXPECT_NE(val2, 0.0);
    EXPECT_NE(val3, 0.0);
    // the 2nd and third should be equal v and der(h)
    EXPECT_DOUBLE_EQ(val2, val3);
    vFed.finalize();
    result.get();
}

TEST(bouncingBall, setHeight)
{
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<CoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<CoSimFederate>("bball", inputFile, fedInfo));

    fedInfo.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fedInfo);
    csFed->set("h", 4.0);
    csFed->configure(0.1, 0.0);

    auto result = std::async(std::launch::async, [csFed]() { csFed->run(1.0); });

    auto& sub1 = vFed.registerSubscription("bball/h");
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription("bball/v");
    sub2.setDefault(-20.0);
    vFed.enterExecutingMode();

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 4.0);
    EXPECT_DOUBLE_EQ(val2, 0.0);

    auto time = vFed.requestTime(2.0);
    EXPECT_EQ(time, 0.1);

    val = sub1.getValue<double>();
    val2 = sub2.getValue<double>();
    EXPECT_LT(val, 4.0);
    EXPECT_GT(val, 3.0);
    EXPECT_LT(val2, 0.0);
    vFed.finalize();
    result.get();
}

TEST(bouncingBall, setHeightCommand)
{
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<CoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<CoSimFederate>("bball", inputFile, fedInfo));

    fedInfo.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fedInfo);
    csFed->runCommand("set h 3.0");
    csFed->configure(0.1, 0.0);

    auto result = std::async(std::launch::async, [csFed]() { csFed->run(1.0); });

    auto& sub1 = vFed.registerSubscription("bball/h");
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription("bball/v");
    sub2.setDefault(-20.0);
    vFed.enterExecutingMode();

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 3.0);
    EXPECT_DOUBLE_EQ(val2, 0.0);

    auto time = vFed.requestTime(2.0);
    EXPECT_EQ(time, 0.1);

    val = sub1.getValue<double>();
    val2 = sub2.getValue<double>();
    EXPECT_LT(val, 3.0);
    EXPECT_GT(val, 2.0);
    EXPECT_LT(val2, 0.0);
    vFed.finalize();
    result.get();
}

TEST(bouncingBall, setHeightRemoteCommand)
{
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<CoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<CoSimFederate>("bball", inputFile, fedInfo));

    fedInfo.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fedInfo);
    vFed.sendCommand("bball", "set h 3.0");
    vFed.query("root", "global_flush");
    csFed->configure(0.1, 0.0);

    auto result = std::async(std::launch::async, [csFed]() { csFed->run(1.0); });

    auto& sub1 = vFed.registerSubscription("bball/h");
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription("bball/v");
    sub2.setDefault(-20.0);
    vFed.enterExecutingMode();

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 3.0);
    EXPECT_DOUBLE_EQ(val2, 0.0);

    auto time = vFed.requestTime(2.0);
    EXPECT_EQ(time, 0.1);

    val = sub1.getValue<double>();
    val2 = sub2.getValue<double>();
    EXPECT_LT(val, 3.0);
    EXPECT_GT(val, 2.0);
    EXPECT_LT(val2, 0.0);
    vFed.finalize();
    result.get();
}
