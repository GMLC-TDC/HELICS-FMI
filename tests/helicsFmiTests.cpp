/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "FmiCoSimFederate.hpp"

#include "gtest/gtest.h"
#include <filesystem>

TEST(loadtests, simpleRun)
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
