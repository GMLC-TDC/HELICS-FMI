# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(helics-fmi_sources helics-fmi-main.cpp)

set(helics-fmi_headers)

add_executable(helics-fmi ${helics-fmi_sources} ${helics-fmi_headers})

target_link_libraries(helics-fmi PUBLIC helicsFMI)

install(TARGETS helics-fmi RUNTIME DESTINATION bin)
