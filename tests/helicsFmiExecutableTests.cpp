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
    const std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";

    /**test that things run to completion with auto broker*/
    auto out = hfmi.run(std::string("--autobroker ") + inputFile);
    EXPECT_EQ(out, 0);
}

TEST(exeTests, singleFedAsync)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");
    const std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runAsync(std::string("--autobroker ") + inputFile);
    EXPECT_EQ(out.get(), 0);
}

TEST(exeTests, singleFedAsyncZMQ)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");
    const std::string inputFile = std::string(FMI_REFERENCE_DIR) + "Feedthrough.fmu";

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runAsync(std::string("--autobroker --core=zmq ") + inputFile);
    EXPECT_EQ(out.get(), 0);
}

TEST(exeTests, dualFedAsyncZMQ)
{
    exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "Feedthrough.fmu";

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runCaptureOutputAsync(
        std::string(
            "--autobroker --coretype=zmq --step=0.1 --stop=2.0 --name=ftfed --brokerargs=\"-f2 --force\" ") +
        inputFile);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    helics::ValueFederate vFed("fed1", "--coretype=zmq");

    bool init = helics::waitForInit(&vFed, "ftfed", std::chrono::milliseconds(10000));
    if (!init) {
        init = helics::waitForInit(&vFed, "ftfed", std::chrono::milliseconds(10000));
    }
    if (!init) {
        vFed.globalError(-4, "init failed");
        auto result = out.get();
        std::cout << result << std::endl;
    }
    ASSERT_TRUE(init);

    auto qres = helics::vectorizeQueryResult(vFed.query("ftfed", "publications"));

    EXPECT_EQ(qres.size(), 4U);

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
    auto str = out.get();
}