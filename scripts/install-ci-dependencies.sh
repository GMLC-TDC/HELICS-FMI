#!/bin/bash

# Set variables based on build environment
if [[ "$TRAVIS" == "true" ]]; then
    if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
        HOMEBREW_NO_AUTO_UPDATE=1 brew install pcre
        HOMEBREW_NO_AUTO_UPDATE=1 brew install ccache
        export PATH="/usr/local/opt/ccache/libexec:$PATH"
    fi

    export CI_DEPENDENCY_DIR=${TRAVIS_BUILD_DIR}/dependencies

    WAIT_COMMAND=travis_wait

    # Convert commit message to lower case
    commit_msg=$(tr '[:upper:]' '[:lower:]' <<< ${TRAVIS_COMMIT_MESSAGE})

    if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
        os_name="Linux"
    elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
        os_name="Darwin"
    fi
else
    export CI_DEPENDENCY_DIR=$1
    commit_msg=""
    os_name="$(uname -s)"
fi

shared_lib_ext=so
if [[ "$os_name" == "Darwin" ]]; then
    shared_lib_ext=dylib
fi

boost_version=$CI_BOOST_VERSION
if [[ -z "$CI_BOOST_VERSION" ]]; then
    boost_version=1.65.0
fi
boost_install_path=${CI_DEPENDENCY_DIR}/boost

# Wipe out cached dependencies if commit message has '[update_cache]'
if [[ $commit_msg == *'[update_cache]'* ]]; then
    individual="false"
    if [[ $commit_msg == *'boost'* ]]; then
        rm -rf ${boost_install_path};
        individual="true"
    fi

    # If no dependency named in commit message, update entire cache
    if [[ "$individual" != 'true' ]]; then
        rm -rf ${CI_DEPENDENCY_DIR};
    fi
fi

if [[ ! -d "${CI_DEPENDENCY_DIR}" ]]; then
    mkdir -p ${CI_DEPENDENCY_DIR};
fi

# Install Boost
if [[ ! -d "${boost_install_path}" ]]; then
    echo "*** build boost"
    local boost_sanitizer=""
    if [[ "$RUN_TSAN" ]]; then
        boost_sanitizer="BOOST_SANITIZER=thread"
    fi
    ${BOOST_SANITIZER} ${WAIT_COMMAND} ./scripts/install-dependency.sh boost ${boost_version} ${boost_install_path}
    echo "*** built boost successfully"
fi

# Export variables and set load library paths

if [[ "$os_name" == "Linux" ]]; then
    export LD_LIBRARY_PATH=${boost_install_path}/lib:$LD_LIBRARY_PATH
elif [[ "$os_name" == "Darwin" ]]; then
    export DYLD_LIBRARY_PATH=${boost_install_path}/lib:$DYLD_LIBRARY_PATH
fi

if [[ "$os_name" == "Linux" ]]; then
    export LD_LIBRARY_PATH=${PWD}/build/src/helics/shared_api_library/:$LD_LIBRARY_PATH
elif [[ "$os_name" == "Darwin" ]]; then
    export DYLD_LIBRARY_PATH=${PWD}/build/src/helics/shared_api_library/:$DYLD_LIBRARY_PATH
fi
