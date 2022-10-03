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

set(SKIP_INSTALL_LIBRARIES ON)
add_subdirectory(
    ${PROJECT_SOURCE_DIR}/ThirdParty/Zlib/zlib-1.2.6 ${PROJECT_BINARY_DIR}/Zlib EXCLUDE_FROM_ALL
)

add_subdirectory(
    ${PROJECT_SOURCE_DIR}/ThirdParty/Minizip ${PROJECT_BINARY_DIR}/Minizip EXCLUDE_FROM_ALL
)

target_link_libraries(
    utilities PUBLIC minizip::minizip zlib::zlib helics_fmi_base
)

set_target_properties ( zlib minizip PROPERTIES FOLDER Extern)
install(TARGETS utilities LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
