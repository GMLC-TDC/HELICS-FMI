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

static const std::string inputFile = std::string(FMI_REFERENCE_DIR) + "Resource.fmu";

using helicsfmi::CoSimFederate;

TEST(Resource, simpleRun)
{
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<CoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<CoSimFederate>("resource", inputFile, fedInfo));
    csFed->setOutputCapture(true, "testOut.csv");
    csFed->configure(0.1, 0.0);
    csFed->run(1.0);

    EXPECT_TRUE(std::filesystem::exists("testOut.csv"));

    std::filesystem::remove("testOut.csv");
}

TEST(Resource, checkPubsOutput)
{
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.brokerInitString = "-f2";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
    std::shared_ptr<CoSimFederate> csFed;
    EXPECT_NO_THROW(csFed = std::make_shared<CoSimFederate>("resource", inputFile, fedInfo));

    fedInfo.coreInitString.clear();

    helics::ValueFederate vFed("fed1", fedInfo);
    csFed->configure(0.1, 0.0);

    auto result = std::async(std::launch::async, [csFed]() { csFed->run(1.0); });

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
    EXPECT_EQ(val, static_cast<int>('a'));
    vFed.finalize();
    result.get();
}
