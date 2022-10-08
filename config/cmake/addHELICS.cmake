# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


IF (MSVC)
    set(HELICS_PATH_HINTS
        C:/local/helics_3_3_0
        )
ENDIF(MSVC)

include(GNUInstallDirs)

SHOW_VARIABLE(HELICS_INSTALL_PATH PATH "path to the helics installation" "${PROJECT_BINARY_DIR}/libs")

option(HELICS_FMI_FORCE_HELICS_SUBPROJECT "Force a helics subproject" OFF)

if (HELICS_FMI_FORCE_HELICS_SUBPROJECT)
include(addHELICSsubproject)
else()

option(HELICS_FMI_HELICS_SUBPROJECT "use helics as a subproject" OFF)

set(HELICS_CMAKE_SUFFIXES
    lib/cmake/HELICS/
            cmake/HELICS/)

find_package(HELICS 3.3
    HINTS
        ${HELICS_INSTALL_PATH}
        $ENV{HELICS_INSTALL_PATH}
        ${HELICS_PATH_HINTS}
    PATH_SUFFIXES ${HELICS_CMAKE_SUFFIXES}
    )

if (NOT HELICS_FOUND)
    if (HELICS_FMI_HELICS_SUBPROJECT)
        include(addHELICSsubproject)
    endif()
else()
set(HELICS_FMI_HELICS_TARGET HELICS::helicscpp)
set(HELICS_FMI_HELICS_TARGET_APPS HELICS::helicscpp-apps)
end()


endif()
