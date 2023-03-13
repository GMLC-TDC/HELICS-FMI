----------------------------
HELICS FMI CMake Reference
----------------------------

There are a few CMake variables that control the build process, they can be altered to change how the HELICS_FMIs library is built and what exactly is built.

CMake variables
----------------

-  `BUILD_TESTING` : Generate CMake Variable controlling whether to build the tests or not
-  `HELICS_FMIS_ENABLE_TESTS` :  Does the same thing as `BUILD_TESTING`
-  `HELICS_FMIS_BUILD_STATIC_LIBRARY`:  Controls whether a static library should be built or not
-  `HELICS_FMIS_BUILD_SHARED_LIBRARY`:  Controls whether to build a shared library or not
-  `HELICS_FMIS_BUILD_FUZZ_TARGETS`:  If set to `ON`, the library will try to compile the fuzzing targets for clang libFuzzer
-  `HELICS_FMIS_BUILD_WEB_SERVER`:  If set to `ON`,  build a webserver,  This uses boost::beast and requires boost 1.70 or greater to build it also requires CMake 3.12 or greater
-  `HELICS_FMIS_BUILD_CONVERTER_APP`: enables building a simple command line converter application that can convert HELICS_FMIs from the command line
-  `HELICS_FMIS_ENABLE_EXTRA_COMPILER_WARNINGS`: Turn on bunch of extra compiler warnings, on by default
-  `HELICS_FMIS_ENABLE_ERROR_ON_WARNINGS`:  Mostly useful in some testing contexts but will turn on `Werror` so any normal warnings generate an error.
-  `CMAKE_CXX_STANDARD`:  Compile with a particular C++ standard, valid values are `11`, `14`, `17`, `20`, and likely `23` though that isn't broadly supported.  Will set to 14 by default if not otherwise specified
-  `HELICS_FMIS_BINARY_ONLY_INSTALL`:  Just install shared libraries and executables,  no headers or static libs or packaging information
-  `HELICS_FMIS_CLANG_TIDY`:  Enable the clang tidy tests as part of the build
-  `HELICS_FMIS_BASE_TYPE`:  Set to `uint64_t` for expanded base-HELICS_FMI power support. This increases the size of a HELICS_FMI by 4 Bytes.
-  `HELICS_FMIS_DOMAIN`:  Specify a default domain to use for string conversions.  Can be either a name from the domains namespace such as `domains::surveying` or one of 'COOKING', 'ASTRONOMY', 'NUCLEAR', 'SURVEYING', 'USE_CUSTOMARY', 'CLIMATE', or 'UCUM'.

-  `HELICS_FMIS_NAMESPACE`:  The top level namespace of the library, defaults to `HELICS_FMIs`.
   When compiling with C++17 (or higher), this can be set to, e.g., `mynamespace::HELICS_FMIs` to avoid name clashes with other libraries defining `HELICS_FMIs`.
-  `HELICS_FMIS_INSTALL`:  This is set to `ON` normally but defaults to `OFF` if used as a subproject.  This controls whether anything gets installed by the install target.

If compiling as part of a subproject then a few other options are useful

-  `HELICS_FMIS_HEADER_ONLY`:  Only generate the header only target, sets `HELICS_FMIS_BUILD_STATIC_LIBRARY` and `HELICS_FMIS_BUILD_SHARED_LIBRARY` to OFF
-  `HELICS_FMIS_INSTALL`:  enable the install instructions of the library
-  `HELICS_FMIS_BUILD_OBJECT_LIBRARY`:  Generate an object library that can be used as part of other builds.  Only one of `HELICS_FMIS_BUILD_SHARED_LIBRARY`, `HELICS_FMIS_BUILD_STATIC_LIBRARY`, or `HELICS_FMIS_BUILD_OBJECT_LIBRARY` can be set to `ON`.  If more than one are set,  the shared library and object library settings take precedence over the static library.
-  `HELICS_FMIS_LIBRARY_EXPORT_COMMAND`:  If desired the targets for the HELICS_FMIs library can be merged into an root project target list by modifying this variable.  The use cases for this are rare, but if this is something you want to do this variable should be set to something like `EXPORT rootProjectTargets`

CMake Targets
--------------

If you are using the library as a submodule or importing the package there are a couple targets that can be used depending on the build

-  `HELICS_FMIs::HELICS_FMIs`  will be set to the library being built, either the shared, static, or object
-  `HELICS_FMIs::header_only` is a target for the headers if `HELICS_FMIS_HEADER_ONLY` CMake variable is set, then only this target is generated.  This target is always created.


Example
---------

As part of the `HELICS <https://github.com/GMLC-TDC/HELICS>`_ library the HELICS_FMIs library is used as a submodule it is included by the following code

.. code-block:: cmake

   # so HELICS_FMIs cpp exports to the correct target export
   set(HELICS_FMIS_INSTALL OFF CACHE INTERNAL "")

   if(NOT CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 17) # Supported values are ``11``, ``14``, and ``17``.
   endif()

   set(HELICS_FMIS_BUILD_OBJECT_LIBRARY OFF CACHE INTERNAL "")
   set(HELICS_FMIS_BUILD_STATIC_LIBRARY ON CACHE INTERNAL "")
   set(HELICS_FMIS_BUILD_SHARED_LIBRARY OFF CACHE INTERNAL "")
   set(HELICS_FMIS_BUILD_CONVERTER_APP OFF CACHE INTERNAL "")
   set(HELICS_FMIS_BUILD_WEBSERVER OFF CACHE INTERNAL "")
   set(HELICS_FMIS_CLANG_TIDY_OPTIONS "" CACHE INTERNAL "")
   set(HELICS_FMIS_BUILD_FUZZ_TARGETS OFF CACHE INTERNAL "")

   add_subdirectory(
        "${PROJECT_SOURCE_DIR}/ThirdParty/HELICS_FMIs" "${PROJECT_BINARY_DIR}/ThirdParty/HELICS_FMIs"
   )

   set_target_properties(HELICS_FMIs PROPERTIES FOLDER Extern)

   hide_variable(HELICS_FMIS_HEADER_ONLY)
   hide_variable(HELICS_FMIS_BUILD_OBJECT_LIBRARY)
   hide_variable(HELICS_FMIS_NAMESPACE)

Then the target linked by

.. code-block:: cmake

   target_link_libraries(helics_common PUBLIC HELICS::utilities HELICS_FMIs::HELICS_FMIs)
