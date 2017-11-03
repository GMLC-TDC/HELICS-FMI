# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End



#check for clang 3.4 and the fact that CMAKE_CXX_STANDARD doesn't work yet for that compiler

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5)
    set(CMAKE_REQUIRED_FLAGS -std=c++1y)
	set(VERSION_OPTION -std=c++1y)
  else ()
    set(CMAKE_REQUIRED_FLAGS -std=c++1z)
    set(VERSION_OPTION -std=c++1z)
  endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  # c++14 becomes default in GCC 6.1
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.1)
    set(CMAKE_REQUIRED_FLAGS -std=c++1y)
    set(VERSION_OPTION -std=c++1y)
  else ()
    set(CMAKE_REQUIRED_FLAGS -std=c++1z)
    set(VERSION_OPTION -std=c++1z)
  endif()
endif()

include(CheckIncludeFileCXX)
check_include_file_cxx("optional" HAVE_OPTIONAL)
check_include_file_cxx("experimental/optional" HAVE_EXP_OPTIONAL)
check_include_file_cxx("variant" HAVE_VARIANT)

include(CheckCXXFeatureTestingMacro)
check_cxx_feature_testing_macro(variable_templates "" HAVE_VARIABLE_TEMPLATES)
check_cxx_feature_testing_macro(if_constexpr "" HAVE_CONSTEXPR_IF)
check_cxx_feature_testing_macro(lib_clamp "algorithm" HAVE_CLAMP)
check_cxx_feature_testing_macro(lib_hypot "cmath" HAVE_HYPOT3)

check_cxx_feature_testing_macro(lib_string_view "string_view" HAVE_STRING_VIEW 201603)
check_cxx_feature_testing_macro(lib_experimental_string_view "experimental/string_view" HAVE_EXP_STRING_VIEW 201411)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.0)
	set(HAVE_FALLTHROUGH ON)
	set(HAVE_UNUSED ON)
	message(STATUS "gt 4.0 ${CMAKE_CXX_COMPILER_VERSION}")
  endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
	set(HAVE_FALLTHROUGH ON)
	set(HAVE_UNUSED ON)
  endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
#intel doesn't really have anything beyond the minimum
  if (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 17.0)
  
  endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  message(STATUS "win_comp_version ${CMAKE_CXX_COMPILER_VERSION}")
  set(HAVE_VARIABLE_TEMPLATES ON)
endif()

if (NOT NO_CONFIG_GENERATION)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/libs/include/config.h)
endif(NOT NO_CONFIG_GENERATION)
