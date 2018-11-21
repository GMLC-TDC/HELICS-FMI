# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(helicsFMI_sources
	FmiCoSimFederate.cpp
	FmiModelExchangeFederate.cpp
	basicOdeSolver.cpp
	arkodeInterface.cpp
	sundialsInterface.cpp
	cvodeInterface.cpp
	)

set(helicsFMI_headers
	FmiCoSimFederate.hpp
	FmiModelExchangeFederate.hpp
	sundialsInterface.h
	basicOdeSolver.h
	arkodeInterface.h
	cvodeInterface.h
	solverInterface.h
	)

add_library(helicsFMI STATIC ${helicsFMI_sources} ${helicsFMI_headers})

target_link_libraries(helicsFMI HELICS::helics-static fmiLibrary helics_fmi_base )

