# HELICS-FMI

[![Build Status](https://dev.azure.com/HELICS-test/HELICS_FMI/_apis/build/status/GMLC-TDC.HELICS-FMI?branchName=main)](https://dev.azure.com/HELICS-test/HELICS_FMI/_build/latest?definitionId=8&branchName=main)
[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/GMLC-TDC/HELICS-FMI/main.svg)](https://results.pre-commit.ci/latest/github/GMLC-TDC/HELICS-FMI/main)

Executable and other tools to allow [Functional Mockup units](https://fmi-standard.org/) to interact with [HELICS](https://github.com/GMLC-TDC/HELICS). Currently FMI 2.0 is supported.

## Building

HELICS-FMI uses cmake for build system generation. It requires HELICS 3.3 or greater to operate.

### Windows

For building with Visual Studio the cmake-gui is recommended.
Set the build directory to an empty path, for example HELICS-FMI/build, and the source directory to the HELICS-FMI directory.
HELICS-FMI will attempt to locate an existing HELICS installation and use those files. If one does not exist HELICS will automatically download and build it.

### Linux and others

The process is the same as for Windows, with the exception that HELICS will not automatically build unless the AUTOBUILD_HELICS option is enabled.

## Executing

`helics-fmi` is the main program it work similarly to other federates in HELICS. +

```sh
$ helics-fmi feedthrough.fmu
```

```sh
$ helics-fmi --autobroker --step=0.1 --stop=4.0 feedthrough.fmu
```

Will start up a broker as well as the fmu. The step and stop time can be specified.
additional broker args can be added through `--brokerargs` option.

## Source Repo

The HELICS-FMI source code is hosted on GitHub: [https://github.com/GMLC-TDC/HELICS-FMI](https://github.com/GMLC-TDC/HELICS-FMI)

## Release

HELICS is distributed under the terms of the BSD-3 clause license. All new
contributions must be made under this license. [LICENSE](LICENSE)

SPDX-License-Identifier: BSD-3-Clause

Portions of the code written by LLNL with release number
LLNL-CODE-780177
