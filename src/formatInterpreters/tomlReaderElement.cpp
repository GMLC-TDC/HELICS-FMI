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
    for (auto& parent : parents) {
        ret->parents.push_back(std::make_shared<tomlElement>(*parent));
    }
    ret->current = std::make_shared<tomlElement>(*current);
    ret->doc = doc;
    return ret;
}

bool tomlReaderElement::loadFile(const std::string& fileName)
{
    try {
        auto vres = helics_fmi::fileops::loadToml(fileName);
        doc = std::make_shared<tomlElement>(vres, fileName);
        current = doc;
        clear();
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
        auto vres = helics_fmi::fileops::loadTomlStr(inputString);
        doc = std::make_shared<tomlElement>(vres, inputString);
        current = doc;
        clear();
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
    if (!isValid()) {
        return nullStr;
    }

    if (current->getElement().is_string()) {
        return current->getElement().as_string();
    }
    return nullStr;
}

std::string tomlReaderElement::getMultiText(const std::string& /*sep*/) const
{
    if (!isValid()) {
        return nullStr;
    }

    if (current->getElement().is_string()) {
        return current->getElement().as_string();
    }
    return nullStr;
}
// no attributes in toml
bool tomlReaderElement::hasAttribute(const std::string& attributeName) const
{
    if (!isValid()) {
        return false;
    }

    toml::value uval;
    auto val = toml::find_or(current->getElement(), attributeName, uval);
    if (!val.is_uninitialized()) {
        return false;
    }
    return (!(val.is_array() || val.is_table()));
}

bool tomlReaderElement::hasElement(const std::string& elementName) const
{
    toml::value uval;
    auto val = toml::find_or(current->getElement(), elementName, uval);
    if (!val.is_uninitialized()) {
        return false;
    }
    return (val.is_array() || val.is_table());
}

readerAttribute tomlReaderElement::getFirstAttribute()
{
    if (!isValid()) {
        return readerAttribute();
    }
    if (current->getElement().type() != toml::value_t::table) {
        return readerAttribute();
    }
    auto& tab = current->getElement().as_table();
    auto attIterator = tab.begin();
    auto elementEnd = tab.end();
    iteratorCount = 0;

    while (attIterator != elementEnd) {
        if (isAttribute(attIterator->second)) {
            return readerAttribute(attIterator->first, attIterator->second.as_string());
        }
        ++attIterator;
        ++iteratorCount;
    }
    return readerAttribute();
}

readerAttribute tomlReaderElement::getNextAttribute()
{
    if (!isValid()) {
        return readerAttribute();
    }
    auto& tab = current->getElement().as_table();
    auto attIterator = tab.begin();
    auto elementEnd = tab.end();
    for (int ii = 0; ii < iteratorCount; ++ii) {
        ++attIterator;
        if (attIterator == elementEnd) {
            return readerAttribute();
        }
    }
    if (attIterator == elementEnd) {
        return readerAttribute();
    }
    ++attIterator;
    ++iteratorCount;
    while (attIterator != elementEnd) {
        if (isAttribute(attIterator->second)) {
            return readerAttribute(attIterator->first, attIterator->second.as_string());
        }
        ++attIterator;
        ++iteratorCount;
    }
    return readerAttribute();
}

readerAttribute tomlReaderElement::getAttribute(const std::string& attributeName) const
{
    std::string vs;
    helics_fmi::fileops::replaceIfMember(current->getElement(), attributeName, vs);

    if (!vs.empty()) {
        return readerAttribute(attributeName, vs);
    }
    return readerAttribute();
}

std::string tomlReaderElement::getAttributeText(const std::string& attributeName) const
{
    return helics_fmi::fileops::getOrDefault(current->getElement(), attributeName, nullStr);
}

double tomlReaderElement::getAttributeValue(const std::string& attributeName) const
{
    return helics_fmi::fileops::getOrDefault(current->getElement(), attributeName, readerNullVal);
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
        auto& tab = current->getElement().as_table();
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
    toml::value uval;
    auto val = toml::find_or(current->getElement(), childName, uval);
    if (!val.is_uninitialized()) {
        current = std::make_shared<tomlElement>(uval, childName);
    }

    parents.push_back(current);
    current->clear();
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

    auto& tab = parents.back()->getElement().as_table();
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
        toml::value uval;
        auto val = toml::find_or(current->getElement(), siblingName, uval);
        if (!val.is_uninitialized()) {
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
