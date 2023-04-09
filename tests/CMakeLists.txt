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

get_target_property(BDIR BouncingBall BINARY_DIR)
message(STATUS "binary dir = ${BDIR}")
target_compile_definitions(helics_fmi_test_base INTERFACE -DFMI_REFERENCE_DIR=\"${BDIR}/fmus/\")
target_compile_definitions(
    helics_fmi_test_base INTERFACE -DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test_files/\"
)

target_compile_definitions(
    helics_fmi_test_base INTERFACE -DHELICS_BIN_LOC=\"${CMAKE_BINARY_DIR}/src/helics-fmi/\"
)

add_subdirectory(ExeTests)
add_subdirectory(FmiTests)
add_subdirectory(HelicsFmiTests)
