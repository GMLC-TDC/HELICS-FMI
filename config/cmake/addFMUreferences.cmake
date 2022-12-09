# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#
# Downloads GTest and provides a helper macro to add tests. Add make check, as well, which gives
# output on failed tests without having to set an environment variable.
#

set(fmu_ref_version v0.0.20)

string(TOLOWER "ref_FMU" refName)

include(FetchContent)
mark_as_advanced(FETCHCONTENT_BASE_DIR)
mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_QUIET)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED)

fetchcontent_declare(
    ref_FMU
    GIT_REPOSITORY https://github.com/modelica/Reference-FMUs.git
    GIT_TAG ${fmu_ref_version}
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
)

fetchcontent_getproperties(ref_FMU)

if(NOT ${refName}_POPULATED)
    # Fetch the content using previously declared details
    fetchcontent_populate(ref_FMU)

endif()
hide_variable(FETCHCONTENT_SOURCE_DIR_REF_FMU)
hide_variable(FETCHCONTENT_UPDATES_DISCONNECTED_REF_FMU)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "")
add_subdirectory(${${refName}_SOURCE_DIR} ${${refName}_BINARY_DIR})

message(STATUS "loading reference test FMU's' ${${refName}_SOURCE_DIR}")


set_target_properties(BouncingBall Dahlquist Feedthrough Stair VanDerPol Resource PROPERTIES FOLDER "FMUs")
