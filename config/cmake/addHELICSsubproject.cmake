#
# Copyright (c) 2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
#All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# This file is used to add HELICS to a project
#



string(TOLOWER "helics" lcName)

if(NOT CMAKE_VERSION VERSION_LESS 3.11)
include(FetchContent)

FetchContent_Declare(
  helics
  GIT_REPOSITORY https://github.com/GMLC-TDC/HELICS.git
  GIT_TAG       HELICS_EXPORT_TARGET
)

FetchContent_GetProperties(helics)

if(NOT ${lcName}_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(helics)

endif()
else() #cmake <3.11

# create the directory first
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/_deps)

include(GitUtils)
git_clone(
             PROJECT_NAME                    ${lcName}
             GIT_URL                         https://github.com/GMLC-TDC/HELICS.git
             GIT_BRANCH                      HELICS_EXPORT_TARGET
             DIRECTORY                       ${PROJECT_BINARY_DIR}/_deps
       )

set(${lcName}_BINARY_DIR ${PROJECT_BINARY_DIR}/_deps/${lcName}-build)



endif()

  # Set custom variables, policies, etc.
  # ...

  set(BUILD_HELICS_TESTS OFF CACHE BOOL "" FORCE)
  set(BUILD_HELICS_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_C_SHARED_LIB OFF CACHE BOOL "" FORCE)
add_subdirectory(${${lcName}_SOURCE_DIR} ${${lcName}_BINARY_DIR})

  #set_target_properties(clang-format clang-format-check clang-format-diff PROPERTIES FOLDER "Extern/zmq_clang_format")
