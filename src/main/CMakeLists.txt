# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(helics-fmi_sources helics-fmi-main.cpp)

set(helics-fmi_headers)

add_executable(helics-fmi ${helics-fmi_sources} ${helics-fmi_headers})

target_link_libraries(helics-fmi PUBLIC helicsFMI)

install(TARGETS helics-fmi RUNTIME DESTINATION bin)
