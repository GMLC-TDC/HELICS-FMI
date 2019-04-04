# Contributors
This file describes the contributors to the HELICS library and the software used as part of this project
If you would like to contribute to the HELICS project see [CONTRIBUTING](CONTRIBUTING.md)
## Individual contributors

### Lawrence Livermore National Lab
 - Ryan Mast*
 - Philip Top*
 - Corey Mcneish*
 - Mert Korkali*
 - Brian Kelley*

### Lawrence Berkeley National Lab
 - Jonathan Coignard
 - Thiery Nuidoi

 `*` currently active
 `**` subcontractor

## Used Libraries or Code

### [BOOST](https://www.boost.org)
  Boost is used throughout the code, The FMI library makes heavy use of boost::DLL and boost::filesystem for loading and interacting with the FMUs

### [helics](https://github.com/GMLC-TDC/HELICS-src)
  The library is based on HELICS and interacts with the libary

### [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
  JsonCPP is used for parsing json files, it was chosen for easy inclusion in the project and its support for comments. Jsoncpp is licensed under public domain or MIT in case public domain is not recognized [LICENSE](https://github.com/open-source-parsers/jsoncpp/blob/master/LICENSE)

### [GridDyn](https://github.com/LLNL/GridDyn)
Much of the library code for FMI's and file interpretors originates in GridDyn, so while GridDyn isn't used directly they share some of the code base

### [fmt](http://fmtlib.net/latest/index.html)
FMT replaces boost::format for internal logging and message printing.  The library is included in the source code.  The CMAKE scripts were modified so they don't trigger a bunch of unnecessary checks and warnings as nearly all checks are already required for building HELICS based on minimum compiler support.  HELICS uses the header only library for the time being.  FMT is licensed under [BSD 2 clause](https://github.com/fmtlib/fmt/blob/master/LICENSE.rst)



### cmake scripts
Several cmake scripts came from other sources and were either used or modified for use in HELICS.
 - Lars Bilke [CodeCoverage.cmake](https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake)
 - NATIONAL HEART, LUNG, AND BLOOD INSTITUTE  FindOctave.cmake
 - clang-format, clang-tidy scripts were created using tips from [Emmanuel Fleury](http://www.labri.fr/perso/fleury/posts/programming/using-clang-tidy-and-clang-format.html)
 - Viktor Kirilov, useful cmake macros [ucm](https://github.com/onqtam/ucm)  particularly for the set_runtime macro to use static runtime libraries
