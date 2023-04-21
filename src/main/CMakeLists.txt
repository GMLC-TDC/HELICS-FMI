# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2023, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(helics-fmi_sources helicsFmiRunner.cpp)

set(helics-fmi_headers helicsFmiRunner.hpp)

add_library(helicsFmiRunner STATIC ${helics-fmi_sources} ${helics-fmi_headers})

add_executable(helics-fmi helics-fmi-main.cpp)

target_link_libraries(helicsFmiRunner PUBLIC helicsFMI)
target_link_libraries(helicsFmiRunner PRIVATE fmt::fmt)
target_include_directories(helicsFmiRunner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(helics-fmi PUBLIC helicsFmiRunner)

install(TARGETS helics-fmi RUNTIME DESTINATION bin)

if(WIN32 AND HELICS_BINARIES)
    message(STATUS "copying helics Binaries : ${HELICS_BINARIES}")
    add_custom_command(
        TARGET helics-fmi POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${HELICS_BINARIES}
                                             "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
    )
endif()
