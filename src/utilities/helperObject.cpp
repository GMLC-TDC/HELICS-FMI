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

#include "helperObject.h"

#include "gmlc/libguarded/guarded.hpp"
#include "gmlc/utilities/stringOps.h"
#include "gmlc/utilities/string_viewOps.h"

#include <unordered_map>
#include <utility>

namespace griddyn {
// start at 100 since there are some objects that use low numbers as a check for interface number
// and the id as secondary
std::atomic<std::uint64_t> helperObject::s_obcnt(101);

helperObject::helperObject() noexcept: m_oid(s_obcnt++) {}
helperObject::~helperObject() = default;

helperObject::helperObject(std::string objectName): m_oid(s_obcnt++), um_name(std::move(objectName))
{
}
static gmlc::libguarded::guarded<std::unordered_map<std::uint64_t, std::string>>
    descriptionDictionary;

void helperObject::set(const std::string& param, const std::string& val)
{
    if ((param == "name") || (param == "id")) {
        setName(val);
    } else if (param == "description") {
        setDescription(val);
    } else if ((param == "flags") || (param == "flag")) {
        setMultipleFlags(this, val);
    } else {
        throw(unrecognizedParameter(param));
    }
}

void helperObject::set(const std::string& param, double val)
{
    setFlag(param, (val > 0.1));
}
void helperObject::setDescription(const std::string& description)
{
    descriptionDictionary.lock()->emplace(m_oid, description);
}

std::string helperObject::getDescription() const
{
    auto lk = descriptionDictionary.lock();
    auto res = lk->find(m_oid);
    if (res != lk->end()) {
        return res->second;
    }
    return std::string{};
}
void helperObject::setFlag(const std::string& flag, bool /*val*/)
{
    throw(unrecognizedParameter(flag));
}
bool helperObject::getFlag(const std::string& flag) const
{
    throw(unrecognizedParameter(flag));
}
double helperObject::get(const std::string& param) const
{
    return getFlag(param) ? 1.0 : 0.0;
}
void helperObject::nameUpdate() {}
void helperObject::makeNewOID()
{
    m_oid = ++s_obcnt;
}
coreObject* helperObject::getOwner() const
{
    return nullptr;
}

void setMultipleFlags(helperObject* obj, const std::string& flags)
{
    auto lcflags = gmlc::utilities::convertToLowerCase(flags);
    auto flgs = gmlc::utilities::string_viewOps::split(lcflags);
    gmlc::utilities::string_viewOps::trim(flgs);
    for (const auto& flag : flgs) {
        if (flag.empty()) {
            continue;
        }
        if (flag.front() != '-') {
            obj->setFlag(std::string(flag), true);
        } else {
            obj->setFlag(std::string(flag.substr(1, std::string_view::npos)), false);
        }
    }
}

}  // namespace griddyn
