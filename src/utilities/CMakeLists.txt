# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
    matrixDataBoost.hpp
    functionInterpreter.h
    zipUtilities.h
    valuePredictor.hpp
    matrixCreation.h
    indexTypes.hpp
    factoryTemplates.hpp
    helperObject.h
    gridRandom.h
)

add_library(utilities STATIC ${utilities_sources} ${utilities_headers})

set(SKIP_INSTALL_LIBRARIES ON)
add_subdirectory(
    ${PROJECT_SOURCE_DIR}/ThirdParty/Zlib/zlib-1.2.6 ${PROJECT_BINARY_DIR}/Zlib EXCLUDE_FROM_ALL
)

add_subdirectory(
    ${PROJECT_SOURCE_DIR}/ThirdParty/Minizip ${PROJECT_BINARY_DIR}/Minizip EXCLUDE_FROM_ALL
)

target_link_libraries(utilities PUBLIC minizip::minizip zlib::zlib helics_fmi_base)

set_target_properties(zlib minizip PROPERTIES FOLDER Extern)
install(TARGETS utilities LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
