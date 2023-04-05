/*
 * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "exeTestHelper.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <streambuf>

int exeTestRunner::counter = 1;

exeTestRunner::exeTestRunner()
{
    ++counter;
    outFile = "exeText_" + std::to_string(counter) + ".out";
}

exeTestRunner::exeTestRunner(const std::string& baseLocation, const std::string& target)
{
    ++counter;
    outFile = "exeText_" + std::to_string(counter) + ".out";
    active = findFileLocation(baseLocation, target);
}

exeTestRunner::exeTestRunner(const std::string& baseLocation,
                             const std::string& baseLocation2,
                             const std::string& target)
{
    ++counter;
    outFile = "exeText_" + std::to_string(counter) + ".out";
    active = findFileLocation(baseLocation, target);
    if (!active) {
        active = findFileLocation(baseLocation2, target);
    }
}

bool exeTestRunner::findFileLocation(const std::string& baseLocation, const std::string& target)
{
    const std::filesystem::path sourcePath(baseLocation);

    const auto tryPath1 = sourcePath / target;
    if (std::filesystem::exists(tryPath1)) {
        exeString = tryPath1.string();
        return true;
    }

    const auto tryPath2 = sourcePath / (target + ".exe");
    if (std::filesystem::exists(tryPath2)) {
        exeString = tryPath2.string();
        return true;
    }
#ifndef NDEBUG
    const auto tryPathD1 = sourcePath / "Debug" / target;
    if (std::filesystem::exists(tryPathD1)) {
        exeString = tryPathD1.string();
        return true;
    }

    const auto tryPathD2 = sourcePath / "Debug" / (target + ".exe");
    if (std::filesystem::exists(tryPathD2)) {
        exeString = tryPathD2.string();
        return true;
    }
#endif
    const auto tryPathR1 = sourcePath / "Release" / target;
    if (std::filesystem::exists(tryPathR1)) {
        exeString = tryPathR1.string();
        return true;
    }

    const auto tryPathR2 = sourcePath / "Release" / (target + ".exe");
    if (std::filesystem::exists(tryPathR2)) {
        exeString = tryPathR2.string();
        return true;
    }

    const std::filesystem::path tryPatht1 = target;
    if (std::filesystem::exists(tryPatht1)) {
        exeString = tryPatht1.string();
        return true;
    }

    const std::filesystem::path tryPatht2 = (target + ".exe");
    if (std::filesystem::exists(tryPatht2)) {
        exeString = tryPatht2.string();
        return true;
    }
    return false;
}

std::future<int> exeTestRunner::runAsync(const std::string& args) const
{
    if (!active) {
        std::promise<int> prom;
        auto fut = prom.get_future();
        prom.set_value(-101);
        return fut;
    }
    const std::string rstr = exeString + " " + args;
    return std::async(std::launch::async, [rstr]() { return system(rstr.c_str()); });
}

int exeTestRunner::run(const std::string& args) const
{
    if (!active) {
        return -101;
    }
    const std::string rstr = exeString + " " + args;
    return system(rstr.c_str());
}

std::string exeTestRunner::runCaptureOutput(const std::string& args) const
{
    if (!active) {
        return "invalid executable";
    }
    const std::string rstr = exeString + " " + args + " > " + outFile + " 2>&1";
    int ret = system(rstr.c_str());

    std::ifstream capture(outFile);
    std::string str((std::istreambuf_iterator<char>(capture)), std::istreambuf_iterator<char>());

    str.append(exeString);
    str.append(" returned ");
    str.append(std::to_string(ret) + "\n");
    ret = remove(outFile.c_str());
    if (ret != 1) {
        str.append(" failed to remove output file\n");
    }
    return str;
}

std::future<std::string> exeTestRunner::runCaptureOutputAsync(const std::string& args) const
{
    if (!active) {
        std::promise<std::string> prom;
        auto fut = prom.get_future();
        prom.set_value("invalid executable");
        return fut;
    }
    return std::async(std::launch::async, [this, args]() {
        return runCaptureOutput(args);
    });
}
