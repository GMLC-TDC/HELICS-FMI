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
project(helics-fmi)

#states that Cmake version > 3.4
cmake_minimum_required(VERSION 3.4)
cmake_policy(VERSION 3.4)

#version number
set(HELICS_FMI_MAJOR 0)
set(HELICS_FMI_MINOR 2)
set(HELICS_FMI_PATCH 2)
set(HELICS_FMI_DATE 11-07-2017)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/config)

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# -------------------------------------------------------------
# MACRO definitions
# -------------------------------------------------------------

# Macros to hide/show cached variables.
# These two macros can be used to "hide" or "show" in the
# list of cached variables various variables and/or options 
# that depend on other options.
# Note that once a variable is modified, it will preserve its
# value (hiding it merely makes it internal)

MACRO(HIDE_VARIABLE var)
  IF(DEFINED ${var})
    SET(${var} "${${var}}" CACHE INTERNAL "")
  ENDIF(DEFINED ${var})
ENDMACRO(HIDE_VARIABLE)

MACRO(SHOW_VARIABLE var type doc default)
  IF(DEFINED ${var})
    SET(${var} "${${var}}" CACHE "${type}" "${doc}" FORCE)
  ELSE(DEFINED ${var})
    SET(${var} "${default}" CACHE "${type}" "${doc}")
  ENDIF(DEFINED ${var})
ENDMACRO(SHOW_VARIABLE)


# Prohibit in-source build
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source build is not supported. Please, use an empty directory for building the project.")
ENDIF()

OPTION(ENABLE_EXTRA_COMPILER_WARNINGS "disable compiler warning for helics-fmi build" ON)


# -------------------------------------------------------------
# Setup compiler options and configurations
# -------------------------------------------------------------
message(STATUS "setting up for ${CMAKE_CXX_COMPILER_ID}")
IF(UNIX)
  # Since default builds of boost library under Unix don't use
  # CMake, turn off using CMake build and find include/libs the
  # regular way.
  set(Boost_NO_BOOST_CMAKE ON)

  set(Boost_USE_MULTITHREADED      OFF)   # Needed if MT libraries not built
  if (ENABLE_EXTRA_COMPILER_WARNINGS)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wall>)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-pedantic>)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing=1>)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-overflow=5>)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wredundant-decls>)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wcast-align>)
  endif(ENABLE_EXTRA_COMPILER_WARNINGS)
ELSE(UNIX)
  IF(MINGW)
  if (ENABLE_EXTRA_COMPILER_WARNINGS)
 add_compile_options(-Wall -pedantic)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing=1>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-overflow=5>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wredundant-decls>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wcast-align>)
  #this options produces lots of warning but is useful for checking ever once in a while with Clang GCC warnin notices with this aren't as useful
  #add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wpadded>)
  endif(ENABLE_EXTRA_COMPILER_WARNINGS)
  ENDIF(MINGW)
  
# -------------------------------------------------------------
# Extra definitions for visual studio
# -------------------------------------------------------------
  IF(MSVC)
  set(Boost_USE_STATIC_LIBS ON)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
  add_compile_options(/EHsc)
  if (ENABLE_EXTRA_COMPILER_WARNINGS)
   add_compile_options(-W4 /wd4065 /wd4101 /wd4102 /wd4244 /wd4297 /wd4355 /wd4800 /wd4484)
  endif(ENABLE_EXTRA_COMPILER_WARNINGS)
 
ENDIF(MSVC)
ENDIF(UNIX)

#message(STATUS ${CMAKE_CXX_FLAGS})
include(configGenerator)

IF (VERSION_OPTION)
	add_compile_options(${VERSION_OPTION})
ELSE ()
set(CMAKE_CXX_STANDARD 14)
ENDIF ()

include(ExternalProject)

#########################################################################################

#include(mergestaticlibs)


# -------------------------------------------------------------
# Find multithreading headers and includes
# -------------------------------------------------------------

IF (NOT DISABLE_MULTITHREADING)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
list(APPEND external_library_list Threads::Threads)
ENDIF(NOT DISABLE_MULTITHREADING)

# -------------------------------------------------------------
# Find (and test) the KLU libraries
# -------------------------------------------------------------


