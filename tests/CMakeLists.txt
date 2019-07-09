##############################################################################
#Copyright (C) 2017-2018, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################

#-----------------------------------------------------------------------------
# helics fmi tests using Boost
#-----------------------------------------------------------------------------


set(helics_fmi_test_sources
exeTestHelper.cpp
exeTestHelper.h
fmiLoadTests.cpp
helics-fmiTests.cpp
)


add_executable(helics_fmi-tests ${helics_fmi_test_sources})

set_target_properties (helics_fmi-tests PROPERTIES FOLDER tests)

target_link_libraries(helics_fmi-tests HELICS::helics-static)

target_link_libraries(helics_fmi-tests Boostlibs::core Boostlibs::test)

include_directories(${PROJECT_SOURCE_DIR}/ThirdParty)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_test(NAME helics_fmi-tests COMMAND helics_fmi-tests --log_level=test_suite --report_level=short)

target_compile_definitions(helics_fmi-tests PRIVATE -DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")

target_compile_definitions(helics_fmi-tests PRIVATE -DHELICS_BIN_LOC=\"${CMAKE_BINARY_DIR}/src/helics-fmi/\")

IF(WIN32)
foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET helics_fmi-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:helics_fmi-tests>/")                 # <--this is out-file path
endforeach(keyfile)
ENDIF(WIN32)

