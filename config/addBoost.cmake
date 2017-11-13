# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

IF (MSVC)

set( boost_paths
C:/boost_1_65_1
 C:/boost_1_64_0
C:/boost_1_63_0
C:/boost_1_61_0
)
IF (IS_DIRECTORY C:/boost)
list(APPEND boost_paths
C:/boost/boost_1_65_1
 C:/boost/boost_1_64_0
C:/boost/boost_1_63_0
C:/boost/boost_1_61_0
)
ENDIF()

IF (IS_DIRECTORY C:/local)
list(APPEND boost_paths
C:/local/boost_1_65_1
 C:/local/boost_1_64_0
C:/local/boost_1_63_0
C:/local/boost_1_61_0
)
ENDIF()


IF (EXISTS D:/)
list(APPEND boost_paths
D:/boost_1_65_1
 D:/boost_1_64_0
D:/boost_1_63_0
D:/boost_1_61_0
)
IF (IS_DIRECTORY D:/boost)
list(APPEND boost_paths
D:/boost/boost_1_65_1
 D:/boost/boost_1_64_0
D:/boost/boost_1_63_0
D:/boost/boost_1_61_0
)
ENDIF()

IF (IS_DIRECTORY D:/local)
list(APPEND boost_paths
D:/local/boost_1_65_1
 D:/local/boost_1_64_0
D:/local/boost_1_63_0
D:/local/boost_1_61_0
)
ENDIF()

ENDIF()

 message(STATUS ${boost_paths})

find_path(BOOST_TEST_PATH
			NAMES 			boost/version.hpp
			PATHS		${boost_paths}
		)

		if (BOOST_TEST_PATH)
		set(BOOST_ROOT ${BOOST_TEST_PATH})
		endif(BOOST_TEST_PATH)
ENDIF(MSVC)

SHOW_VARIABLE(BOOST_ROOT PATH "Boost root directory" "${BOOST_ROOT}")

# Minimum version of Boost required for building HELICS
set(BOOST_MINIMUM_VERSION 1.61)
 find_package(Boost ${BOOST_MINIMUM_VERSION} COMPONENTS program_options filesystem system date_time REQUIRED)

mark_as_advanced(CLEAR BOOST_ROOT)

message(STATUS "Using Boost include files : ${Boost_INCLUDE_DIR}")
message(STATUS "Using Boost libraries in : ${Boost_LIBRARY_DIRS}")
message(STATUS "Using Boost libraries : ${Boost_LIBRARIES}")

list(APPEND external_library_list ${Boost_LIBRARIES})
list(APPEND external_link_directories ${Boost_LIBRARY_DIRS})