OPTION(KLU_ENABLE "Enable KLU support" ON)

if(NOT DEFINED SuiteSparse_DIR)
  set(SuiteSparse_DIR ${PROJECT_BINARY_DIR}/libs CACHE PATH "path to SuiteSparse/KLU")
endif()

SHOW_VARIABLE(SuiteSparse_DIR PATH
  "KLU library directory" "${SuiteSparse_DIR}")

if(KLU_ENABLE)
  IF(MSVC)
    set(SuiteSparse_USE_LAPACK_BLAS ON)
  ENDIF(MSVC)
  find_package(SuiteSparse COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE)
  if(SuiteSparse_FOUND) 
        list(APPEND external_library_list ${SuiteSparse_LIBRARIES})
        #list(APPEND external_link_directories ${SuiteSparse_LIBRARY_DIR})
        IF (AUTOBUILD_KLU)
		OPTION(FORCE_KLU_REBUILD "force rebuild of KLU" OFF)
		IF(FORCE_KLU_REBUILD)
			include(buildSuiteSparse)
			build_suitesparse()
			set(FORCE_KLU_REBUILD OFF CACHE BOOL "force rebuild of KLU" FORCE)
		ENDIF(FORCE_KLU_REBUILD)
	ENDIF(AUTOBUILD_KLU)
  else(SuiteSparse_FOUND)
	OPTION(AUTOBUILD_KLU "enable Suitesparse to automatically download and build" OFF)
	IF (AUTOBUILD_KLU)
		include(buildSuiteSparse)
		build_suitesparse()
		set(SuiteSparse_DIR ${PROJECT_BINARY_DIR}/libs)
		find_package(SuiteSparse COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE)
	ENDIF(AUTOBUILD_KLU)
	if (SuiteSparse_FOUND)
	   list(APPEND external_library_list ${SuiteSparse_LIBRARIES})
	else()
		message( FATAL_ERROR "KLU not functional - support will not be provided")
		message( "Double check spelling specified libraries (search is case sensitive)")
	endif()
  endif(SuiteSparse_FOUND)

endif(KLU_ENABLE)

# -------------------------------------------------------------
# Sundials
# -------------------------------------------------------------

if(NOT DEFINED SUNDIALS_DIR)
  set(SUNDIALS_DIR ${PROJECT_BINARY_DIR}/libs CACHE PATH "path to SUNDIALS")
endif()

SHOW_VARIABLE(SUNDIALS_DIR PATH
  "SUNDIALS library directory" "${SUNDIALS_DIR}")
  
  set(SUNDIALS_FIND_QUIETLY ON)
  
  set(SUNDIALS_REQUIRED IDA KINSOL)
  if (OPENMP_FOUND)
  list(APPEND SUNDIALS_REQUIRED nvecopenmp)
  endif(OPENMP_FOUND)
