IF (MSVC)

set (boost_versions
boost_1_66_0
boost_1_65_1
boost_1_65_0
boost_1_64_0
boost_1_63_0
boost_1_62_0
boost_1_61_0
)

set(poss_prefixes
C:
C:/boost
C:/local
C:/boost
C:/Libraries
D:
D:/boost
D:/local
D:/boost
)

# create an empty list
list(APPEND boost_paths "")

foreach( dir ${poss_prefixes})
	foreach( boostver ${boost_versions})
		IF (IS_DIRECTORY ${dir}/${boostver})
			list(APPEND boost_paths ${dir}/${boostver})
		ENDIF()
	endforeach()
endforeach()


find_path(BOOST_TEST_PATH
			NAMES 			boost/version.hpp
			PATHS		${boost_paths}
		)

		if (BOOST_TEST_PATH)
		set(BOOST_ROOT ${BOOST_TEST_PATH})
		endif(BOOST_TEST_PATH)
ENDIF(MSVC)

SHOW_VARIABLE(BOOST_ROOT PATH "Boost root directory" "${BOOST_ROOT}")

# Minimum version of Boost required for building HELICS-fmi
set(BOOST_MINIMUM_VERSION 1.61)
#most others needed are included through HELICS
find_package(Boost ${BOOST_MINIMUM_VERSION} COMPONENTS unit_test_framework REQUIRED)

# Minimum version of Boost required for building test suite
if (Boost_VERSION GREATER 106599)
	#in 1.166 there were some changes to asio and inclusion of beast that will enable other components
	set(BOOST_VERSION_LEVEL 2)
else()
	set(BOOST_VERSION_LEVEL 1)
ENDIF()

#mark_as_advanced(CLEAR BOOST_ROOT)

message(STATUS "Using Boost include files : ${Boost_INCLUDE_DIR}")
#message(STATUS "Using Boost libraries in : ${Boost_LIBRARY_DIRS}")
#message(STATUS "Using Boost libraries : ${Boost_LIBRARIES}")
set(modifier,"")
foreach(loop_var ${Boost_LIBRARIES})
	if (${loop_var} MATCHES "debug")
		list(INSERT modifier 0 ${loop_var})
	elseif(${loop_var} MATCHES "optimized")
		list(INSERT modifier 0 ${loop_var})
	else()
		#message("Boost_LIBRARIES ${loop_var}")
		if(${loop_var} MATCHES "unit_test")
			list(APPEND Boost_LIBRARIES_test ${modifier} ${loop_var})
		else()
			list(APPEND Boost_LIBRARIES_core ${modifier} ${loop_var})
		endif()
		list(LENGTH modifier modifier_size)
		if (modifier_size GREATER 0)
		list(REMOVE_AT modifier -1)
		endif()
	endif()
endforeach(loop_var)

#message(STATUS "Using Boost core libraries : ${Boost_LIBRARIES_core}")
#message(STATUS "Using Boost test libraries : ${Boost_LIBRARIES_test}")
list(APPEND external_library_list ${Boost_LIBRARIES_core})
list(APPEND external_link_directories ${Boost_LIBRARY_DIRS})
