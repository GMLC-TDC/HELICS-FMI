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

static const std::string inputFile = std::string(FMI_REFERENCE_DIR) + "VanDerPol.fmu";

TEST(vanderpol, ExtractFMU)
{
    FmiLibrary fmi;

    EXPECT_TRUE(std::filesystem::exists(inputFile));
    EXPECT_NO_THROW(fmi.loadFMU(inputFile));

    EXPECT_TRUE(fmi.isXmlLoaded());
    EXPECT_FALSE(fmi.isSoLoaded());

    auto dir = std::string(FMI_REFERENCE_DIR) + "VanDerPol";
    EXPECT_TRUE(std::filesystem::exists(dir));

    fmi.close();
    std::filesystem::remove_all(dir);
}

TEST(vanderpol, loadXML)
{
    auto fmi = std::make_shared<FmiLibrary>();
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto info = fmi->getInfo();

    EXPECT_TRUE(info->checkFlag(modelExchangeCapable));
    EXPECT_TRUE(info->checkFlag(coSimulationCapable));

    EXPECT_EQ(info->getString("modelName"), "Van der Pol oscillator");
    EXPECT_EQ(info->getCounts(fmiVariableType::derivative), 2);
    EXPECT_EQ(info->getCounts(fmiVariableType::local), 2);

    EXPECT_EQ(info->getCounts(fmiVariableType::meObject), 1);
    EXPECT_EQ(info->getCounts(fmiVariableType::csObject), 1);

    EXPECT_DOUBLE_EQ(info->getReal("version"), 2.0);

    const auto& vinfo = info->getVariableInfo("x0");

    EXPECT_EQ("the first state", vinfo.description);

    fmi->deleteFMUdirectory();

    fmi.reset();

    auto dir = std::string(FMI_REFERENCE_DIR) + "VanDerPol";
    EXPECT_FALSE(std::filesystem::exists(dir));
}

TEST(vanderpol, loadSharedME)
{
    auto fmi = std::make_shared<FmiLibrary>();
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto fmiObj = fmi->createModelExchangeObject("model1");
    ASSERT_TRUE(fmiObj);
    EXPECT_EQ(fmiObj->getName(), "model1");

    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::INSTANTIATED);
    auto str = fmiObj->getInputNames();

    fmiObj->setMode(FmuMode::TERMINATED);
    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::TERMINATED);
    fmiObj.reset();
    fmi.reset();
}

TEST(vanderpol, loadSharedCS)
{
    auto fmi = std::make_shared<FmiLibrary>();
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto fmiObj = fmi->createCoSimulationObject("model_cs");
    ASSERT_TRUE(fmiObj);
    EXPECT_EQ(fmiObj->getName(), "model_cs");

    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::INSTANTIATED);
    auto str = fmiObj->getInputNames();

    fmiObj->setMode(FmuMode::TERMINATED);
    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::TERMINATED);
    fmiObj.reset();

    fmi->deleteFMUdirectory();

    fmi.reset();

    auto dir = std::string(FMI_REFERENCE_DIR) + "BouncingBall";
    // this is true since we didn'time open the directory in this test
    EXPECT_TRUE(std::filesystem::exists(dir));

    std::filesystem::remove_all(dir);
}

TEST(vanderpol, runModeSequence)
{
    auto fmi = std::make_shared<FmiLibrary>();
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto fmiObj = fmi->createCoSimulationObject("model_cs");
    ASSERT_TRUE(fmiObj);
    EXPECT_EQ(fmiObj->getName(), "model_cs");

    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::INSTANTIATED);
    auto str = fmiObj->getInputNames();

    fmiObj->setMode(FmuMode::INITIALIZATION);
    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::INITIALIZATION);

    fmiObj->setMode(FmuMode::CONTINUOUS_TIME);  // this mode is not valid for cosim object so
                                                // going to stepMode
    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::STEP);

    fmiObj->setMode(FmuMode::TERMINATED);
    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::TERMINATED);
    fmiObj.reset();

    fmi->deleteFMUdirectory();

    fmi.reset();
}

TEST(vanderpol, csExecution)
{
    auto fmi = std::make_shared<FmiLibrary>();
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto fmiObj = fmi->createCoSimulationObject("model_cs");
    auto info = fmi->getInfo();
    const auto& exp = info->getExperiment();
    EXPECT_EQ(exp.startTime, 0.0);
    EXPECT_EQ(exp.stepSize, 1e-2);
    EXPECT_EQ(exp.stopTime, 20.0);

    ASSERT_TRUE(fmiObj);
    EXPECT_EQ(fmiObj->getName(), "model_cs");

    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::INSTANTIATED);

    fmiObj->setupExperiment();

    fmiObj->setMode(FmuMode::INITIALIZATION);
    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::INITIALIZATION);

    fmiObj->setMode(FmuMode::CONTINUOUS_TIME);  // this mode is not valid for cosim object so
                                                // going to stepMode
    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::STEP);

    double time = 0;
    while (time < 10.0) {
        EXPECT_NO_THROW(fmiObj->doStep(time, 0.01, fmi2True));
        time = time + 0.01;
    }
    fmiObj->setMode(FmuMode::TERMINATED);
    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::TERMINATED);
    fmiObj.reset();

    fmi->deleteFMUdirectory();

    fmi.reset();
}