find_package(SUNDIALS REQUIRED COMPONENTS ${SUNDIALS_REQUIRED} OPTIONAL_COMPONENTS CVODE ARKODE)

  if(SUNDIALS_FOUND) 
	list(INSERT external_library_list 0 ${SUNDIALS_LIBRARIES})
  OPTION(FORCE_SUNDIALS_REBUILD "force rebuild of sundials" OFF)
	IF (AUTOBUILD_SUNDIALS)
		
		IF(FORCE_SUNDIALS_REBUILD)
			include(buildSundials)
			build_sundials()
			set(FORCE_SUNDIALS_REBUILD OFF CACHE BOOL "force rebuild of sundials" FORCE)
		ENDIF(FORCE_SUNDIALS_REBUILD)
	ELSE (AUTOBUILD_SUNDIALS)
	   IF(FORCE_SUNDIALS_REBUILD)
			include(buildSundials)
			build_sundials()
			set(SUNDIALS_FOUND OFF CACHE BOOL "sundials not found" FORCE)
			set(SUNDIALS_LIBRARIES NOT_FOUND CACHE FORCE)
			set(FORCE_SUNDIALS_REBUILD OFF CACHE BOOL "force rebuild of sundials" FORCE)
			set(SUNDIALS_DIR ${PROJECT_BINARY_DIR}/libs)
			find_package(SUNDIALS REQUIRED COMPONENTS ${SUNDIALS_REQUIRED} OPTIONAL_COMPONENTS CVODE ARKODE)
		ENDIF(FORCE_SUNDIALS_REBUILD)
	ENDIF(AUTOBUILD_SUNDIALS)
  else(SUNDIALS_FOUND)
    OPTION(AUTOBUILD_SUNDIALS "enable Sundials to automatically download and build" OFF)
    message(STATUS "abs=${AUTOBUILD_SUNDIALS}")
    IF (AUTOBUILD_SUNDIALS)
      include(buildSundials)
      build_sundials()
      set(SUNDIALS_DIR ${PROJECT_BINARY_DIR}/libs)
      find_package(SUNDIALS REQUIRED COMPONENTS ${SUNDIALS_REQUIRED} OPTIONAL_COMPONENTS CVODE ARKODE)
    ENDIF(AUTOBUILD_SUNDIALS)
  if (SUNDIALS_FOUND)
    list(INSERT external_library_list 0 ${SUNDIALS_LIBRARIES})
  else (SUNDIALS_FOUND)
	message( FATAL_ERROR "sundials not found - unable to continue")
	message( "Double check spelling specified libraries (search is case sensitive)")
  endif(SUNDIALS_FOUND)
  endif(SUNDIALS_FOUND)


if (SUNDIALS_ARKODE_FOUND)
set(LOAD_ARKODE TRUE)
endif(SUNDIALS_ARKODE_FOUND)

if (SUNDIALS_CVODE_FOUND)
set(LOAD_CVODE TRUE)
endif(SUNDIALS_CVODE_FOUND)

set(optional_component_test_files)
set(optional_system_test_files)

set(helics_fmi_optional_libraries )

set (optional_library_key_headers )
set (optional_library_functions )

# -------------------------------------------------------------
# BOOST
# -------------------------------------------------------------

IF (MSVC)
find_path(BOOST_TEST_PATH 
			NAMES 			boost/version.hpp
			PATHS		C:/boost_1_65_0
						C:/boost_1_64_0
						C:/boost_1_63_0
						C:/boost_1_61_0
						D:/boost_1_65_0
						D:/boost_1_64_0
						D:/boost_1_63_0
						D:/boost_1_61_0
						C:/boost/boost_1_65_0
						C:/boost/boost_1_64_0
						C:/boost/boost_1_63_0
						C:/boost/boost_1_61_0
						D:/boost/boost_1_65_0
						D:/boost/boost_1_64_0
						D:/boost/boost_1_63_0
						D:/boost/boost_1_61_0
						C:/local/boost_1_65_0
						C:/local/boost_1_64_0
						C:/local/boost_1_63_0
						C:/local/boost_1_61_0
						D:/local/boost_1_65_0
						D:/local/boost_1_64_0
						D:/local/boost_1_63_0
						D:/local/boost_1_61_0
		)
		
		if (BOOST_TEST_PATH)
		set(BOOST_ROOT ${BOOST_TEST_PATH})
		endif(BOOST_TEST_PATH)
ENDIF(MSVC)

SHOW_VARIABLE(BOOST_ROOT PATH "Boost root directory" "${BOOST_ROOT}")

# Minimum version of Boost required 
set(BOOST_MINIMUM_VERSION 1.61)

IF(MPI_ENABLE)
  find_package(Boost ${BOOST_MINIMUM_VERSION} COMPONENTS program_options filesystem serialization mpi system date_time REQUIRED)
ELSE(MPI_ENABLE)
  find_package(Boost ${BOOST_MINIMUM_VERSION} COMPONENTS program_options filesystem system date_time REQUIRED)
ENDIF(MPI_ENABLE)

mark_as_advanced(CLEAR BOOST_ROOT)

message("Using Boost include files : ${Boost_INCLUDE_DIR}")
message("Using Boost libraries ${Boost_LIBRARY_DIRS}")

list(APPEND external_library_list ${Boost_LIBRARIES})
  list(APPEND external_link_directories ${Boost_LIBRARY_DIRS})

