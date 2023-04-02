/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/queryFunctions.hpp"
#include "helicsFmiRunner.hpp"
#include "exeTestHelper.h"

#include "gtest/gtest.h"
#include <filesystem>
#include <future>


static const std::string inputFile = std::string(FMI_REFERENCE_DIR) + "Resource.fmu";

using helicsfmi::FmiRunner;


TEST(Resource, simpleRun)
{
    FmiRunner runner;
    runner.parse(std::string("--autobroker --core=zmq ") + inputFile);
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

TEST(Resource, checkPubsOutput)
{
    FmiRunner runner;
    runner.parse(
        std::string(
            "--autobroker --coretype=zmq --step=0.1 --stop=1.0 --name=reserouce  --brokerargs=\"-f2 --name=r1broker\" ") +
        inputFile);
    int ret = runner.load();
    ASSERT_EQ(ret, 0);
    ret = runner.initialize();
    ASSERT_EQ(ret, 0);

    auto result = runner.runAsync();

    helics::FederateInfo fedInfo(helics::CoreType::ZMQ);
    

    fedInfo.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fedInfo);

    vFed.enterInitializingModeIterative();

    auto qres = helics::vectorizeQueryResult(vFed.query("root", "publications"));

    EXPECT_EQ(qres.size(), 1U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(561);
    
    vFed.enterExecutingMode();
    auto time = vFed.requestTime(2.0);
    EXPECT_LT(time, 2.0);

    auto val = sub1.getValue<int>();

    EXPECT_NE(val, -20.0);
    EXPECT_EQ(val,static_cast<int>('a'));
    vFed.finalize();
    result.get();
}

TEST(Resource, simpleExeRun)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    /**test that things run to completion with auto broker*/
    auto out = hfmi.run(std::string("--autobroker ") + inputFile);
    EXPECT_EQ(out, 0);
}

TEST(Resource, checkExePubsOutput)
{
    const exeTestRunner hfmi(HELICS_EXE_LOC, "helics-fmi");

    /**test that things run to completion with auto broker*/
    auto out = hfmi.runCaptureOutputAsync(
        std::string(
            "--autobroker --coretype=zmq --step=0.1 --stop=2.0 --name=ftfed --brokerargs=\"-f2 \" ") +
        inputFile);

    helics::FederateInfo fedInfo(helics::CoreType::ZMQ);


    fedInfo.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fedInfo);

    vFed.enterInitializingModeIterative();

    auto qres = helics::vectorizeQueryResult(vFed.query("root", "publications"));

    EXPECT_EQ(qres.size(), 1U);

    auto& sub1 = vFed.registerSubscription(qres[0]);
    sub1.setDefault(561);

    vFed.enterExecutingMode();
    auto time = vFed.requestTime(2.0);
    EXPECT_LT(time, 2.0);

    auto val = sub1.getValue<int>();

    EXPECT_NE(val, -20.0);
    EXPECT_EQ(val,static_cast<int>('a'));
    vFed.finalize();
    out.get();
}
