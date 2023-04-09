# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2023, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "")
set(OLD_CMAKE_DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
unset(CMAKE_DEBUG_POSTFIX)

add_subdirectory(${PROJECT_SOURCE_DIR}/ThirdParty/reference ${CMAKE_CURRENT_BINARY_DIR}/fmus)

set(CMAKE_DEBUG_POSTFIX ${OLD_CMAKE_DEBUG_POSTFIX})

set_target_properties(
    BouncingBall
    Dahlquist
    Feedthrough
    Stair
    VanDerPol
    Resource
    PROPERTIES FOLDER "FMUs"
)
