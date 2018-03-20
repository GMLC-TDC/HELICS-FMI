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

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#version number
set(HELICS_FMI_MAJOR 0)
set(HELICS_FMI_MINOR 3)
set(HELICS_FMI_PATCH 0)
set(HELICS_FMI_DATE 03-02-2018)
set(HELICS_FMI_VERSION "${HELICS_FMI_MAJOR}.${HELICS_FMI_MINOR}.${HELICS_FMI_PATCH}")
set(HELICS_FMI_VERSION_STRING "${HELICS_FMI_VERSION} ${HELIC_FMI_DATE}")

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/config/cmake)

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

FILE(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/logs)

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

include(compiler_flags)

IF(UNIX)
  # Since default builds of boost library under Unix don't use
  # CMake, turn off using CMake build and find include/libs the
  # regular way.
  set(Boost_NO_BOOST_CMAKE ON)
  set(Boost_USE_MULTITHREADED      OFF)   # Needed if MT libraries not built
  option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
ELSE(UNIX)
  IF(MINGW)
  option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
  ELSE(MINGW)
  #this would most likely be MSVC
   option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" ON)
  ENDIF(MINGW)
ENDIF(UNIX)

IF (USE_BOOST_STATIC_LIBS)
  set(Boost_USE_STATIC_LIBS ON)
  set(BOOST_STATIC ON)
ENDIF ()

#message(STATUS ${CMAKE_CXX_FLAGS})

include(ExternalProject)

#########################################################################################

#include(mergestaticlibs)

# -------------------------------------------------------------
# Get some configuration for C++17 as that becomes available
# -------------------------------------------------------------
#message(STATUS ${CMAKE_CXX_FLAGS})
set(CONFIGURE_TARGET_LOCATION ${PROJECT_BINARY_DIR}/libs/include/helics-fmi/)
include(configGenerator)

include(GNUInstallDirs)

# -------------------------------------------------------------
# setting the RPATH
# -------------------------------------------------------------
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/bin" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
ENDIF("${isSystemDir}" STREQUAL "-1")

if (NOT USE_BOOST_STATIC_LIBS)
list(APPEND CMAKE_INSTALL_RPATH ${Boost_LIBRARY_DIRS})
endif()

# -------------------------------------------------------------
# Find (and test) the KLU libraries
# -------------------------------------------------------------


#OPTION(KLU_ENABLE "Enable KLU support" ON)

#if(NOT DEFINED SuiteSparse_DIR)
#  set(SuiteSparse_DIR ${PROJECT_BINARY_DIR}/libs CACHE PATH "path to SuiteSparse/KLU")
#endif()

#SHOW_VARIABLE(SuiteSparse_DIR PATH
#  "KLU library directory" "${SuiteSparse_DIR}")

#if(KLU_ENABLE)
#  IF(MSVC)
#    set(SuiteSparse_USE_LAPACK_BLAS ON)
#  ENDIF(MSVC)
#  find_package(SuiteSparse COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE)
#  if(SuiteSparse_FOUND) 
#        list(APPEND external_library_list ${SuiteSparse_LIBRARIES})
#        #list(APPEND external_link_directories ${SuiteSparse_LIBRARY_DIR})
#        IF (AUTOBUILD_KLU)
#		OPTION(FORCE_KLU_REBUILD "force rebuild of KLU" OFF)
#		IF(FORCE_KLU_REBUILD)
#			include(buildSuiteSparse)
#			build_suitesparse()
#			set(FORCE_KLU_REBUILD OFF CACHE BOOL "force rebuild of KLU" FORCE)
#		ENDIF(FORCE_KLU_REBUILD)
#	ENDIF(AUTOBUILD_KLU)
#  else(SuiteSparse_FOUND)
#	OPTION(AUTOBUILD_KLU "enable Suitesparse to automatically download and build" OFF)
#	IF (AUTOBUILD_KLU)
#		include(buildSuiteSparse)
#		build_suitesparse()
#		set(SuiteSparse_DIR ${PROJECT_BINARY_DIR}/libs)
#		find_package(SuiteSparse COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE)
#	ENDIF(AUTOBUILD_KLU)
#	if (SuiteSparse_FOUND)
#	   list(APPEND external_library_list ${SuiteSparse_LIBRARIES})
#	else()
#		message( FATAL_ERROR "KLU not functional - support will not be provided")
#		message( "Double check spelling specified libraries (search is case sensitive)")
#	endif()
# endif(SuiteSparse_FOUND)

#endif(KLU_ENABLE)

# -------------------------------------------------------------
# Sundials
# -------------------------------------------------------------

include(addSundials)
# -------------------------------------------------------------
# BOOST
# -------------------------------------------------------------

include(addBoost)

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

	SHOW_VARIABLE(DOXYGEN_OUTPUT_DIR PATH "location to put Doxygen docs" "${PROJECT_BINARY_DIR}/docs")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${DOXYGET_OUTPUT_DIR}
	COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
