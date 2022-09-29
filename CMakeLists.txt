# ~~~
# Copyright (c) 2017-2022 Lawrence Livermore
# National Security, LLC;
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~

cmake_minimum_required(VERSION 3.17...3.24)

# Install dependencies using vcpkg if VCPKG_ROOT is set and no CMake Toolchain file is given vcpkg
# installation on a system doesn't set VCPKG_ROOT, so setting it should be like an opt-in for users
option(HELICS-FMI_DISABLE_VCPKG "Force CMake to ignore VCPKG_ROOT even if it is set" OFF)
mark_as_advanced(HELICS-FMI_DISABLE_VCPKG)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE AND NOT HELICS_DISABLE_VCPKG)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(HELICS-FMI VERSION 0.7.0)

#set (HELICS-FMI_VERSION_BUILD )
set (HELICS-FMI_DATE "2022-10-01")
set(
  HELICS-FMI_VERSION_UNDERSCORE
  "${HELICS-FMI_VERSION_MAJOR}_${HELICS-FMI_VERSION_MINOR}_${HELICS-FMI_VERSION_PATCH}"
)
if(HELICS-FMI_VERSION_BUILD)
  set(HELICS-FMI_VERSION "${HELICS-FMI_VERSION}-${HELICS-FMI_VERSION_BUILD}")
  set(HELICS-FMI_VERSION_UNDERSCORE "${HELICS-FMI_VERSION_UNDERSCORE}-${HELICS-FMI_VERSION_BUILD}")
endif()
set(HELICS-FMI_VERSION_STRING "${HELICS-FMI_VERSION} (${HELICS-FMI_DATE})")

# -----------------------------------------------------------------------------
# set the module path and include some common macros
# -----------------------------------------------------------------------------

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/config/cmake/"
                          "${PROJECT_SOURCE_DIR}/ThirdParty/cmake/"
    )
# -----------------------------------------------------------------------------
# set the module path and include some common macros
# -----------------------------------------------------------------------------

include(extraMacros)
include(CMakeDependentOption)
include(CTest)
include(ucm)

# -----------------------------------------------------------------------------
# set the install path to a local directory
# -----------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(WIN32)
        if(MSVC)
            set(CMAKE_INSTALL_PREFIX "C:/local/helics-fmi_${HELICS_VERSION_UNDERSCORE}/"
                CACHE PATH "default install path" FORCE
            )
        elseif(MINGW AND NOT MSYS)
            set(CMAKE_INSTALL_PREFIX "C:/local/helics-fmi_${HELICS_VERSION_UNDERSCORE}/"
                CACHE PATH "default install path" FORCE
            )
        elseif(MSYS)
            # use CMAKE_OBJCOPY here since it is somewhat less likely to be overridden by users
            # rather than the compiler
            get_filename_component(path_bin ${CMAKE_OBJCOPY} DIRECTORY)
            get_filename_component(path_install ${path_bin} DIRECTORY)
            set(CMAKE_INSTALL_PREFIX ${path_install} CACHE PATH "default install path" FORCE)
        endif(MSVC)
    endif(WIN32)
endif()

# Warning if CMAKE_INSTALL_PREFIX is empty. Likely set by using the wrong environment variable.
if(NOT CMAKE_INSTALL_PREFIX)
    message(
        WARNING
            "CMAKE_INSTALL_PREFIX is set to nothing. If you are using an environment variable for handling prefix paths, that variable might not have been set before using it with CMake to set the CMAKE_INSTALL_PREFIX option."
    )
endif()

# Check to make sure the install prefix isn't the build folder, if it is, build errors will happen
get_filename_component(tmp_install_prefix "${CMAKE_INSTALL_PREFIX}" REALPATH)
get_filename_component(tmp_proj_bindir "${PROJECT_BINARY_DIR}" REALPATH)
# Windows paths are case insensitive
if(WIN32)
    string(TOLOWER "${tmp_install_prefix}" tmp_install_prefix)
    string(TOLOWER "${tmp_proj_bindir}" tmp_proj_bindir)
endif()
if(tmp_install_prefix STREQUAL tmp_proj_bindir)
    message(FATAL_ERROR "CMAKE_INSTALL_PREFIX must not be set to the build folder")
endif()

if(MSYS
   OR CYGWIN
   OR UNIX
   OR APPLE
)
    set(UNIX_LIKE TRUE)
endif()

# Set the build output paths
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH
                                                                           "Archive output dir."
        )
    endif()
    if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH
                                                                           "Library output dir."
        )
    endif()
    if(NOT CMAKE_PDB_OUTPUT_DIRECTORY)
        set(CMAKE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            CACHE PATH "PDB (MSVC debug symbol)output dir."
        )
    endif()
    if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            CACHE PATH "Executable/dll output dir."
        )
    endif()
endif()

if(NOT TARGET compile_flags_target)
    add_library(compile_flags_target INTERFACE)
endif()

if(NOT TARGET build_flags_target)
    add_library(build_flags_target INTERFACE)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    mark_as_advanced(BUILD_TESTING)
    include(compiler_flags)

endif()
	
# add a baseline library for underlying dependencies and flags
add_library(helics_fmi_base INTERFACE)

