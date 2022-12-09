# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(NOT HELICS_CURRENT_VERSION)
    set(HELICS_CURRENT_VERSION 3.3.2)
endif()

set(HELICS_DOWNLOAD_BASE
    "https://github.com/GMLC-TDC/HELICS/releases/download/v${HELICS_CURRENT_VERSION}/"
)

if(MSVC)
    if(MSVC_VERSION LESS_EQUAL 1929)
        set(HELICS_CPP_FILE
            "${HELICS_DOWNLOAD_BASE}/Helics-${HELICS_CURRENT_VERSION}-msvc2019-win64.zip"
        )
        set(HELICS_CPP_SHA dc2a0768d9986b9d9718bb08150340d642d06e5df36aeb4a9ba4fa866abb1110)
    else()
        set(HELICS_CPP_FILE
            "${HELICS_DOWNLOAD_BASE}/Helics-${HELICS_CURRENT_VERSION}-msvc2022-win64.zip"
        )
        set(HELICS_CPP_SHA a7a854b1d8e2b7d95c493f9791bc5a6dc3809345e553777324fd5c81517dee36)
    endif()

endif()

include(FetchContent)

fetchcontent_declare(helicscpp URL ${HELICS_CPP_FILE} URL_HASH SHA256=${HELICS_CPP_SHA})

fetchcontent_populate(helicscpp)
