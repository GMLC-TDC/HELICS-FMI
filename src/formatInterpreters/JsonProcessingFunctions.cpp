/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "JsonProcessingFunctions.hpp"

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

namespace helicsfmi::fileops {

bool hasJsonExtension(const std::string& jsonString)
{
    auto ext = jsonString.substr(jsonString.length() - 4);
    return ((ext == "json") || (ext == "JSON") || (ext == ".jsn") || (ext == ".JSN"));
}

Json::Value loadJson(const std::string& jsonString)
{
    if (jsonString.size() > 128) {
        try {
            return loadJsonStr(jsonString);
        }
        catch (const std::invalid_argument&) {
            // this was a guess lets try a file now, the same error will be generated again later as
            // well
        }
    }
    std::ifstream file(jsonString);

    if (file.is_open()) {
        Json::Value doc;
        Json::CharReaderBuilder rbuilder;
        std::string errs;
        bool ok = Json::parseFromStream(rbuilder, file, &doc, &errs);
        if (!ok) {
            throw(std::invalid_argument(errs.c_str()));
        }
        return doc;
    }
    return loadJsonStr(jsonString);
}

Json::Value loadJsonStr(std::string_view jsonString)
{
    Json::Value doc;
    Json::CharReaderBuilder rbuilder;
    std::string errs;
    auto reader = std::unique_ptr<Json::CharReader>(rbuilder.newCharReader());
    bool ok = reader->parse(jsonString.data(), jsonString.data() + jsonString.size(), &doc, &errs);
    if (!ok) {
        throw(std::invalid_argument(errs.c_str()));
    }
    return doc;
}

std::string getName(const Json::Value& element)
{
    return (element.isMember("key")) ?
        element["key"].asString() :
        ((element.isMember("name")) ? element["name"].asString() : std::string());
}

std::string generateJsonString(const Json::Value& block)
{
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    builder["commentStyle"] = "None";
    builder["indentation"] = "   ";  // or whatever you like
    builder["precision"] = 17;
    auto writer = std::unique_ptr<Json::StreamWriter>(builder.newStreamWriter());
    std::stringstream sstr;
    writer->write(block, &sstr);
    auto ret = sstr.str();
    return ret;
}

}  // namespace helics::fileops