target_link_libraries(helics_fmi_base INTERFACE compile_flags_target)


add_library(HELICS-FMI::compile_flags_target ALIAS compile_flags_target)
add_library(HELICS-FMI::build_flags_target ALIAS build_flags_target)

get_target_property(EXTRA_BUILD_FLAGS build_flags_target INTERFACE_COMPILE_OPTIONS)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(NOT USE_LIBCXX)
        show_variable(STATIC_STANDARD_LIB STRING "Link against a static standard lib" default)
        set_property(CACHE STATIC_STANDARD_LIB PROPERTY STRINGS default static dynamic)
    else()
        hide_variable(STATIC_STANDARD_LIB)
    endif()
    if(MSVC)
        show_variable(HELICS-FMI_EMBEDDED_DEBUG_INFO STRING "embed debug info into lib files" default)
        set_property(CACHE HELICS-FMI_EMBEDDED_DEBUG_INFO PROPERTY STRINGS default embedded external)
    else()
        hide_variable(HELICS-FMI_EMBEDDED_DEBUG_INFO)
    endif()
endif()

if(STATIC_STANDARD_LIB STREQUAL "default")

elseif(STATIC_STANDARD_LIB STREQUAL "static")
    ucm_set_runtime(STATIC)
elseif(STATIC_STANDARD_LIB STREQUAL "dynamic")
    ucm_set_runtime(DYNAMIC)
endif()

if(HELICS-FMI_EMBEDDED_DEBUG_INFO STREQUAL "default")

elseif(HELICS-FMI_EMBEDDED_DEBUG_INFO STREQUAL "external")
    ucm_set_embedded_debug(EXTERNAL)
else()
    ucm_set_embedded_debug(EMBEDDED)
endif()


#########################################################################################

#include(mergestaticlibs)

include(GNUInstallDirs)

# -------------------------------------------------------------
# BOOST  find the boost libraries
# -------------------------------------------------------------
set(
  BOOST_REQUIRED_LIBRARIES
  filesystem
  system
)
include(addBoost)



target_link_libraries(helics_fmi_base INTERFACE Boostlibs::core)

# ------------------------------------------------------------
# Find (and test) the KLU libraries
# ------------------------------------------------------------
if(ENABLE_KLU)
    include(addKLU_targets)
endif()


# -------------------------------------------------------------
# Sundials
# -------------------------------------------------------------

include(addSundials)

# -------------------------------------------------------------
# setting the RPATH
# -------------------------------------------------------------
# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set(CMAKE_MACOSX_RPATH ON)
set(
  CMAKE_BUILD_RPATH
  "./;${AUTOBUILD_INSTALL_PATH}/bin;${AUTOBUILD_INSTALL_PATH}/lib;${AUTOBUILD_INSTALL_PATH}/lib64"
)

# add the automatically determined parts of the RPATH which point to directories outside
# the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(
  FIND
    CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}" isSystemDir
)
if("${isSystemDir}" STREQUAL "-1")
  list(
    APPEND
      CMAKE_INSTALL_RPATH CMAKE_INSTALL_RPATH
      "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
  )
endif()

list(
  FIND
    CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir
)
if("${isSystemDir}" STREQUAL "-1")
    list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif()

if(NOT Boost_USE_STATIC_LIBS)
	list(APPEND CMAKE_INSTALL_RPATH ${Boost_LIBRARY_DIRS})
	list(APPEND CMAKE_BUILD_RPATH ${Boost_LIBRARY_DIRS})
endif()

# add the local directory to the rpath
list(APPEND CMAKE_INSTALL_RPATH "./")


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

# -------------------------------------------------------------
# Enable HELICS executable
# -------------------------------------------------------------

SHOW_VARIABLE(HELICS_INSTALL_PATH PATH "path to the helics installation" "${PROJECT_BINARY_DIR}/libs")

include(addHELICS)

add_subdirectory(src/formatInterpreters)

add_subdirectory(src/utilities)

add_subdirectory(src/fmi)



add_subdirectory(src/solvers)
add_subdirectory(src/helicsFMI)

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
OPTION(ENABLE_PACKAGE_BUILD "Add projects for making packages and installers for HELICS" OFF)

IF(ENABLE_PACKAGE_BUILD)
set(CPACK_PACKAGE_NAME "HELICS-fmi")
set(CPACK_PACKAGE_VENDOR "Lawrence Livermore National Security")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Helics-fmi Installer")
set(CPACK_PACKAGE_VERSION ${HELICS-FMI_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${HELICS-FMI_VERISON_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${HELICS-FMI_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${HELICS-FMI_VERSION_PATCH})

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
	"https://helics.readthedocs.io/en/latest/" "Helics Documentation"
	"https://www.helics-fmi.org" "Helics FMI Web page"
	"https://www.youtube.com/channel/UCPa81c4BVXEYXt2EShTzbcg" "TDC YouTube channel")
else (WIN32)
	#set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/docs/img/HELICS.ico")
endif(WIN32)


set(CPACK_SOURCE_IGNORE_FILES  "/Build*/;/build*/;/.git/")

#THIS LINE MUST BE LAST
include(CPack)

ENDIF(ENABLE_PACKAGE_BUILD)