# -------------------------------------------------------------
# global include directories
# -------------------------------------------------------------
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/libs/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/ThirdParty)
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})
# -------------------------------------------------------------
# load the required subdirectories
# -------------------------------------------------------------

add_subdirectory(src/formatInterpreters)

add_subdirectory(src/utilities)

add_subdirectory(src/fmi)


option (HELICS_FMI_GENERATE_DOXYGEN_DOC "Generate Doxygen doc target" OFF)

IF (HELICS_FMI_GENERATE_DOXYGEN_DOC)
find_package(Doxygen)
if(DOXYGEN_FOUND)

	SHOW_VARIABLE(DOXYGEN_OUTPUT_DIR PATH "location to put Doxygen docs" "${CMAKE_CURRENT_SOURCE_DIR}/docs")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs}
	COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
endif (HELICS_FMI_GENERATE_DOXYGEN_DOC)

# -------------------------------------------------------------
# Enable HELICS executable
# -------------------------------------------------------------

SHOW_VARIABLE(HELICS_INSTALL_PATH PATH "path to the helics installation" "${PROJECT_BINARY_DIR}/libs")
	
	set(HELICS_INSTALL_PATH ${PROJECT_BINARY_DIR}/libs)
	find_package(HELICS)
	IF(NOT HELICS_FOUND)
		OPTION(AUTOBUILD_HELICS "enable HELICS to automatically download and build" OFF)
		IF(AUTOBUILD_HELICS)
			include(buildHELICS)
			build_helics()
			set(HELICS_INSTALL_PATH ${PROJECT_BINARY_DIR}/libs)
			find_package(HELICS)
		ENDIF(AUTOBUILD_HELICS)
	ELSE (NOT HELICS_FOUND)
		OPTION(FORCE_HELICS_REBUILD "force rebuild of helics" OFF)
		IF (AUTOBUILD_HELICS)
			IF(FORCE_HELICS_REBUILD)
				include(buildHELICS)
				build_helics()
				set(HELICS_INSTALL_PATH ${PROJECT_BINARY_DIR}/libs)
				set(FORCE_HELICS_REBUILD OFF CACHE BOOL "force rebuild of helics" FORCE)
			ENDIF(FORCE_HELICS_REBUILD)
		ENDIF(AUTOBUILD_HELICS)
	ENDIF(NOT HELICS_FOUND)
	
add_subdirectory(src/main)

FILE(GLOB KEY_LIBRARY_FILES  ${PROJECT_BINARY_DIR}/libs/bin/*)
message(STATUS "key files ${KEY_LIBRARY_FILES}")

# -------------------------------------------------------------
# Enable clang analysis and formatting tools
# -------------------------------------------------------------

OPTION(ENABLE_CLANG_TOOLS "if clang is found enable some custom targets for clang formatting and tidy" OFF)

if (ENABLE_CLANG_TOOLS)
include(clang-cxx-dev-tools)
endif(ENABLE_CLANG_TOOLS)


FILE(GLOB efiles "examples/*")
INSTALL(FILES ${efiles} DESTINATION examples)


INSTALL(FILES ${KEY_LIBRARY_FILES} DESTINATION bin)


# -------------------------------------------------------------
# Future Additions 
# -------------------------------------------------------------

#adding dlls
# INSTALL(FILES ${LOCATION_OF_FILES} DESTINATION bin)
#FILE(GLOB docs "docs/manuals/*")
#INSTALL(FILES ${docs} DESTINATION docs)



# -------------------------------------------------------------
# CPack for NSIS Installer
# -------------------------------------------------------------

set(CPACK_PACKAGE_NAME "HELICS-fmi")
set(CPACK_PACKAGE_VENDOR "Lawrence Livermore National Security")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Helics-fmi Installer")
set(CPACK_PACKAGE_VERSION "${HELICS_FMI_MAJOR_VERSION}.${HELICS_FMI_MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR ${HELICS_FMI_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${HELICS_FMI_MINOR_VERSION})
#set(CPACK_PACKAGE_VERSION_PATCH ${HELICS_FMI_VERSION_PATCH})

#THIS LINE MUST BE LAST
include(CPack)
