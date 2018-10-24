# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.5)

project(HELICS-FMI VERSION 0.4.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#set (HELICS-FMI_VERSION_BUILD alpha.1)
set (HELICS-FMI_DATE "08-30-18")
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
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/config/cmake/")
include(extraMacros)
include(CMakeDependentOption)

FILE(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/logs)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(WIN32)
    if(MSYS)
      set(
        CMAKE_INSTALL_PREFIX "/usr/local/helics-fmi_${HELICS-FMI_VERSION_UNDERSCORE}/"
        CACHE PATH "default install path"
        FORCE
      )
		else(MSYS)
      set(
        CMAKE_INSTALL_PREFIX "C:/local/helics-fmi_${HELICS-FMI_VERSION_UNDERSCORE}/"
        CACHE PATH "default install path"
        FORCE
      )
		endif(MSYS)
	endif(WIN32)
endif()

show_variable(
  AUTOBUILD_INSTALL_PATH
  PATH
  "location to install the autobuild libraries and Headers"
  "${PROJECT_BINARY_DIR}/libs"
)

mark_as_advanced(AUTOBUILD_INSTALL_PATH)

# Prohibit in-source build
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source build is not supported. Please, use an empty directory for building the project.")
ENDIF()


add_library(helics_fmi_base INTERFACE)

include(compiler_flags)
message(STATUS "setting version build options to \"${VERSION_OPTION}\"")
if(VERSION_OPTION)
  if(MSVC)
		target_compile_options(helics_fmi_base INTERFACE ${VERSION_OPTION})
	else()
    target_compile_options(
      helics_fmi_base
      INTERFACE $<$<COMPILE_LANGUAGE:CXX>:${VERSION_OPTION}>
    )
  endif(MSVC)
endif(VERSION_OPTION)

cmake_dependent_option(
  STATIC_STANDARD_LIB
  "Link against a static standard lib"
  OFF
  "NOT USE_LIBCXX"
  OFF
)
include(ucm)
mark_as_advanced(STATIC_STANDARD_LIB)
if(STATIC_STANDARD_LIB)
	ucm_set_runtime(STATIC)
else()
	ucm_set_runtime(DYNAMIC)
endif()

if(MSVC)
	option(EMBEDDED_DEBUG_INFO "embed debug info into obj files" OFF)
	mark_as_advanced(EMBEDDED_DEBUG_INFO)
  if(EMBEDDED_DEBUG_INFO)
		ucm_set_embedded_debug(EMBEDDED)
	else()
		ucm_set_embedded_debug(EXTERNAL)
	endif()
endif(MSVC)

# -------------------------------------------------------------
# Get some configuration for C++17 as that becomes available
# -------------------------------------------------------------
# message(STATUS ${CMAKE_CXX_FLAGS})
set(CONFIGURE_TARGET_LOCATION ${AUTOBUILD_INSTALL_PATH}/include/helics-fmi/)
include(configGenerator)

#########################################################################################

#include(mergestaticlibs)

include(GNUInstallDirs)

# -------------------------------------------------------------
# BOOST  find the boost libraries
# -------------------------------------------------------------
set(
  BOOST_REQUIRED_LIBRARIES
  program_options
  unit_test_framework
  filesystem
  system
  date_time
  timer
  chrono
)
include(addBoost)



target_link_libraries(helics_fmi_base INTERFACE Boostlibs::core)

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


# -------------------------------------------------------------
# Enable HELICS executable
# -------------------------------------------------------------

SHOW_VARIABLE(HELICS_INSTALL_PATH PATH "path to the helics installation" "${PROJECT_BINARY_DIR}/libs")

include(addHELICS)


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
	"https://gmlc-tdc.github.io/HELICS-src/" "Helics Documentation"
	"https://www.helics-fmi.org" "Helics FMI Web page"
	"https://www.youtube.com/channel/UCPa81c4BVXEYXt2EShTzbcg" "TDC YouTube channel")
else (WIN32)
	#set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/docs/img/HELICS.ico")
endif(WIN32)


set(CPACK_SOURCE_IGNORE_FILES  "/Build*/;/build*/;/.git/")

#THIS LINE MUST BE LAST
include(CPack)

ENDIF(ENABLE_PACKAGE_BUILD)