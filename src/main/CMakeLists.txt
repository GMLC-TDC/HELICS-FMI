# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(helics-fmi_sources
	helics-fmi-main.cpp
	)

set(helics-fmi_headers
	)

add_executable(helics-fmi ${helics-fmi_sources} ${helics-fmi_headers})


target_link_libraries(helics-fmi helicsFMI)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET helics-fmi POST_BUILD        # Adds a post-build event to helics-fmi
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        $<TARGET_FILE_DIR:helics-fmi>)                 # <--this is out-file path
endforeach(keyfile)

INSTALL(TARGETS helics-fmi RUNTIME DESTINATION bin)
