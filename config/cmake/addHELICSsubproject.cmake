# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
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


include(FetchContent)

FetchContent_Declare(
  helics
  GIT_REPOSITORY https://github.com/GMLC-TDC/HELICS.git
  GIT_TAG       external_subprojects
)

FetchContent_GetProperties(helics)

if(NOT ${lcName}_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(helics)

endif()


  # Set custom variables, policies, etc.
  # ...

  set(HELICS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(HELICS_USE_EXTERNAL_FMT ON CACHE BOOL "" FORCE)
  set(fmt_FOUND ON CACHE BOOL "" FORCE)
  set(fmt_DIR ON CACHE BOOL "" FORCE)
  set(HELICS_USE_EXTERNAL_JSONCPP ON CACHE BOOL "" FORCE)
  set(HELICS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(HELICS_DISABLE_C_SHARED_LIB ON CACHE BOOL "" FORCE)
  set(HELICS_BUILD_CXX_SHARED_LIB ON CACHE BOOL "" FORCE)
add_subdirectory(${${lcName}_SOURCE_DIR} ${${lcName}_BINARY_DIR})

  #set_target_properties(clang-format clang-format-check clang-format-diff PROPERTIES FOLDER "Extern/zmq_clang_format")