endif (HELICS_FMI_GENERATE_DOXYGEN_DOC)


include(GNUInstallDirs)

# -------------------------------------------------------------
# setting the RPATH
# -------------------------------------------------------------
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/bin" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
ENDIF("${isSystemDir}" STREQUAL "-1")

if (NOT Boost_USE_STATIC_LIBS)
list(APPEND CMAKE_INSTALL_RPATH ${Boost_LIBRARY_DIRS})
endif()

# -------------------------------------------------------------
# Enable HELICS executable
# -------------------------------------------------------------

SHOW_VARIABLE(HELICS_INSTALL_PATH PATH "path to the helics installation" "${PROJECT_BINARY_DIR}/libs")

if(EXISTS "${HELICS_INSTALL_PATH}/cmake/HELICS/HELICSImport.cmake")
	IF (AUTOBUILD_HELICS)
		OPTION(FORCE_HELICS_REBUILD "force rebuild of helics" OFF)
		IF(FORCE_HELICS_REBUILD)
			include(buildHELICS)
			build_helics()
			set(HELICS_INSTALL_PATH ${PROJECT_BINARY_DIR}/libs)
			set(FORCE_HELICS_REBUILD OFF CACHE BOOL "force rebuild of helics" FORCE)
		ENDIF(FORCE_HELICS_REBUILD)
	ENDIF(AUTOBUILD_HELICS)
	include(${HELICS_INSTALL_PATH}/cmake/HELICS/HELICSImport.cmake)
	set(HELICS_FOUND TRUE)
else()
   OPTION(AUTOBUILD_HELICS "enable HELICS to automatically download and build" OFF)
	IF(AUTOBUILD_HELICS)
		include(buildHELICS)
		build_helics()
		set(HELICS_INSTALL_PATH ${PROJECT_BINARY_DIR}/libs)
		set(HELICS_FOUND TRUE)
		include(${HELICS_INSTALL_PATH}/cmake/HELICS/HELICSImport.cmake)
	ELSE(AUTOBUILD_HELICS)
		 message( FATAL_ERROR "Unable to locate HELICS library")
	ENDIF(AUTOBUILD_HELICS)
endif()



FILE(GLOB KEY_LIBRARY_FILES  ${PROJECT_BINARY_DIR}/libs/bin/*)
message(STATUS "key files ${KEY_LIBRARY_FILES}")


add_subdirectory(src/main)

# -------------------------------------------------------------
# Enable clang analysis and formatting tools
# -------------------------------------------------------------

OPTION(ENABLE_CLANG_TOOLS "if clang is found enable some custom targets for clang formatting and tidy" OFF)

if (ENABLE_CLANG_TOOLS)
include(clang-cxx-dev-tools)
endif(ENABLE_CLANG_TOOLS)



INSTALL(FILES ${KEY_LIBRARY_FILES} DESTINATION bin)


CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/config/helics-fmi-config.h.in ${PROJECT_BINARY_DIR}/libs/include/helics-fmi/helics-fmi-config.h)

#-----------------------------------------------------------------------------
# Setup CTEST environment
#-----------------------------------------------------------------------------
OPTION(BUILD_HELICS_FMI_TESTS "Enable the test Executables to be built" ON)
# enable testing
if (BUILD_HELICS_FMI_TESTS)
include (CTest)
enable_testing ()
add_subdirectory (tests)
endif(BUILD_HELICS_FMI_TESTS)


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
set(CPACK_PACKAGE_VERSION ${HELICS_FMI_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${HELICS_FMI_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${HELICS_FMI_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${HELICS_FMI_PATCH})

if (WIN32)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}\\\\LICENSE")
else(WIN32)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif(WIN32)

 if (WIN32)
#	set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\\\docs\\\\img\\\\HELICS.ico")
#	set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/docs/img/HELICS.ico")
#	set(CPACK_NSIS_INSTALL_ROOT "C:\\\\local\\\\")
	set(CPACK_NSIS_URL_INFO_ABOUT "https://www.github.com/GMLC-TDC/Helics-fmi")
set(CPACK_NSIS_MENU_LINKS 
	"https://www.github.com/GMLC-TDC/Helics-fmi" "source code"
	"https://gmlc-tdc.github.io/HELICS-src/" "Helics Documentation"
	"https://www.helics-fmi.org" "Helics FMI Web page"
	"https://www.youtube.com/channel/UCPa81c4BVXEYXt2EShTzbcg" "TDC YouTube channel")
else (WIN32)
	#set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/docs/img/HELICS.ico")
endif(WIN32)


set(CPACK_SOURCE_IGNORE_FILES  "/Build*/;/build*/;/.git/")

#THIS LINE MUST BE LAST
include(CPack)
