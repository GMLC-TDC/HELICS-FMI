# This function is used to force a build on a dependant project at cmake configuration phase.
# 

function (build_helics)

	include(escape_string)
	
	escape_string(cxx_compiler_string ${CMAKE_CXX_COMPILER})
	escape_string(c_compiler_string ${CMAKE_C_COMPILER})
	escape_string(linker_string ${CMAKE_LINKER})
	
	set(extra_cxx_flags ${CMAKE_CXX_FLAGS})
	
	if (VERSION_OPTION)
		set(extra_cxx_flags "${extra_cxx_flags} ${VERSION_OPTION}")
	endif()
	
	#message(STATUS "${CMAKE_CXX_COMPILER} to ${compiler_string}")
	
	escape_string(binary_dir_string ${CMAKE_BINARY_DIR})
    set(trigger_build_dir ${binary_dir_string}/autobuild/force_helics)

    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)

    #generate false dependency project
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.4)
    include(ExternalProject)
ExternalProject_Add(helics
    SOURCE_DIR ${binary_dir_string}/Download/helics
    GIT_REPOSITORY  https://github.com/GMLC-TDC/HELICS-src.git
    DOWNLOAD_COMMAND " " 
    UPDATE_COMMAND " " 
    BINARY_DIR ${binary_dir_string}/ThirdParty/helics
     
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${binary_dir_string}/libs
        -DCMAKE_BUILD_TYPE=\$\{CMAKE_BUILD_TYPE\}
		-DBOOST_TEST_PATH=${BOOST_ROOT}
		-DBUILD_HELICS_TESTS=OFF
		-DBUILD_HELICS_EXAMPLES=OFF
		-DBUILD_C_SHARED_LIB=OFF
		-DCMAKE_DEBUG_POSTFIX=d
		-DBUILD_PYTHON=OFF
		-DGENERATE_PYTHON=OFF
        -DCMAKE_CXX_COMPILER=${cxx_compiler_string}
        -DCMAKE_C_COMPILER=${c_compiler_string}
		\"-DCMAKE_CXX_FLAGS=${extra_cxx_flags}\"
		\"-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}\"
		-DZeroMQ_ENABLE=ON
        -DCMAKE_LINKER=${linker_string}
        
    INSTALL_DIR ${binary_dir_string}/libs
    )")


 file(WRITE ${trigger_build_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")
 
if (NOT BUILD_RELEASE_ONLY)
	
	message(STATUS "Configuring HELICS Autobuild for debug logging to ${PROJECT_BINARY_DIR}/logs/helics_autobuild_config_debug.log")
	execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${cxx_compilier_string} -D CMAKE_C_COMPILER=${c_compiler_string} -D CMAKE_LINKER=${linker_string}
         -D CMAKE_BUILD_TYPE=Debug -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/helics_autobuild_config_debug.log
        )
		
	message(STATUS "Building HELICS debug build logging to ${PROJECT_BINARY_DIR}/logs/helics_autobuild_build_debug.log")
	 execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/helics_autobuild_build_debug.log
        )

  endif()
  
  message(STATUS "Configuring HELICS Autobuild for release logging to ${PROJECT_BINARY_DIR}/logs/helics_autobuild_config_release.log")
    execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${cxx_compilier_string} -D CMAKE_C_COMPILER=${c_compiler_string} -D CMAKE_LINKER=${linker_string}
         -D CMAKE_BUILD_TYPE=Release -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/helics_autobuild_config_release.log
        )
	
	message(STATUS "Building HELICS release build logging to ${PROJECT_BINARY_DIR}/logs/helics_autobuild_build_release.log")
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/helics_autobuild_build_release.log
        )
	
	endfunction()
