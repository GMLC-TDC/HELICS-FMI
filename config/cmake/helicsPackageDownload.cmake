# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(NOT HELICS_CURRENT_VERSION)
    set(HELICS_CURRENT_VERSION 3.3.0)
endif()

set(HELICS_DOWNLOAD_BASE
    "https://github.com/GMLC-TDC/HELICS/releases/download/v${HELICS_CURRENT_VERSION}/"
)

if(MSVC)
    if(MSVC_VERSION LESS_EQUAL 1929)
        set(HELICS_CPP_FILE
            "${HELICS_DOWNLOAD_BASE}/Helics-${HELICS_CURRENT_VERSION}-msvc2019-win64.zip"
        )
        set(HELICS_CPP_SHA 036990acee88a667af17d13fb9d1c67dce6824a596583310003aa02fd97a42d4)
    else()
        set(HELICS_CPP_FILE
            "${HELICS_DOWNLOAD_BASE}/Helics-${HELICS_CURRENT_VERSION}-msvc2022-win64.zip"
        )
        set(HELICS_CPP_SHA a67d3d79898ba0c003efd73e6cadb5c97967403ea354ff1c47579894f2112388)
    endif()

endif()

include(FetchContent)

fetchcontent_declare(helicscpp URL ${HELICS_CPP_FILE} URL_HASH SHA256=${HELICS_CPP_SHA})

fetchcontent_populate(helicscpp)
