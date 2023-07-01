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

#include "tomlReaderElement.h"

#include "TomlProcessingFunctions.hpp"
#include "gmlc/utilities/stringConversion.h"
#include "tomlElement.h"

#include <cassert>
#include <fstream>
#include <iostream>

static const std::string nullStr{};

bool isElement(const toml::value& testValue);
bool isAttribute(const toml::value& testValue);
tomlReaderElement::tomlReaderElement() = default;
tomlReaderElement::tomlReaderElement(const std::string& fileName)
{
    tomlReaderElement::loadFile(fileName);
}
void tomlReaderElement::clear()
{
    parents.clear();
    if (current) {
        current->clear();
    }
}

bool tomlReaderElement::isValid() const
{
    return ((current) && (!current->isNull()));
}
bool tomlReaderElement::isDocument() const
{
    if (parents.empty()) {
        if (doc) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<readerElement> tomlReaderElement::clone() const
{
    auto ret = std::make_shared<tomlReaderElement>();
    ret->parents.reserve(parents.size());
    for (const auto& parent : parents) {
        ret->parents.push_back(std::make_shared<tomlElement>(*parent));
    }
    ret->current = std::make_shared<tomlElement>(*current);
    ret->doc = doc;
    return ret;
}

bool tomlReaderElement::loadFile(const std::string& fileName)
{
    try {
        auto vres = helicsfmi::fileops::loadToml(fileName);
        doc = std::make_shared<tomlElement>(vres, fileName);
        clear();
        current = doc;
        return true;
    }
    catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

bool tomlReaderElement::parse(const std::string& inputString)
{
    try {
        auto vres = helicsfmi::fileops::loadTomlStr(inputString);
        doc = std::make_shared<tomlElement>(vres, inputString);
        clear();
        current = doc;
        return true;
    }
    catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

std::string tomlReaderElement::getName() const
{
    return current->name;
}
double tomlReaderElement::getValue() const
{
    if (!isValid()) {
        return readerNullVal;
    }

    if (current->getElement().is_floating()) {
        return current->getElement().as_floating();
    }
    if (current->getElement().is_string()) {
        return gmlc::utilities::numeric_conversionComplete(current->getElement().as_string(),
                                                           readerNullVal);
    }
    return readerNullVal;
}

std::string tomlReaderElement::getText() const
{
    std::string text;
    if (isValid() && current->getElement().is_string()) {
        text = current->getElement().as_string();
    }
    return text;
}

std::string tomlReaderElement::getMultiText(const std::string& /*sep*/) const
{
    std::string text;

    if (isValid() && current->getElement().is_string()) {
        text = current->getElement().as_string();
    }
    return text;
}
static const toml::value uval;

// no attributes in toml
bool tomlReaderElement::hasAttribute(const std::string& attributeName) const
{
    if (!isValid()) {
        return false;
    }

    auto val = toml::find_or(current->getElement(), attributeName, uval);
    if (val.is_uninitialized()) {
        return false;
    }
    return (!(val.is_array() || val.is_table()));
}

bool tomlReaderElement::hasElement(const std::string& elementName) const
{
    auto val = toml::find_or(current->getElement(), elementName, uval);
    if (val.is_uninitialized()) {
        return false;
    }
    return (val.is_array() || val.is_table());
}

static const readerAttribute emptyAttribute{};

readerAttribute tomlReaderElement::getFirstAttribute()
{
    if (!isValid()) {
        return emptyAttribute;
    }
    if (current->getElement().type() != toml::value_t::table) {
        return emptyAttribute;
    }
    const auto& tab = current->getElement().as_table();
    auto attIterator = tab.begin();
    auto elementEnd = tab.end();
    iteratorCount = 0;

    while (attIterator != elementEnd) {
        if (isAttribute(attIterator->second)) {
            switch (attIterator->second.type()) {
            case toml::value_t::integer:
                    return {attIterator->first, std::to_string(attIterator->second.as_integer())};
            case toml::value_t::floating:
                    return {attIterator->first, std::to_string(attIterator->second.as_floating())};
            case toml::value_t::string:
                return {attIterator->first, attIterator->second.as_string()};
            case toml::value_t::boolean:
                return {attIterator->first,attIterator->second.as_boolean()?"true":"false"};
            }
        }
        ++attIterator;
        ++iteratorCount;
    }
    return emptyAttribute;
}

readerAttribute tomlReaderElement::getNextAttribute()
{
    if (!isValid()) {
        return emptyAttribute;
    }
    const auto& tab = current->getElement().as_table();
    auto attIterator = tab.begin();
    auto elementEnd = tab.end();
    for (int ii = 0; ii < iteratorCount; ++ii) {
        ++attIterator;
        if (attIterator == elementEnd) {
            return emptyAttribute;
        }
    }
    if (attIterator == elementEnd) {
        return emptyAttribute;
    }
    ++attIterator;
    ++iteratorCount;
    while (attIterator != elementEnd) {
        if (isAttribute(attIterator->second)) {
            switch (attIterator->second.type()) {
            case toml::value_t::integer:
                    return {attIterator->first, std::to_string(attIterator->second.as_integer())};
            case toml::value_t::floating:
                    return {attIterator->first, std::to_string(attIterator->second.as_floating())};
            case toml::value_t::string:
                return {attIterator->first, attIterator->second.as_string()};
            case toml::value_t::boolean:
                return {attIterator->first,attIterator->second.as_boolean()?"true":"false"};
            }
        }
        ++attIterator;
        ++iteratorCount;
    }
    return emptyAttribute;
}

readerAttribute tomlReaderElement::getAttribute(const std::string& attributeName) const
{
    std::string valueString;
    helicsfmi::fileops::replaceIfMember(current->getElement(), attributeName, valueString);

    if (!valueString.empty()) {
        return {attributeName, valueString};
    }
    return emptyAttribute;
}

std::string tomlReaderElement::getAttributeText(const std::string& attributeName) const
{
    return helicsfmi::fileops::getOrDefault(current->getElement(), attributeName, nullStr);
}

double tomlReaderElement::getAttributeValue(const std::string& attributeName) const
{
    return helicsfmi::fileops::getOrDefault(current->getElement(), attributeName, readerNullVal);
}

std::shared_ptr<readerElement> tomlReaderElement::firstChild() const
{
    auto newElement = clone();
    newElement->moveToFirstChild();
    return newElement;
}

std::shared_ptr<readerElement> tomlReaderElement::firstChild(const std::string& childName) const
{
    auto newElement = clone();
    newElement->moveToFirstChild(childName);
    return newElement;
}

void tomlReaderElement::moveToFirstChild()
{
    if (!isValid()) {
        return;
    }
    current->elementIndex = 0;
    if (current->getElement().type() == toml::value_t::table) {
        const auto& tab = current->getElement().as_table();
        auto elementIterator = tab.begin();
        auto endIterator = tab.end();

        while (elementIterator != endIterator) {
            if (isElement(elementIterator->second)) {
                parents.push_back(current);
                current =
                    std::make_shared<tomlElement>(elementIterator->first, elementIterator->first);
                return;
            }
            ++elementIterator;
            ++current->elementIndex;
        }
    }
    parents.push_back(current);
    current->clear();
}

void tomlReaderElement::moveToFirstChild(const std::string& childName)
{
    if (!isValid()) {
        return;
    }
    if (current->getElement().type() != toml::value_t::table) {
        return;
    }
    auto val = toml::find_or(current->getElement(), childName, uval);
    if (val.is_uninitialized()) {
        current = std::make_shared<tomlElement>(uval, childName);
    }

    parents.push_back(current);
    current = std::make_shared<tomlElement>(val, childName);
}

void tomlReaderElement::moveToNextSibling()
{
    if (!isValid()) {
        return;
    }
    ++current->arrayIndex;
    while (current->arrayIndex < current->count()) {
        if (!current->getElement().is_uninitialized()) {
            return;
        }
        ++current->arrayIndex;
    }
    if (parents.empty()) {
        current->clear();
        return;
    }
    // there are no more elements in a potential array

    const auto& tab = parents.back()->getElement().as_table();
    auto elementIterator = tab.begin();
    auto elementEnd = tab.end();
    ++parents.back()->elementIndex;
    // iterators don't survive copy so have to move the iterator to the next element index
    for (int ii = 0; ii < parents.back()->elementIndex; ++ii) {
        ++elementIterator;
        if (elementIterator == elementEnd) {
            current->clear();
        }
    }
    // Now find the next valid element
    while (elementIterator != elementEnd) {
        if (isElement(elementIterator->first)) {
            current = std::make_shared<tomlElement>(elementIterator->first, elementIterator->first);
            return;
        }
        ++elementIterator;
        ++parents.back()->elementIndex;
    }
    current->clear();
}

void tomlReaderElement::moveToNextSibling(const std::string& siblingName)
{
    if (!isValid()) {
        return;
    }
    if (siblingName == current->name) {
        ++current->arrayIndex;
        while (current->arrayIndex < current->count()) {
            if (!current->getElement().is_uninitialized()) {
                return;
            }
            ++current->arrayIndex;
        }
        current->clear();
    } else {
        auto val = toml::find_or(current->getElement(), siblingName, uval);
        if (val.is_uninitialized()) {
            current = std::make_shared<tomlElement>(uval, siblingName);
        }
    }
}

void tomlReaderElement::moveToParent()
{
    if (parents.empty()) {
        return;
    }
    current = parents.back();
    parents.pop_back();
}

std::shared_ptr<readerElement> tomlReaderElement::nextSibling() const
{
    auto newElement = clone();
    newElement->moveToNextSibling();
    return newElement;
}

std::shared_ptr<readerElement> tomlReaderElement::nextSibling(const std::string& siblingName) const
{
    auto newElement = clone();
    newElement->moveToNextSibling(siblingName);
    return newElement;
}

void tomlReaderElement::bookmark()
{
    bookmarks.push_back(std::static_pointer_cast<tomlReaderElement>(clone()));
}

void tomlReaderElement::restore()
{
    if (bookmarks.empty()) {
        return;
    }
    parents = bookmarks.back()->parents;
    current = bookmarks.back()->current;
    bookmarks.pop_back();
}

bool isAttribute(const toml::value& testValue)
{
    if (testValue.is_uninitialized()) {
        return false;
    }
    if (testValue.type() == toml::value_t::array) {
        return false;
    }
    if (testValue.type() == toml::value_t::table) {
        return false;
    }
    return true;
}

bool isElement(const toml::value& testValue)
{
    if (testValue.is_uninitialized()) {
        return false;
    }

    return (!isAttribute(testValue));
}
