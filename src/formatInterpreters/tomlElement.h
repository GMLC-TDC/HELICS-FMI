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
#ifdef __GNUC__
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif
#include <string_view>
#define TOML11_USING_STRING_VIEW 1
#include "toml.hpp"
#ifdef __GNUC__
#    pragma GCC diagnostic pop
#endif

#include <string>
#include <utility>

class tomlElement {
  public:
    int elementIndex = 0;
    std::string name;
    int arrayIndex = 0;
    tomlElement() = default;
    tomlElement(toml::value vElement, std::string newName);

    void clear();
    const toml::value& getElement() const { return (arraytype) ? element[arrayIndex] : element; }
    int count() const { return (arraytype) ? static_cast<int>(element.size()) : 1; }
    bool isNull() const
    {
        return (arraytype) ? arrayIndex>=static_cast<int>(element.size()) || element[arrayIndex].is_uninitialized() : element.is_uninitialized();
    }

  private:
    toml::value element;
    bool arraytype = false;
};
