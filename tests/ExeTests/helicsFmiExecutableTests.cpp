/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "exeTestHelper.h"
#include "helics-fmi/helics-fmi-config.h"
#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/application_api/queryFunctions.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <future>
#include <thread>

using ::testing::HasSubstr;

static const std::string bballFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
static const std::string ftFile = std::string(FMI_REFERENCE_DIR) + "Feedthrough.fmu";
TEST(exeTests, version)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    EXPECT_TRUE(hfmi.isActive());

    auto out = hfmi.runCaptureOutput("--version");
    EXPECT_THAT(out, HasSubstr(HELICS_FMI_VERSION_STRING));
}

TEST(exeTests, singleFed)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    /**test that things run to completion with auto broker*/
    auto out = hfmi.run(std::string("--autobroker ") + bballFile);
    EXPECT_EQ(out, 0);
}

TEST(exeTests, singleFedAsync)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runAsync(std::string("--autobroker ") + bballFile);
    EXPECT_EQ(out.get(), 0);
}

TEST(exeTests, singleFedAsyncZMQ)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runAsync(std::string("--autobroker --core=zmq ") + ftFile);
    EXPECT_EQ(out.get(), 0);
}

TEST(exeTests, dualFedAsyncZMQ)
{
    helics::cleanupHelicsLibrary();
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runCaptureOutputAsync(
        std::string(
            "--autobroker --coretype=zmq --step=0.1 --stoptime=2.0 --name=ftfed --brokerargs=\"-f2 --force --port=23808 --loglevel=trace\" ") +
        ftFile);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    helics::ValueFederate vFed("fed1", "--coretype=zmq --forcenewcore --brokerport=23808");

    vFed.enterInitializingModeIterative();

    auto qres = helics::vectorizeQueryResult(vFed.query("root", "federates"));
    EXPECT_EQ(qres.size(), 2U);

    qres = helics::vectorizeQueryResult(vFed.query("ftfed", "publications"));

    EXPECT_EQ(qres.size(), 5U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);

    qres = helics::vectorizeQueryResult(vFed.query("ftfed", "inputs"));

    EXPECT_EQ(qres.size(), 5U);

    vFed.enterExecutingMode();
    auto time1 = vFed.requestTime(2.0);
    EXPECT_LE(time1, 2.0);

    auto val = sub1.getValue<double>();
    auto val2 = sub2.getValue<double>();
    EXPECT_NE(val, -20.0);
    EXPECT_NE(val2, -20.0);
    vFed.finalize();
    auto str = out.get();
}

TEST(exeTests, singleFedLoggingInterfaces)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runCaptureOutput(std::string("--autobroker --loglevel=interfaces --name=bb1 ") +
                                     bballFile);
    EXPECT_THAT(out, HasSubstr("bb1.h"));
    EXPECT_THAT(out, HasSubstr("2 publications"));
}


TEST(exeTests, dualFedLoggingData)
{
    helics::cleanupHelicsLibrary();
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runCaptureOutputAsync(
        std::string(
            "--autobroker --coretype=zmq --step=0.1 --loglevel=data --stoptime=2.0 --name=ftfedlog --brokerargs=\"-f2 \" ") +
        ftFile);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    helics::ValueFederate vFed("fed1", "--coretype=zmq --forcenewcore ");

    vFed.enterInitializingModeIterative();

   

    auto qres = helics::vectorizeQueryResult(vFed.query("ftfedlog", "publications"));

    EXPECT_EQ(qres.size(), 5U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(-20.0);
    auto& sub2 = vFed.registerSubscription(qres[1]);
    sub2.setDefault(-20.0);

    qres = helics::vectorizeQueryResult(vFed.query("ftfedlog", "inputs"));
    auto& pub1 = vFed.registerPublication<double>("");

    pub1.addInputTarget(qres[0]);

    vFed.enterExecutingMode();
    auto time1 = vFed.requestTime(2.0);
    EXPECT_LE(time1, 2.0);
    pub1.publish(87.63);

    vFed.finalize();
    auto str = out.get();

    EXPECT_THAT(str, HasSubstr("received 87.63"));
    EXPECT_THAT(str, HasSubstr("publishing 87.63"));
}
