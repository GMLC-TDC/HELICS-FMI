# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(fmiImport_sources
	fmi_import/fmiImport.cpp
	fmi_import/fmiInfo.cpp
	fmi_import/fmi2Object.cpp
	fmi_import/fmi2ModelExchangeObject.cpp
	fmi_import/fmi2CoSimObject.cpp
	fmi_import/fmiVariableSet.cpp
	fmi_import/fmiEnumDefinitions.cpp
	fmi_import/fmiLibraryManager.cpp
	
	)

set(fmiImport_headers
	fmi_import/fmiInfo.h
	fmi_import/fmiImport.h
	fmi_import/fmiObjects.h
	fmi_import/fmiEnumDefinitions.h
	fmi_import/fmiLibraryManager.h
	)

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

IF(UNIX)
  add_compile_options(-fvisibility=hidden)
ELSE(UNIX)
  IF(MINGW)
 add_compile_options(-fvisibility=hidden)
  ENDIF(MINGW)
ENDIF(UNIX)



add_library(fmiLibrary ${fmiImport_sources} ${fmiImport_headers})

target_link_libraries(fmiLibrary formatInterpreter utilities)

