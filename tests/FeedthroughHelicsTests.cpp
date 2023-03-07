/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "FmiCoSimFederate.hpp"
#include "helics/application_api/helicsTypes.hpp"
#include "helics/application_api/queryFunctions.hpp"

#include "gtest/gtest.h"
#include <filesystem>
#include <future>

TEST(feedthrough, simpleRun)
{
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "Feedthrough.fmu";
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<FmiCoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<FmiCoSimFederate>("fthrough", inputFile, fedInfo));
    csFed->setOutputCapture(true, "testOut.csv");
    csFed->configure(0.1, 0.0);
    csFed->run(2.0);

    EXPECT_TRUE(std::filesystem::exists("testOut.csv"));

    std::filesystem::remove("testOut.csv");
}

TEST(feedthrough, checkIO)
{
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "Feedthrough.fmu";
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<FmiCoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<FmiCoSimFederate>("fthrough", inputFile, fedInfo));

    fedInfo.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fedInfo);
    csFed->configure(0.1, 0.0);

    auto sync = std::async(std::launch::async, [csFed]() { csFed->run(2.0); });

    bool init = helics::waitForInit(&vFed, "fthrough", std::chrono::milliseconds(500));
    if (!init) {
        init = helics::waitForInit(&vFed, "fthrough", std::chrono::milliseconds(500));
    }
    EXPECT_TRUE(init);

    auto qres = helics::vectorizeQueryResult(vFed.query("fthrough", "publications"));

    EXPECT_EQ(qres.size(), 4U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);

    qres = helics::vectorizeQueryResult(vFed.query("fthrough", "inputs"));

    EXPECT_EQ(qres.size(), 4U);

    vFed.enterExecutingMode();
    auto t1 = vFed.requestTime(2.0);
    EXPECT_LT(t1, 2.0);

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    EXPECT_NE(val, -20.0);
    EXPECT_NE(val2, -20.0);
    vFed.finalize();
        sync.get();
}

TEST(feedthrough, checkFeedthrough)
{
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "Feedthrough.fmu";
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<FmiCoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<FmiCoSimFederate>("fthrough", inputFile, fedInfo));

    fedInfo.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fedInfo);
    csFed->configure(0.1, 0.0);

    auto sync = std::async(std::launch::async, [csFed]() { csFed->run(2.0); });

    bool init = helics::waitForInit(&vFed, "fthrough", std::chrono::milliseconds(500));
    if (!init) {
        init = helics::waitForInit(&vFed, "fthrough", std::chrono::milliseconds(500));
    }
    EXPECT_TRUE(init);

    auto qres = helics::vectorizeQueryResult(vFed.query("root", "publications"));

    EXPECT_EQ(qres.size(), 4U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);
    auto& sub3 = vFed.registerSubscription(qres[2]);
    sub2.setDefault(-20.0);
    auto& sub4 = vFed.registerSubscription(qres[3]);
    sub2.setDefault(-20.0);

    qres = helics::vectorizeQueryResult(vFed.query("fthrough", "inputs"));

    EXPECT_EQ(qres.size(), 4U);
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

    auto t1 = vFed.requestTime(2.0);
    EXPECT_LT(t1, 2.0);

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
    sync.get();
}

TEST(feedthrough, pubTypes)
{
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "Feedthrough.fmu";
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<FmiCoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<FmiCoSimFederate>("fthrough", inputFile, fedInfo));

    fedInfo.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fedInfo);
    csFed->configure(0.1, 0.0);

    auto sync = std::async(std::launch::async, [csFed]() { csFed->run(2.0); });

    bool init = helics::waitForInit(&vFed, "fthrough", std::chrono::milliseconds(500));
    if (!init) {
        init = helics::waitForInit(&vFed, "fthrough", std::chrono::milliseconds(500));
    }
    EXPECT_TRUE(init);

    auto qres = helics::vectorizeQueryResult(vFed.query("root", "publications"));

    EXPECT_EQ(qres.size(), 4U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);
    auto& sub3 = vFed.registerSubscription(qres[2]);
    sub2.setDefault(-20.0);
    auto& sub4 = vFed.registerSubscription(qres[3]);
    sub2.setDefault(-20.0);

    vFed.enterInitializingMode();

    EXPECT_EQ(sub1.getPublicationType(), helics::typeNameString<double>());
    EXPECT_EQ(sub2.getPublicationType(), helics::typeNameString<double>());
    EXPECT_EQ(sub3.getPublicationType(), helics::typeNameString<int64_t>());
    EXPECT_EQ(sub4.getPublicationType(), helics::typeNameString<bool>());

    vFed.enterExecutingMode();

    auto t1 = vFed.requestTime(2.0);
    EXPECT_EQ(t1, 0.1);

    vFed.finalize();
    sync.get();
}
