##############################################################################
#Copyright (C) 2017-2023, Battelle Memorial Institute
#All rights reserved.

# This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle
# Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for
# Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence
# Livermore National Security, LLC.
# ##################################################################################################

# -----------------------------------------------------------------------------
# helics fmi tests
# -----------------------------------------------------------------------------

set(helics_fmi_test_sources
    helicsFmiTests.cpp BouncingBallHelicsTests.cpp FeedthroughHelicsTests.cpp
    ResourceHelicsTests.cpp helicsFmiFailureTests.cpp
)
if(WIN32)
    set(platform_fmi_tests WindowsTest1.cpp)
else()

endif()

add_executable(helics_fmi-tests ${helics_fmi_test_sources} ${platform_fmi_tests})

set_target_properties(helics_fmi-tests PROPERTIES FOLDER tests)

target_link_libraries(
    helics_fmi-tests PRIVATE ${HELICS_FMI_HELICS_TARGET_APPS} helics_fmi_test_base
)

target_link_libraries(helics_fmi-tests PUBLIC helicsFMI)

add_test(NAME helics_fmi-tests COMMAND helics_fmi-tests --log_level=test_suite --report_level=short)
