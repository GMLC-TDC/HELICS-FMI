##############################################################################
#Copyright (C) 2017-2023, Battelle Memorial Institute
#All rights reserved.

# This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle
# Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for
# Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence
# Livermore National Security, LLC.
# ##################################################################################################


# -----------------------------------------------------------------------------
# fmi tests
# -----------------------------------------------------------------------------

set(fmi_test_sources fmiTests.cpp BouncingBallFMITests.cpp ResourceFMITests.cpp fmiFailureTests.cpp)

if(WIN32)
    set(platform_fmi_tests WindowsTest1.cpp)
else()

endif()

add_executable(fmi-tests ${fmi_test_sources})

set_target_properties(fmi-tests PROPERTIES FOLDER tests)

target_link_libraries(
   fmi-tests PRIVATE helics_fmi_test_base
)

target_link_libraries(fmi-tests PUBLIC fmiLibrary)

add_test(NAME fmi-tests COMMAND fmi-tests --log_level=test_suite --report_level=short)


target_compile_definitions(
    fmi-tests PRIVATE -DHELICS_BIN_LOC=\"${CMAKE_BINARY_DIR}/src/helics-fmi/\"
)
