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

TEST(bouncingBall, simpleRun)
{
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    helics::FederateInfo fi(helics::CoreType::INPROC);
    fi.coreInitString = "--autobroker";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<FmiCoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<FmiCoSimFederate>("bball", inputFile, fi));
    csFed->setOutputCapture(true, "testOut.csv");
    csFed->configure(0.1, 0.0);
    csFed->run(2.0);

    EXPECT_TRUE(std::filesystem::exists("testOut.csv"));

    std::filesystem::remove("testOut.csv");
}

TEST(bouncingBall, checkPubsOutput)
{
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    helics::FederateInfo fi(helics::CoreType::INPROC);
    fi.coreInitString = "--autobroker";
    fi.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<FmiCoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<FmiCoSimFederate>("bball", inputFile, fi));

    fi.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fi);
    csFed->configure(0.1, 0.0);

    auto rs = std::async(std::launch::async, [csFed]() { csFed->run(2.0); });

    bool init = helics::waitForInit(&vFed, "bball", std::chrono::milliseconds(500));
    if (!init) {
        init = helics::waitForInit(&vFed, "bball", std::chrono::milliseconds(500));
    }
    EXPECT_TRUE(init);

    auto qres = helics::vectorizeQueryResult(vFed.query("root", "publications"));

    EXPECT_EQ(qres.size(), 2U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);
    vFed.enterExecutingMode();
    auto t1 = vFed.requestTime(2.0);
    EXPECT_LT(t1, 2.0);

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    EXPECT_NE(val, -20.0);
    EXPECT_NE(val2, -20.0);
    vFed.finalize();
}

TEST(bouncingBall, checkPubsExtra)
{
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    helics::FederateInfo fi(helics::CoreType::INPROC);
    fi.coreInitString = "--autobroker";
    fi.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<FmiCoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<FmiCoSimFederate>("bball", inputFile, fi));

    fi.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fi);
    csFed->addOutput("der(h)");
    csFed->configure(0.1, 0.0);

    auto rs = std::async(std::launch::async, [csFed]() { csFed->run(2.0); });

    bool init = helics::waitForInit(&vFed, "bball", std::chrono::milliseconds(500));
    if (!init) {
        init = helics::waitForInit(&vFed, "bball", std::chrono::milliseconds(500));
    }
    EXPECT_TRUE(init);

    auto qres = helics::vectorizeQueryResult(vFed.query("root", "publications"));

    EXPECT_EQ(qres.size(), 3U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);
    auto& sub3 = vFed.registerSubscription(qres[2]);
    sub2.setDefault(-20.0);
    vFed.enterExecutingMode();
    auto t1 = vFed.requestTime(2.0);
    EXPECT_LT(t1, 2.0);

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
}

TEST(bouncingBall, setHeight)
{
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    helics::FederateInfo fi(helics::CoreType::INPROC);
    fi.coreInitString = "--autobroker";
    fi.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<FmiCoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<FmiCoSimFederate>("bball", inputFile, fi));

    fi.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fi);
    csFed->set("h", 4.0);
    csFed->configure(0.1, 0.0);

    auto rs = std::async(std::launch::async, [csFed]() { csFed->run(1.0); });

    auto& sub1 = vFed.registerSubscription("bball/h");
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription("bball/v");
    sub2.setDefault(-20.0);
    vFed.enterExecutingMode();

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 4.0);
    EXPECT_DOUBLE_EQ(val2, 0.0);

    auto t1 = vFed.requestTime(2.0);
    EXPECT_EQ(t1, 0.1);

    val = sub1.getValue<double>();
    val2 = sub2.getValue<double>();
    EXPECT_LT(val, 4.0);
    EXPECT_GT(val, 3.0);
    EXPECT_LT(val2, 0.0);
    vFed.finalize();
}

TEST(bouncingBall, setHeightCommand)
{
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    helics::FederateInfo fi(helics::CoreType::INPROC);
    fi.coreInitString = "--autobroker";
    fi.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<FmiCoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<FmiCoSimFederate>("bball", inputFile, fi));

    fi.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fi);
    csFed->runCommand("set h 3.0");
    csFed->configure(0.1, 0.0);

    auto rs = std::async(std::launch::async, [csFed]() { csFed->run(1.0); });

    auto& sub1 = vFed.registerSubscription("bball/h");
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription("bball/v");
    sub2.setDefault(-20.0);
    vFed.enterExecutingMode();

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 3.0);
    EXPECT_DOUBLE_EQ(val2, 0.0);

    auto t1 = vFed.requestTime(2.0);
    EXPECT_EQ(t1, 0.1);

    val = sub1.getValue<double>();
    val2 = sub2.getValue<double>();
    EXPECT_LT(val, 3.0);
    EXPECT_GT(val, 2.0);
    EXPECT_LT(val2, 0.0);
    vFed.finalize();
}

TEST(bouncingBall, setHeightRemoteCommand)
{
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    helics::FederateInfo fi(helics::CoreType::INPROC);
    fi.coreInitString = "--autobroker";
    fi.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<FmiCoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<FmiCoSimFederate>("bball", inputFile, fi));

    fi.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fi);
    vFed.sendCommand("bball", "set h 3.0");
    csFed->configure(0.1, 0.0);

    auto rs = std::async(std::launch::async, [csFed]() { csFed->run(1.0); });

    auto& sub1 = vFed.registerSubscription("bball/h");
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription("bball/v");
    sub2.setDefault(-20.0);
    vFed.enterExecutingMode();

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 3.0);
    EXPECT_DOUBLE_EQ(val2, 0.0);

    auto t1 = vFed.requestTime(2.0);
    EXPECT_EQ(t1, 0.1);

    val = sub1.getValue<double>();
    val2 = sub2.getValue<double>();
    EXPECT_LT(val, 3.0);
    EXPECT_GT(val, 2.0);
    EXPECT_LT(val2, 0.0);
    vFed.finalize();
}
