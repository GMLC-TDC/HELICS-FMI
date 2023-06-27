##############################################################################
#Copyright (C) 2017-2023, Battelle Memorial Institute
#All rights reserved.

# This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle
# Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for
# Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence
# Livermore National Security, LLC.
# ##################################################################################################

# -----------------------------------------------------------------------------
# helics fmi executable tests
# -----------------------------------------------------------------------------

set(executable_test_sources exeTestHelper.cpp exeTestHelper.h helicsFmiExecutableTests.cpp
                            RunnerTests.cpp ResourceExecutableTests.cpp
)

set(executable_fail_test_sources exeTestHelper.cpp exeTestHelper.h exeFailureTests.cpp
                                 runnerFailureTests.cpp
)

add_executable(helics_fmi_executable-tests ${executable_test_sources})
add_executable(helics_fmi_fail_executable-tests ${executable_fail_test_sources})

target_compile_definitions(
    helics_fmi_executable-tests PRIVATE -DHELICS_EXE_LOC=\"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/\"
)

target_compile_definitions(
    helics_fmi_fail_executable-tests
    PRIVATE -DHELICS_EXE_LOC=\"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/\"
)

target_link_libraries(helics_fmi_executable-tests PUBLIC helicsFMI helicsFmiRunner)
target_link_libraries(helics_fmi_fail_executable-tests PUBLIC helicsFMI helicsFmiRunner)

add_test(NAME helics_fmi_executable-tests COMMAND helics_fmi_executable-tests
                                                  --log_level=test_suite --report_level=short
)

add_test(NAME helics_fmi_fail_executable-tests COMMAND helics_fmi_fail_executable-tests
                                                       --log_level=test_suite --report_level=short
)

set_target_properties(helics_fmi_executable-tests PROPERTIES FOLDER tests)

target_link_libraries(
    helics_fmi_executable-tests PRIVATE ${HELICS_FMI_HELICS_TARGET_APPS} helics_fmi_test_base fmt::fmt
)

set_target_properties(helics_fmi_fail_executable-tests PROPERTIES FOLDER tests)

target_link_libraries(
    helics_fmi_fail_executable-tests PRIVATE ${HELICS_FMI_HELICS_TARGET_APPS} helics_fmi_test_base
)
