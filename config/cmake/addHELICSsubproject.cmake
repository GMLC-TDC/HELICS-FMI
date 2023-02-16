# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2023, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#
# This file is used to add HELICS to a project
#

string(TOLOWER "helics" lcName)

if(NOT HELICS_CURRENT_VERSION)
    set(HELICS_CURRENT_VERSION 3.4.0)
endif()

include(FetchContent)

fetchcontent_declare(
    helics GIT_REPOSITORY https://github.com/GMLC-TDC/HELICS.git GIT_TAG v${HELICS_CURRENT_VERSION}
)

fetchcontent_getproperties(helics)

if(NOT ${lcName}_POPULATED)
    # Fetch the content using previously declared details
    fetchcontent_populate(helics)

endif()

# Set custom variables, policies, etc. ...

set(HELICS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(HELICS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(HELICS_BUILD_APP_EXECUTABLES OFF CACHE BOOL "" FORCE)
set(HELICS_DISABLE_C_SHARED_LIB ON CACHE BOOL "" FORCE)
set(HELICS_HIDE_CMAKE_VARIABLES ON CACHE BOOL "" FORCE)

add_subdirectory(${${lcName}_SOURCE_DIR} ${${lcName}_BINARY_DIR})

set(${PROJECT_NAME}_HELICS_TARGET_APPS HELICS::apps)
set(${PROJECT_NAME}_HELICS_TARGET HELICS::helicscpp)
# set_target_properties(clang-format clang-format-check clang-format-diff PROPERTIES FOLDER
# "Extern/zmq_clang_format")
