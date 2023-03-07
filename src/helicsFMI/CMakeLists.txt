# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2023, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(helicsFMI_sources FmiCoSimFederate.cpp FmiModelExchangeFederate.cpp FmiHelics.cpp)

set(helicsFMI_headers FmiCoSimFederate.hpp FmiModelExchangeFederate.hpp FmiHelics.hpp)

add_library(helicsFMI STATIC ${helicsFMI_sources} ${helicsFMI_headers})

target_link_libraries(
    helicsFMI PUBLIC fmiLibrary ${HELICS_FMI_HELICS_TARGET_APPS} helics_fmi_base solvers
)

target_include_directories(
    helicsFMI PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                     $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
