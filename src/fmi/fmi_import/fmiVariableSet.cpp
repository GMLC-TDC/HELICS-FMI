/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "fmiInfo.h"

#include <algorithm>

FmiVariableSet::FmiVariableSet() = default;

FmiVariableSet::FmiVariableSet(fmi2ValueReference newvr): vrset({newvr}) {}
FmiVariableSet::FmiVariableSet(const FmiVariableSet& vset) = default;

FmiVariableSet::FmiVariableSet(FmiVariableSet&& vset) = default;

FmiVariableSet& FmiVariableSet::operator=(const FmiVariableSet& other) = default;

FmiVariableSet& FmiVariableSet::operator=(FmiVariableSet&& other) = default;

const fmi2ValueReference* FmiVariableSet::getValueRef() const
{
    return vrset.data();
}

size_t FmiVariableSet::getVRcount() const
{
    return vrset.size();
}

fmi_variable_type FmiVariableSet::getType() const
{
    return type;
}

void FmiVariableSet::push(fmi2ValueReference newvr)
{
    vrset.push_back(newvr);
}

void FmiVariableSet::push(const FmiVariableSet& vset)
{
    vrset.reserve(vset.vrset.size() + vrset.size());
    vrset.insert(vrset.end(), vset.vrset.begin(), vset.vrset.end());
}

void FmiVariableSet::reserve(size_t newSize)
{
    vrset.reserve(newSize);
}

void FmiVariableSet::clear()
{
    vrset.clear();
}

void FmiVariableSet::remove(fmi2ValueReference rmvr)
{
    auto rm = std::remove(vrset.begin(), vrset.end(), rmvr);
    vrset.erase(rm, vrset.end());
}
