/*
 * LLNS Copyright Start
 * Copyright (c) 2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */
#include "helics-fmi/helics-fmi-config.h"

#ifndef BOOST_STATIC
#    define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE helics_fmi_tests
#define BOOST_TEST_DETECT_MEMORY_LEAK 0

#include "helics/application_api/Federate.hpp"

#include <boost/test/unit_test.hpp>

struct globalTestConfig {
    globalTestConfig() = default;
    ~globalTestConfig() { helics::cleanupHelicsLibrary(); }
};

//____________________________________________________________________________//

BOOST_GLOBAL_FIXTURE(globalTestConfig);
