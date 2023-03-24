# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2023, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

show_variable(
    HELICS_INSTALL_PATH PATH "path to the helics installation" "${PROJECT_BINARY_DIR}/libs"
)

set(HELICS_CURRENT_VERSION 3.4.0)

if(MSVC)
    set(HELICS_PATH_HINTS C:/local/helics_3_4_0 C:/local/helics_3_4_1)
endif(MSVC)

include(GNUInstallDirs)

show_variable(
    HELICS_INSTALL_PATH PATH "path to the helics installation" "${PROJECT_BINARY_DIR}/libs"
)

option(${PROJECT_NAME}_FORCE_HELICS_SUBPROJECT "Force a helics subproject" OFF)

set(HELICS_MINIMUM_VERSION 3.4)

if(${PROJECT_NAME}_FORCE_HELICS_SUBPROJECT)
    include(addHELICSsubproject)
else()

    option(${PROJECT_NAME}_HELICS_SUBPROJECT "use helics as a subproject" OFF)

    set(HELICS_CMAKE_SUFFIXES lib/cmake/HELICS/ cmake/HELICS/)

    find_package(
        HELICS
        ${HELICS_MINIMUM_VERSION}
        HINTS
        ${HELICS_INSTALL_PATH}
        $ENV{HELICS_INSTALL_PATH}
        ${HELICS_PATH_HINTS}
        PATH_SUFFIXES
        ${HELICS_CMAKE_SUFFIXES}
    )

    if(NOT HELICS_FOUND)
        if(${PROJECT_NAME}_HELICS_SUBPROJECT)

            include(addHELICSsubproject)
        elseif(${PROJECT_NAME}_HELICS_PACKAGE_DOWNLOAD)
            message(STATUS "HELICS FMI helics package download")
            include(helicsPackageDownload)
            if(helicscpp_POPULATED)

                find_package(HELICS ${HELICS_MINIMUM_VERSION} HINTS ${helicscpp_SOURCE_DIR})
                set(${PROJECT_NAME}_HELICS_TARGET HELICS::helicscpp)
                set(${PROJECT_NAME}_HELICS_TARGET_APPS HELICS::helicscpp-apps)
                set(${PROJECT_NAME}_HELICS_EXTERNAL ON)
            endif()
        endif()
    else()
        set(${PROJECT_NAME}_HELICS_TARGET HELICS::helicscpp)
        set(${PROJECT_NAME}_HELICS_TARGET_APPS HELICS::helicscpp-apps)
        set(${PROJECT_NAME}_HELICS_EXTERNAL ON)
    endif()

endif()

if(${PROJECT_NAME}_HELICS_EXTERNAL)
    get_target_property(
        HELICS_BINARY_TARGET ${${PROJECT_NAME}_HELICS_TARGET} IMPORTED_LOCATION_RELEASE
    )
    if(NOT HELICS_BINARY_TARGET)
        get_target_property(
            HELICS_BINARY_TARGET ${${PROJECT_NAME}_HELICS_TARGET} IMPORTED_LOCATION_DEBUG
        )
    endif()
    get_filename_component(HELICS_BINARY_DIR ${HELICS_BINARY_TARGET} DIRECTORY)

    file(GLOB HELICS_BINARIES "${HELICS_BINARY_DIR}/*")
endif()
