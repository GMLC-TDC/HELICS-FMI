# HELICS-FMI

[![Build status](https://ci.appveyor.com/api/projects/status/8pt3yp3tveflmj3s/branch/main?svg=true)](https://ci.appveyor.com/project/phlptp/helics-fmi/branch/main)
[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/GMLC-TDC/HELICS-FMI/main.svg)](https://results.pre-commit.ci/latest/github/GMLC-TDC/HELICS-FMI/main)

Executable to allow Functional Mockup units to interact with [HELICS](https://github.com/GMLC-TDC/HELICS)

## Building

HELICS-FMI uses cmake for build system generation. It requires HELICS 3.3 or greater to operate.  

### Windows

For building with Visual Studio the cmake-gui is recommended.
Set the build directory to an empty path, for example HELICS-FMI/build, and the source directory to the HELICS-FMI directory.
HELICS-FMI will attempt to locate an existing HELICS installation and use those files. If one does not exist HELICS will automatically download and build it.

### Linux and others

The process is the same as for Windows, with the exception that HELICS will not automatically build unless the AUTOBUILD_HELICS option is enabled.

## Source Repo

The HELICS-FMI source code is hosted on GitHub: [https://github.com/GMLC-TDC/HELICS-FMI](https://github.com/GMLC-TDC/HELICS-FMI)

## Release

HELICS is distributed under the terms of the BSD-3 clause license. All new
contributions must be made under this license. [LICENSE](LICENSE)

SPDX-License-Identifier: BSD-3-Clause

Portions of the code written by LLNL with release number
LLNL-CODE-780177
