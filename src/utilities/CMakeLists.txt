# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

# project name

set(utilities_sources
    gridRandom.cpp
    functionInterpreter.cpp
    zipUtilities.cpp
    matrixCreation.cpp
    matrixDataSparse.cpp
    helperObject.cpp
)

set(utilities_headers
    dataDictionary.hpp
    matrixData.hpp
    matrixOps.h
    matrixDataCompact.hpp
    matrixDataSparse.hpp
    matrixDataSparse_impl.hpp
    matrixDataTranslate.hpp
    matrixDataContainer.hpp
    matrixDataScale.hpp
    matrixDataOrdered.hpp
    matrixDataOrdering.hpp
    matrixDataFilter.hpp
    matrixDataCustomWriteOnly.hpp
    functionInterpreter.h
    zipUtilities.h
    valuePredictor.hpp
    matrixCreation.h
    indexTypes.hpp
    factoryTemplates.hpp
    helperObject.h
)

add_library(utilities STATIC ${utilities_sources} ${utilities_headers})

include(buildZlib)

include(buildMiniZip)
# ##################################################################################################
# building zlib
# ##################################################################################################
set(ZLIB_ROOT_DIR ${PROJECT_BINARY_DIR}/libs)

if(NOT EXISTS ${PROJECT_BINARY_DIR}/libs/cmake/zlib-targets.cmake)
    build_zlib()
endif()
include(${PROJECT_BINARY_DIR}/libs/cmake/zlib-targets.cmake)

# ##################################################################################################
# building minizip
# ##################################################################################################
set(ZLIB_INCLUDES ${PROJECT_BINARY_DIR}/libs/include)
if(NOT EXISTS ${PROJECT_BINARY_DIR}/libs/cmake/minizip-targets.cmake)
    build_minizip()
endif()
include(${PROJECT_BINARY_DIR}/libs/cmake/minizip-targets.cmake)

target_link_libraries(
    utilities PUBLIC minizip::minizip zlib::zlib helics_fmi_base
)

install(TARGETS utilities LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
