/*
 * LLNS Copyright Start
 * Copyright (c) 2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "fmi/fmi_import/fmiImport.h"

#include "gtest/gtest.h"
#include <filesystem>

TEST(loadtests, ExtractFMU)
{
    FmiLibrary fmi;
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    EXPECT_NO_THROW(fmi.loadFMU(inputFile));

    EXPECT_TRUE(fmi.isXmlLoaded());
    EXPECT_FALSE(fmi.isSoLoaded());

    auto dir = std::string(FMI_REFERENCE_DIR) + "BouncingBall";
    EXPECT_TRUE(std::filesystem::exists(dir));

    fmi.close();
    std::filesystem::remove_all(dir);
}

TEST(loadtests, loadXML)
{
    auto fmi = std::make_shared<FmiLibrary>();
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto info = fmi->getInfo();

    EXPECT_TRUE(info->checkFlag(modelExchangeCapable));
    EXPECT_TRUE(info->checkFlag(coSimulationCapable));

    EXPECT_EQ(info->getString("modelName"), "BouncingBall");
    EXPECT_EQ(info->getCounts(fmiVariableType::input), 0);
    EXPECT_EQ(info->getCounts(fmiVariableType::output), 2);
    EXPECT_EQ(info->getCounts(fmiVariableType::parameter), 2);
    EXPECT_EQ(info->getCounts(fmiVariableType::any), 8);
    EXPECT_EQ(info->getCounts(fmiVariableType::state), 2);
    EXPECT_EQ(info->getCounts(fmiVariableType::local), 3);
    EXPECT_EQ(info->getCounts(fmiVariableType::units), 3);

    EXPECT_EQ(info->getCounts(fmiVariableType::meObject), 1);
    EXPECT_EQ(info->getCounts(fmiVariableType::csObject), 1);

    EXPECT_DOUBLE_EQ(info->getReal("version"), 2.0);
    fmi->deleteFMUdirectory();

    fmi.reset();

    auto dir = std::string(FMI_REFERENCE_DIR) + "BouncingBall";
    EXPECT_FALSE(std::filesystem::exists(dir));
}
