# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2023, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(NOT HELICS_CURRENT_VERSION)
    set(HELICS_CURRENT_VERSION 3.4.0)
endif()

set(HELICS_DOWNLOAD_BASE
    "https://github.com/GMLC-TDC/HELICS/releases/download/v${HELICS_CURRENT_VERSION}/"
)

if(MSVC)
    if(MSVC_VERSION LESS_EQUAL 1929)
        set(HELICS_CPP_FILE
            "${HELICS_DOWNLOAD_BASE}/Helics-${HELICS_CURRENT_VERSION}-msvc2019-win64.zip"
        )
        set(HELICS_CPP_SHA eb541c958a689e5692c78fd55e8d98051dc6a66741b0de1eaf0f266becaa01e3)
    else()
        set(HELICS_CPP_FILE
            "${HELICS_DOWNLOAD_BASE}/Helics-${HELICS_CURRENT_VERSION}-msvc2022-win64.zip"
        )
        set(HELICS_CPP_SHA 095e230171272aca49ed94bebba8c1763599929005cc97d1c817c3f9c8391a7a)
    endif()

endif()

include(FetchContent)

fetchcontent_declare(helicscpp URL ${HELICS_CPP_FILE} URL_HASH SHA256=${HELICS_CPP_SHA})

fetchcontent_populate(helicscpp)
