/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "fmi/fmi_import/fmiImport.h"
#include "fmi/fmi_import/fmiObjects.h"

#include "gtest/gtest.h"
#include <filesystem>

TEST(loadtests, ExtractFMU)
{
    FmiLibrary fmi;
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    EXPECT_TRUE(std::filesystem::exists(inputFile));
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

TEST(loadtests, loadSO_ME)
{
    auto fmi = std::make_shared<FmiLibrary>();
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto fmiObj = fmi->createModelExchangeObject("model1");
    ASSERT_TRUE(fmiObj);
    EXPECT_EQ(fmiObj->getName(), "model1");

    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::instantiatedMode);
    auto str = fmiObj->getInputNames();

    fmiObj->setMode(fmuMode::terminated);
    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::terminated);
    fmiObj.reset();
    fmi.reset();
}

TEST(loadtests, loadSO_CS)
{
    auto fmi = std::make_shared<FmiLibrary>();
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto fmiObj = fmi->createCoSimulationObject("model_cs");
    ASSERT_TRUE(fmiObj);
    EXPECT_EQ(fmiObj->getName(), "model_cs");

    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::instantiatedMode);
    auto str = fmiObj->getInputNames();

    fmiObj->setMode(fmuMode::terminated);
    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::terminated);
    fmiObj.reset();

    fmi->deleteFMUdirectory();

    fmi.reset();

    auto dir = std::string(FMI_REFERENCE_DIR) + "BouncingBall";
    // this is true since we didn't open the directory in this test
    EXPECT_TRUE(std::filesystem::exists(dir));

    std::filesystem::remove_all(dir);
}

TEST(loadtests, run_mode_sequence)
{
    auto fmi = std::make_shared<FmiLibrary>();
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto fmiObj = fmi->createCoSimulationObject("model_cs");
    ASSERT_TRUE(fmiObj);
    EXPECT_EQ(fmiObj->getName(), "model_cs");

    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::instantiatedMode);
    auto str = fmiObj->getInputNames();

    fmiObj->setMode(fmuMode::initializationMode);
    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::initializationMode);

    fmiObj->setMode(fmuMode::continuousTimeMode);  // this mode is not valid for cosim object so
                                                   // going to stepMode
    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::stepMode);

    fmiObj->setMode(fmuMode::terminated);
    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::terminated);
    fmiObj.reset();

    fmi->deleteFMUdirectory();

    fmi.reset();
}

TEST(loadtests, cs_execution)
{
    auto fmi = std::make_shared<FmiLibrary>();
    std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto fmiObj = fmi->createCoSimulationObject("model_cs");
    ASSERT_TRUE(fmiObj);
    EXPECT_EQ(fmiObj->getName(), "model_cs");

    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::instantiatedMode);
    auto str = fmiObj->getInputNames();

    fmiObj->setMode(fmuMode::initializationMode);
    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::initializationMode);

    fmiObj->setMode(fmuMode::continuousTimeMode);  // this mode is not valid for cosim object so
                                                   // going to stepMode
    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::stepMode);

    double t = 0;
    while (t < 10.0) {
        EXPECT_NO_THROW(fmiObj->doStep(t, 1.0, true));
        t = t + 1.0;
    }
    fmiObj->setMode(fmuMode::terminated);
    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::terminated);
    fmiObj.reset();

    fmi->deleteFMUdirectory();

    fmi.reset();
}
