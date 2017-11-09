# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

#project name

set(utilities_sources
	gridRandom.cpp
	stringOps.cpp
	string_viewOps.cpp
	matrixOps.cpp
	stringConversion.cpp
	units.cpp
	vectorOps.cpp
	functionInterpreter.cpp
	charMapper.cpp
	workQueue.cpp
	zipUtilities.cpp
	base64.cpp
	matrixCreation.cpp
	logger.cpp
	matrixDataSparse.cpp
	stringToCmdLine.cpp
	)

set(utilities_headers
	units.h
	dataDictionary.hpp
	timeSeries.hpp
	charMapper.h
	timeSeriesMulti.hpp
	vectorOps.hpp
	generic_string_ops.hpp
	stringOps.h
	string_viewOps.h
	string_viewDef.h
	optionalDef.hpp
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
	workQueue.h
	simpleQueue.hpp
	zipUtilities.h
	base64.h
	stringConversion.h
	string_viewConversion.h
	mapOps.hpp
	valuePredictor.hpp
	matrixCreation.h
	timeRepresentation.hpp
	indexTypes.hpp
	logger.h
	stringToCmdLine.h
	)

add_library(utilities STATIC ${utilities_sources} ${utilities_headers})

include(escape_string)

escape_string(project_src_dir_string ${PROJECT_SOURCE_DIR})
escape_string(binary_dir_string ${CMAKE_BINARY_DIR})

externalProject_Add(minizip
    SOURCE_DIR ${project_src_dir_string}/ThirdParty/Zlib/zlib-1.2.6
    DOWNLOAD_COMMAND " "
    UPDATE_COMMAND " "
	BINARY_DIR ${binary_dir_string}/ThirdParty/Minizip
	PREFIX  ${binary_dir_string}/libs
    CMAKE_ARGS 
		-DZLIB_SOURCE_DIR=${zlib_source_dir}

		
	INSTALL_DIR ${binary_dir_string}/libs
	)


add_dependencies(utilities minizip)

INSTALL(TARGETS utilities
       LIBRARY DESTINATION lib
       ARCHIVE DESTINATION lib)

