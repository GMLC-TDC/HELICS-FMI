/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>

class FmiLibrary;
class fmi2ModelExchangeObject;
class fmi2CoSimObject;

/** singleton class for managing fmi library objects*/
class fmiLibraryManager {
  private:
    std::map<std::string, std::shared_ptr<FmiLibrary>> libraries;
    std::map<std::string, std::string> quickReferenceLibraries;
    mutable std::mutex libraryLock;

  public:
    ~fmiLibraryManager();
    std::shared_ptr<FmiLibrary> getLibrary(const std::string& libFile);
    std::unique_ptr<fmi2ModelExchangeObject>
        createModelExchangeObject(const std::string& fmuIdentifier, const std::string& ObjectName);
    std::unique_ptr<fmi2CoSimObject> createCoSimulationObject(const std::string& fmuIdentifier,
                                                              const std::string& ObjectName);
    void loadBookMarkFile(const std::string& bookmarksFile);
    void addShortCut(const std::string& name, const std::string& fmuLocation);
    static fmiLibraryManager& instance();

  private:
    fmiLibraryManager();
};
