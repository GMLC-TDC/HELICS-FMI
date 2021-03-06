
set(BUILD_CVODES OFF CACHE INTERNAL "")
set(BUILD_IDAS OFF CACHE INTERNAL "")
set(BUILD_IDA OFF CACHE INTERNAL "")
set(BUILD_KINSOL OFF CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(EXAMPLES_ENABLE_C OFF CACHE INTERNAL "")
set(EXAMPLES_ENABLE_CXX OFF CACHE INTERNAL "")
set(EXAMPLES_INSTALL OFF CACHE INTERNAL "")
set(SUNDIALS_INDEX_SIZE 32 CACHE INTERNAL "")
		

  if (ENABLE_KLU)
	set(ENABLE_KLU ON INTERNAL "")
  endif(ENABLE_KLU)

add_subdirectory(extern/sundials)

add_library(sundials_all INTERFACE)
target_include_directories(sundials_all INTERFACE extern/sundials/include)
target_include_directories(sundials_all INTERFACE ${CMAKE_BINARY_DIR}/extern/sundials/include)
add_library(SUNDIALS::SUNDIALS ALIAS sundials_all) 

set(SUNDIALS_LIBRARIES
	sundials_arkode_static
	sundials_cvode_static
	sundials_nvecserial_static
	sundials_sunlinsolband_static
	sundials_sunlinsoldense_static
	sundials_sunlinsolpcg_static
	sundials_sunlinsolspbcgs_static
	sundials_sunlinsolspfgmr_static
	sundials_sunlinsolspgmr_static
	sundials_sunlinsolsptfqmr_static
	sundials_sunmatrixband_static
	sundials_sunmatrixdense_static
	sundials_sunmatrixsparse_static
	sundials_sunnonlinsolfixedpoint_static
	sundials_sunnonlinsolnewton_static
)
set_target_properties ( ${SUNDIALS_LIBRARIES} PROPERTIES FOLDER sundials)

if (MSVC)
target_compile_options(sundials_cvode_static PRIVATE "/sdl-")
target_compile_options(sundials_cvode_static PRIVATE "/W3")
endif()