/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "FmiCoSimFederate.hpp"
#include "FmiHelics.hpp"
#include "fmi/fmi_import/fmiImport.h"

#include "gtest/gtest.h"
#include <filesystem>

static const std::string inputDir = std::string(FMI_REFERENCE_DIR) + "/";

TEST(invalidFMU, nonExistingFile)
{
    helics::FederateInfo fedInfo(helics::CoreType::INPROC);
    fedInfo.coreInitString = "--autobroker";
    std::shared_ptr<helicsfmi::CoSimFederate> csFed;
    EXPECT_THROW(csFed = std::make_shared<helicsfmi::CoSimFederate>("bball",
                                                                    inputDir + "nonExistent.fmu",
                                                                    fedInfo),
                 helicsfmi::Error);

    EXPECT_FALSE(csFed);

    std::filesystem::remove("testOut.csv");
}

TEST(invalidFMU, invalidZip)
{
    FmiLibrary fmi;
    bool res{true};
    EXPECT_NO_THROW(res = fmi.loadFMU(inputDir + "dummy.fmu"));
    EXPECT_FALSE(res);
    EXPECT_FALSE(fmi.isXmlLoaded());
    EXPECT_FALSE(fmi.isSoLoaded());

    fmi.close();
}

TEST(invalidFMU, invalidFMU)
{
    FmiLibrary fmi;
    bool res{true};
    EXPECT_NO_THROW(res = fmi.loadFMU(inputDir + "validZip.fmu"));
    EXPECT_FALSE(res);
    EXPECT_FALSE(fmi.isXmlLoaded());
    EXPECT_FALSE(fmi.isSoLoaded());

    fmi.close();
}

TEST(invalidFMU, missingSO)
{
    FmiLibrary fmi;
    bool res{true};
    EXPECT_NO_THROW(res = fmi.loadFMU(inputDir + "missingSO.fmu"));
    EXPECT_FALSE(res);
    EXPECT_FALSE(fmi.isXmlLoaded());
    EXPECT_FALSE(fmi.isSoLoaded());

    fmi.close();
}
