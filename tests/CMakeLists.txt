##############################################################################
#Copyright (C) 2017-2022, Battelle Memorial Institute
#All rights reserved.

# This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle
# Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for
# Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence
# Livermore National Security, LLC.
# ##################################################################################################

include(AddGoogletest)

# add a baseline library for underlying dependencies and flags for test executables
add_library(helics_fmi_test_base INTERFACE)
target_link_libraries(helics_fmi_test_base INTERFACE gtest gtest_main gmock)
target_link_libraries(helics_fmi_test_base INTERFACE compile_flags_target)
target_include_directories(helics_fmi_test_base INTERFACE ${PROJECT_SOURCE_DIR}/ThirdParty)

# -----------------------------------------------------------------------------
# helics fmi tests
# -----------------------------------------------------------------------------

set(helics_fmi_test_sources exeTestHelper.cpp exeTestHelper.h fmiLoadTests.cpp helics-fmiTests.cpp)

add_executable(helics_fmi-tests ${helics_fmi_test_sources})

set_target_properties(helics_fmi-tests PROPERTIES FOLDER tests)

target_link_libraries(helics_fmi-tests PRIVATE ${HELICS_FMI_HELICS_TARGET_APPS} helics_fmi_test_base)

target_link_libraries(helics_fmi-tests)

add_test(NAME helics_fmi-tests COMMAND helics_fmi-tests --log_level=test_suite --report_level=short)

target_compile_definitions(helics_fmi-tests PRIVATE -DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")

target_compile_definitions(
    helics_fmi-tests PRIVATE -DHELICS_BIN_LOC=\"${CMAKE_BINARY_DIR}/src/helics-fmi/\"
)
