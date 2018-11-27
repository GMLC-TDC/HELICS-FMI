/*
* LLNS Copyright Start
* Copyright (c) 2014-2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#pragma once

#include <stdexcept>

using solver_index_type = int32_t;
#define FUNCTION_EXECUTION_SUCCESS 0

using invalidParameterValue = std::invalid_argument;
using unrecognizedParameter = std::invalid_argument;
constexpr solver_index_type invalidLocation = -245;
