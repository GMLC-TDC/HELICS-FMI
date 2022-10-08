# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

# project name

set(elementReader_files
    tinyxml2ReaderElement.cpp
    tinyxml2ReaderElement.h
    readerElement.h
    readerElement.cpp
    jsonElement.cpp
    jsonElement.h
    jsonReaderElement.cpp
    jsonReaderElement.h
    tomlElement.cpp
    tomlElement.h
    tomlReaderElement.cpp
    tomlReaderElement.h
    TomlProcessingFunctions.cpp
    TomlProcessingFunctions.hpp
    JsonProcessingFunctions.hpp
    JsonProcessingFunctions.cpp
    ${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml2/tinyxml2.cpp
    ${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml2/tinyxml2.h
)

add_library(formatInterpreter STATIC ${elementReader_files})

target_link_libraries(formatInterpreter PUBLIC gmlc_utilities JsonCpp::JsonCpp toml11::toml11 helics_fmi_base)

install(TARGETS formatInterpreter LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

set(formatInterpreter_include_directories ${PROJECT_SOURCE_DIR}/src/formatInterpreters PARENT_SCOPE)
