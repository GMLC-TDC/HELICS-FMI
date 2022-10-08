# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(helicsFMI_sources FmiCoSimFederate.cpp FmiModelExchangeFederate.cpp)

set(helicsFMI_headers FmiCoSimFederate.hpp FmiModelExchangeFederate.hpp)

add_library(helicsFMI STATIC ${helicsFMI_sources} ${helicsFMI_headers})

target_link_libraries(helicsFMI PUBLIC ${HELICS_FMI_HELICS_TARGET_APPS} fmiLibrary helics_fmi_base solvers)
