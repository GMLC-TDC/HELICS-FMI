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

static const std::string inputFile = std::string(FMI_REFERENCE_DIR) + "BouncingBall.fmu";

TEST(bouncingBall, ExtractFMU)
{
    FmiLibrary fmi;

    EXPECT_TRUE(std::filesystem::exists(inputFile));
    EXPECT_NO_THROW(fmi.loadFMU(inputFile));

    EXPECT_TRUE(fmi.isXmlLoaded());
    EXPECT_FALSE(fmi.isSoLoaded());

    auto dir = std::string(FMI_REFERENCE_DIR) + "BouncingBall";
    EXPECT_TRUE(std::filesystem::exists(dir));

    fmi.close();
    std::filesystem::remove_all(dir);
}

TEST(bouncingBall, loadXML)
{
    auto fmi = std::make_shared<FmiLibrary>();
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
    EXPECT_EQ(info->getCounts(fmiVariableType::local), 2);
    EXPECT_EQ(info->getCounts(fmiVariableType::units), 3);

    EXPECT_EQ(info->getCounts(fmiVariableType::meObject), 1);
    EXPECT_EQ(info->getCounts(fmiVariableType::csObject), 1);

    EXPECT_DOUBLE_EQ(info->getReal("version"), 2.0);
    fmi->deleteFMUdirectory();

    fmi.reset();

    auto dir = std::string(FMI_REFERENCE_DIR) + "BouncingBall";
    EXPECT_FALSE(std::filesystem::exists(dir));
}

TEST(bouncingBall, loadSharedME)
{
    auto fmi = std::make_shared<FmiLibrary>();
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

TEST(bouncingBall, loadSharedCS)
{
    auto fmi = std::make_shared<FmiLibrary>();
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
    // this is true since we didn'time open the directory in this test
    EXPECT_TRUE(std::filesystem::exists(dir));

    std::filesystem::remove_all(dir);
}

TEST(bouncingBall, runModeSequence)
{
    auto fmi = std::make_shared<FmiLibrary>();
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

TEST(bouncingBall, csExecution)
{
    auto fmi = std::make_shared<FmiLibrary>();
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

    double time = 0;
    while (time < 10.0) {
        EXPECT_NO_THROW(fmiObj->doStep(time, 1.0, true));
        time = time + 1.0;
    }
    fmiObj->setMode(fmuMode::terminated);
    EXPECT_EQ(fmiObj->getCurrentMode(), fmuMode::terminated);
    fmiObj.reset();

    fmi->deleteFMUdirectory();

    fmi.reset();
}
