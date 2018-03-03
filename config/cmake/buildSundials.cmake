# This function is used to force a build on a Dependant project at cmake configuration phase.
# 

function (build_sundials)

    set(trigger_build_dir ${CMAKE_BINARY_DIR}/autobuild/force_sundials)

    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)
#find the directory where KLU is located
    
    #generate false dependency project
    IF (UNIX)
    set(EXTRA_C_FLAGS "-fpic")
    ENDIF(UNIX)
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.4)
    include(ExternalProject)
ExternalProject_Add(sundials
    URL https://computation.llnl.gov/projects/sundials/download/sundials-3.1.0.tar.gz
    UPDATE_COMMAND " " 
    BINARY_DIR ${PROJECT_BINARY_DIR}/ThirdParty/sundials
     
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/libs
        -DCMAKE_BUILD_TYPE=\$\{CMAKE_BUILD_TYPE\}
        -DBUILD_CVODES=OFF
        -DBUILD_IDAS=OFF
		-DBUILD_IDA=OFF
		-DBUILD_KINSOL=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DEXAMPLES_ENABLE_C=OFF
		-DEXAMPLES_ENABLE_CXX=OFF
        -DEXAMPLES_INSTALL=OFF
		-DSUNDIALS_INDEX_TYPE=int32_t
        -DKLU_ENABLE=OFF
        -DCMAKE_C_FLAGS=${EXTRA_C_FLAGS}
        -DOPENMP_ENABLE=${OPENMP_FOUND}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_LINKER=${CMAKE_LINKER}
		-DCMAKE_DEBUG_POSTFIX=d
        
        
    INSTALL_DIR ${PROJECT_BINARY_DIR}/libs
    )")


    file(WRITE ${trigger_build_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")
if (NOT BUILD_RELEASE_ONLY)
	message(STATUS "Configuring Sundials Autobuild for debug: logging to ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_config_debug.log")
    execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -D CMAKE_C_COMPILER=${CMAKE_C_COMPILER} -D CMAKE_LINKER=${CMAKE_LINKER}
        -D CMAKE_BUILD_TYPE=Debug -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_config_debug.log
        )
	
	message(STATUS "Building Sundials Autobuild for debug: logging to ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_build_debug.log")	
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_build_debug.log
        )
	
	endif()
	
	message(STATUS "Configuring Sundials Autobuild for release: logging to ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_config_release.log")	
	execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -D CMAKE_C_COMPILER=${CMAKE_C_COMPILER} -D CMAKE_LINKER=${CMAKE_LINKER}
        -D CMAKE_BUILD_TYPE=Release -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_config_release.log
        )
		
	message(STATUS "Building Sundials Autobuild for release: logging to ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_build_release.log")
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_build_release.log
        )

endfunction()