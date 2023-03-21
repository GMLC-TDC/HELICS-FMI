##############################################################################
#Copyright (C) 2017-2023, Battelle Memorial Institute
#All rights reserved.

# This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle
# Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for
# Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence
# Livermore National Security, LLC.
# ##################################################################################################

include(AddGoogletest)

include(addFMUreferences)

# add a baseline library for underlying dependencies and flags for test executables
add_library(helics_fmi_test_base INTERFACE)
target_link_libraries(helics_fmi_test_base INTERFACE gtest gtest_main gmock)
target_link_libraries(helics_fmi_test_base INTERFACE compile_flags_target)
target_include_directories(helics_fmi_test_base INTERFACE ${PROJECT_SOURCE_DIR}/ThirdParty)

# -----------------------------------------------------------------------------
# helics fmi tests
# -----------------------------------------------------------------------------

set(fmi_test_sources fmiTests.cpp BouncingBallFMITests.cpp fmiFailureTests.cpp)
set(helics_fmi_test_sources helicsFmiTests.cpp BouncingBallHelicsTests.cpp
                            FeedthroughHelicsTests.cpp helicsFmiFailureTests.cpp
)
if(WIN32)
    set(platform_fmi_tests WindowsTest1.cpp)
else()

endif()

add_executable(fmi-tests ${fmi_test_sources})
add_executable(helics_fmi-tests ${helics_fmi_test_sources} ${platform_fmi_tests})

set_target_properties(fmi-tests PROPERTIES FOLDER tests)
set_target_properties(helics_fmi-tests PROPERTIES FOLDER tests)

target_link_libraries(
    helics_fmi-tests PRIVATE ${HELICS_FMI_HELICS_TARGET_APPS} helics_fmi_test_base
)

target_link_libraries(fmi-tests PRIVATE helics_fmi_test_base)

target_link_libraries(fmi-tests PUBLIC fmiLibrary)

target_link_libraries(helics_fmi-tests PUBLIC helicsFMI)

add_test(NAME fmi-tests COMMAND fmi-tests --log_level=test_suite --report_level=short)

target_compile_definitions(fmi-tests PRIVATE -DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test_files/\")

add_test(NAME helics_fmi-tests COMMAND helics_fmi-tests --log_level=test_suite --report_level=short)

target_compile_definitions(
    helics_fmi-tests PRIVATE -DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test_files/\"
)

get_target_property(BDIR BouncingBall BINARY_DIR)
message(STATUS "binary dir = ${BDIR}")
target_compile_definitions(helics_fmi-tests PRIVATE -DFMI_REFERENCE_DIR=\"${BDIR}/fmus/\")

target_compile_definitions(
    helics_fmi-tests PRIVATE -DHELICS_BIN_LOC=\"${CMAKE_BINARY_DIR}/src/helics-fmi/\"
)

target_compile_definitions(fmi-tests PRIVATE -DFMI_REFERENCE_DIR=\"${BDIR}/fmus/\")

target_compile_definitions(
    fmi-tests PRIVATE -DHELICS_BIN_LOC=\"${CMAKE_BINARY_DIR}/src/helics-fmi/\"
)

# -----------------------------------------------------------------------------
# helics fmi executable tests
# -----------------------------------------------------------------------------

set(executable_test_sources exeTestHelper.cpp exeTestHelper.h helicsFmiExecutableTests.cpp)

add_executable(helics_fmi_executable-tests ${executable_test_sources})

target_compile_definitions(
    helics_fmi_executable-tests PRIVATE -DFMI_REFERENCE_DIR=\"${BDIR}/fmus/\"
)

target_compile_definitions(
    helics_fmi_executable-tests PRIVATE -DHELICS_EXE_LOC=\"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/\"
)

target_link_libraries(helics_fmi_executable-tests PUBLIC helicsFMI)

add_test(NAME helics_fmi_executable-tests COMMAND helics_fmi_executable-tests
                                                  --log_level=test_suite --report_level=short
)

target_compile_definitions(
    helics_fmi_executable-tests PRIVATE -DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test_files/\"
)

set_target_properties(helics_fmi_executable-tests PROPERTIES FOLDER tests)

target_link_libraries(
    helics_fmi_executable-tests PRIVATE ${HELICS_FMI_HELICS_TARGET_APPS} helics_fmi_test_base
)
