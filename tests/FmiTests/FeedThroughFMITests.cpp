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

static const std::string fmuName = "Feedthrough";
static const std::string inputFile = std::string(FMI_REFERENCE_DIR) + fmuName + ".fmu";

TEST(feedthrough, ExtractFMU)
{
    FmiLibrary fmi;

    EXPECT_TRUE(std::filesystem::exists(inputFile));
    EXPECT_NO_THROW(fmi.loadFMU(inputFile));

    EXPECT_TRUE(fmi.isXmlLoaded());
    EXPECT_FALSE(fmi.isSoLoaded());

    auto dir = std::string(FMI_REFERENCE_DIR) + fmuName;
    EXPECT_TRUE(std::filesystem::exists(dir));

    fmi.close();
    std::filesystem::remove_all(dir);
}

TEST(feedthrough, loadXML)
{
    auto fmi = std::make_shared<FmiLibrary>();
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto info = fmi->getInfo();

    EXPECT_TRUE(info->checkFlag(modelExchangeCapable));
    EXPECT_TRUE(info->checkFlag(coSimulationCapable));

    EXPECT_EQ(info->getString("modelName"), fmuName);
    EXPECT_EQ(info->getCounts(fmiVariableType::input), 5);
    EXPECT_EQ(info->getCounts(fmiVariableType::output), 5);
    EXPECT_EQ(info->getCounts(fmiVariableType::parameter), 3);
    EXPECT_EQ(info->getCounts(fmiVariableType::any), 14);
    EXPECT_EQ(info->getCounts(fmiVariableType::state), 0);
    EXPECT_EQ(info->getCounts(fmiVariableType::local), 0);
    EXPECT_EQ(info->getCounts(fmiVariableType::units), 0);

    EXPECT_EQ(info->getCounts(fmiVariableType::meObject), 1);
    EXPECT_EQ(info->getCounts(fmiVariableType::csObject), 1);

    EXPECT_DOUBLE_EQ(info->getReal("version"), 2.0);
    fmi->deleteFMUdirectory();

    fmi.reset();
}

bool variableCheck(const std::shared_ptr<FmiInfo>& info,
                   const std::string& variableName,
                   fmi_causality causality,
                   fmi_variability variability,
                   fmi_variable_type type)
{
    const auto& vinfo = info->getVariableInfo(variableName);
    bool ret = true;
    EXPECT_EQ(vinfo.causality, causality)
        << std::to_string(static_cast<int>(ret = false)) << " " << causality._to_string()
        << " does not match variable causality which is " << vinfo.causality._to_string();
    EXPECT_EQ(vinfo.variability, variability)
        << std::to_string(static_cast<int>(ret = false)) << " " << variability._to_string()
        << " does not match variable variability " << vinfo.variability._to_string();
    EXPECT_EQ(vinfo.type, type) << std::to_string(static_cast<int>(ret = false)) << " "
                                << type._to_string() << " does not match variable type "
                                << vinfo.type._to_string();
    return ret;
}

TEST(feedthrough, variableTypes)
{
    auto fmi = std::make_shared<FmiLibrary>();
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto info = fmi->getInfo();
    EXPECT_TRUE(variableCheck(info,
                              "time",
                              fmi_causality::independent,
                              fmi_variability::continuous,
                              fmi_variable_type::real));
    {
        const auto& vinfo = info->getVariableInfo("time");
        EXPECT_EQ(vinfo.derivative, false);
        EXPECT_EQ(vinfo.isAlias, false);
    }
    EXPECT_TRUE(variableCheck(info,
                              "Float64_fixed_parameter",
                              fmi_causality::parameter,
                              fmi_variability::fixed,
                              fmi_variable_type::real));
    EXPECT_TRUE(variableCheck(info,
                              "Float64_tunable_parameter",
                              fmi_causality::parameter,
                              fmi_variability::tunable,
                              fmi_variable_type::real));
    EXPECT_TRUE(variableCheck(info,
                              "Float64_continuous_input",
                              fmi_causality::input,
                              fmi_variability::continuous,
                              fmi_variable_type::real));
    EXPECT_TRUE(variableCheck(info,
                              "Float64_continuous_output",
                              fmi_causality::output,
                              fmi_variability::continuous,
                              fmi_variable_type::real));

    EXPECT_TRUE(variableCheck(info,
                              "Float64_discrete_input",
                              fmi_causality::input,
                              fmi_variability::discrete,
                              fmi_variable_type::real));
    EXPECT_TRUE(variableCheck(info,
                              "Float64_discrete_output",
                              fmi_causality::output,
                              fmi_variability::discrete,
                              fmi_variable_type::real));

    EXPECT_TRUE(variableCheck(info,
                              "Int32_input",
                              fmi_causality::input,
                              fmi_variability::discrete,
                              fmi_variable_type::integer));
    EXPECT_TRUE(variableCheck(info,
                              "Int32_output",
                              fmi_causality::output,
                              fmi_variability::discrete,
                              fmi_variable_type::integer));

    EXPECT_TRUE(variableCheck(info,
                              "Boolean_input",
                              fmi_causality::input,
                              fmi_variability::discrete,
                              fmi_variable_type::boolean));
    EXPECT_TRUE(variableCheck(info,
                              "Boolean_output",
                              fmi_causality::output,
                              fmi_variability::discrete,
                              fmi_variable_type::boolean));

    EXPECT_TRUE(variableCheck(info,
                              "String_parameter",
                              fmi_causality::parameter,
                              fmi_variability::fixed,
                              fmi_variable_type::string));

    EXPECT_TRUE(variableCheck(info,
                              "Enumeration_input",
                              fmi_causality::input,
                              fmi_variability::discrete,
                              fmi_variable_type::enumeration));
    EXPECT_TRUE(variableCheck(info,
                              "Enumeration_output",
                              fmi_causality::output,
                              fmi_variability::discrete,
                              fmi_variable_type::enumeration));

    fmi->deleteFMUdirectory();

    fmi.reset();
}

TEST(feedthrough, loadSharedME)
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

TEST(feedthrough, loadSharedCS)
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

    auto dir = std::string(FMI_REFERENCE_DIR) + fmuName;
    // this is true since we didn'time open the directory in this test
    EXPECT_TRUE(std::filesystem::exists(dir));

    std::filesystem::remove_all(dir);
}

TEST(feedthrough, runModeSequence)
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

TEST(feedthrough, csExecution)
{
    auto fmi = std::make_shared<FmiLibrary>();
    EXPECT_NO_THROW(fmi->loadFMU(inputFile));

    auto fmiObj = fmi->createCoSimulationObject("model_cs");
    ASSERT_TRUE(fmiObj);
    EXPECT_EQ(fmiObj->getName(), "model_cs");

    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::INSTANTIATED);
    fmiObj->setupExperiment(fmi2False, 0.0, 0.0, fmi2True, 11.0);

    fmiObj->setMode(FmuMode::INITIALIZATION);
    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::INITIALIZATION);

    fmiObj->setMode(FmuMode::CONTINUOUS_TIME);  // this mode is not valid for cosim object so
                                                // going to stepMode
    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::STEP);

    double time = 0;
    while (time < 10.0) {
        EXPECT_NO_THROW(fmiObj->doStep(time, 1.0, fmi2True));
        time = time + 1.0;
    }
    fmiObj->setMode(FmuMode::TERMINATED);
    EXPECT_EQ(fmiObj->getCurrentMode(), FmuMode::TERMINATED);
    fmiObj.reset();

    fmi->deleteFMUdirectory();

    fmi.reset();
}
