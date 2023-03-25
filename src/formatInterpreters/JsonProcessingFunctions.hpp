/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
@details functions related to loading and evaluating JSON files and helper functions for reading
them using the jsoncpp library
*/

#include "json/json.h"
#include <functional>
#include <string>
namespace helicsfmi::fileops {

/** check if the file has a valid JSON extension*/
bool hasJsonExtension(const std::string& jsonString);

/** load a JSON string or filename that points to a JSON file and return a
JSON::Value to the root object
*/
Json::Value loadJson(const std::string& jsonString);

/** load a JSON object in a string
 */
Json::Value loadJsonStr(std::string_view jsonString);

/** get a name or key from the element*/
std::string getName(const Json::Value& element);

/** generate a Json String*/
std::string generateJsonString(const Json::Value& block);

inline std::string JsonAsString(const Json::Value& element)
{
    return (element.isString()) ? element.asString() : generateJsonString(element);
}

inline std::string
    getOrDefault(const Json::Value& element, const std::string& key, std::string_view defVal)
{
    return (element.isMember(key)) ? JsonAsString(element[key]) : std::string(defVal);
}

inline double getOrDefault(const Json::Value& element, const std::string& key, double defVal)
{
    return (element.isMember(key)) ? element[key].asDouble() : defVal;
}

inline bool getOrDefault(const Json::Value& element, const std::string& key, bool defVal)
{
    return (element.isMember(key)) ? element[key].asBool() : defVal;
}

inline int64_t getOrDefault(const Json::Value& element, const std::string& key, int64_t defVal)
{
    return (element.isMember(key)) ? element[key].asInt64() : defVal;
}

inline bool callIfMember(const Json::Value& element,
                         const std::string& key,
                         const std::function<void(const std::string&, bool)>& call)
{
    if (element.isMember(key)) {
        call(key, element[key].asBool());
        return true;
    }
    return false;
}

inline bool callIfMember(const Json::Value& element,
                         const std::string& key,
                         const std::function<void(const std::string&, int)>& call)
{
    if (element.isMember(key)) {
        call(key, element[key].asInt());
        return true;
    }
    return false;
}

inline bool callIfMember(const Json::Value& element,
                         const std::string& key,
                         const std::function<void(const std::string&)>& call)
{
    if (element.isMember(key)) {
        call(element[key].asString());
        return true;
    }
    return false;
}

inline void replaceIfMember(const Json::Value& element, const std::string& key, std::string& sval)
{
    if (element.isMember(key)) {
        sval = element[key].asString();
    }
}

inline void replaceIfMember(const Json::Value& element, const std::string& key, bool& bval)
{
    if (element.isMember(key)) {
        bval = element[key].asBool();
    }
}

inline void replaceIfMember(const Json::Value& element, const std::string& key, int& sval)
{
    if (element.isMember(key)) {
        sval = element[key].asInt();
    }
}

inline void replaceIfMember(const Json::Value& element, const std::string& key, double& sval)
{
    if (element.isMember(key)) {
        sval = element[key].asDouble();
    }
}

}  // namespace helicsfmi::fileops
