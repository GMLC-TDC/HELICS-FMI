# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(helics-fmi_sources
	helics-fmi.cpp
	)
	
set(helics-fmi_headers
	helics-fmi.h
	)

add_executable(helics-fmi ${helics-fmi_sources} ${helics-fmi_headers})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${formatInterpreter_include_directories})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/fmi)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/utilities)
INCLUDE_DIRECTORIES(${HELICS_INCLUDE_DIR})

target_link_libraries(helics-fmi helics ${external_library_list})

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET helics-fmi POST_BUILD        # Adds a post-build event to helics-fmi
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        $<TARGET_FILE_DIR:helics-fmi>)                 # <--this is out-file path
endforeach(keyfile)

INSTALL(TARGETS helics-fmi RUNTIME DESTINATION bin)
