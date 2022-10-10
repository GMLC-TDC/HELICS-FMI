set(BUILD_CVODES OFF CACHE INTERNAL "")
set(BUILD_IDAS OFF CACHE INTERNAL "")
set(BUILD_IDA OFF CACHE INTERNAL "")
set(BUILD_KINSOL OFF CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")
set(EXAMPLES_ENABLE_C OFF CACHE INTERNAL "")
set(EXAMPLES_ENABLE_CXX OFF CACHE INTERNAL "")
set(EXAMPLES_INSTALL OFF CACHE INTERNAL "")
set(SUNDIALS_INDEX_SIZE 32 CACHE INTERNAL "")

if(ENABLE_KLU)
    set(ENABLE_KLU ON INTERNAL "")
endif(ENABLE_KLU)

add_subdirectory(ThirdParty/sundials)

add_library(sundials_headers INTERFACE)
target_include_directories(sundials_headers INTERFACE ThirdParty/sundials/include)
target_include_directories(
    sundials_headers INTERFACE ${CMAKE_BINARY_DIR}/ThirdParty/sundials/include
)
add_library(HELICS_FMI::sundials_headers ALIAS sundials_headers)

set(SUNDIALS_LIBRARIES
    sundials_arkode_static
    sundials_cvode_static
    sundials_arkode_obj_static
    sundials_cvode_obj_static
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
    sundials_generic_static
    sundials_generic_obj_static
    sundials_nvecmanyvector_obj_static
    sundials_nvecmanyvector_static
    sundials_nvecserial_obj_static
    sundials_sunlinsolband_obj_static
    sundials_sunlinsoldense_obj_static
    sundials_sunlinsolpcg_obj_static
    sundials_sunlinsolspbcgs_obj_static
    sundials_sunlinsolspfgmr_obj_static
    sundials_sunlinsolspgmr_obj_static
    sundials_sunlinsolsptfqmr_obj_static
    sundials_sunmatrixband_obj_static
    sundials_sunmatrixdense_obj_static
    sundials_sunmatrixsparse_obj_static
    sundials_sunmemsys_obj_static
    sundials_sunnonlinsolfixedpoint_obj_static
    sundials_sunnonlinsolnewton_obj_static
)
set_target_properties(${SUNDIALS_LIBRARIES} PROPERTIES FOLDER sundials)

if(MSVC)
    target_compile_options(sundials_cvode_static PRIVATE "/sdl-")
    target_compile_options(sundials_cvode_static PRIVATE "/W3")
endif()

get_target_property(cvode_target_debug SUNDIALS::cvode OUTPUT_NAME)
get_target_property(cvode_dir_debug SUNDIALS::cvode LIBRARY_OUTPUT_DIRECTORY)

get_target_property(cvode_target_release SUNDIALS::cvode LIBRARY_OUTPUT_NAME_RELEASE)
get_target_property(cvode_dir_release SUNDIALS::cvode LIBRARY_OUTPUT_DIRECTORY_RELEASE)

add_library(sundials_import_cvode STATIC IMPORTED)
message(STATUS "linking to ${cvode_dir_debug}/${cvode_target_debug}${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_property(
    TARGET sundials_import_cvode
    PROPERTY IMPORTED_LOCATION
             "${cvode_dir_debug}/${cvode_target_debug}${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
# set_property(TARGET sundials_import_cvode PROPERTY IMPORTED_LOCATION_RELEASE
# "${cvode_dir_release}/${cvode_target_release}${CMAKE_STATIC_LIBRARY_SUFFIX}")

add_library(HELICS_FMI::cvode ALIAS sundials_import_cvode)
