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

#include "tomlElement.h"

static const std::string nullStr{};

tomlElement::tomlElement(toml::value vElement, std::string newName):
    name(std::move(newName)), element(std::move(vElement))
{
    elementIndex = 0;

    if (element.is_array()) {
        arraytype = true;
        arrayIndex = 0;
        while ((arrayIndex < static_cast<int>(element.size())) &&
               (!element[arrayIndex].is_uninitialized())) {
            ++arrayIndex;
        }
    }
}

void tomlElement::clear()
{
    element = toml::value();
    elementIndex = 0;
    arrayIndex = 0;
    arraytype = false;
    name = nullStr;
}
