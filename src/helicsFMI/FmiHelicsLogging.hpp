/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#define LOG_ERROR(message) logMessage(HELICS_LOG_LEVEL_ERROR, message)
#define LOG_WARNING(message) logMessage(HELICS_LOG_LEVEL_WARNING, message)

#define LOG_SUMMARY(message)                                                                       \
    if (logLevel >= HELICS_LOG_LEVEL_SUMMARY) {                                                    \
        logMessage(HELICS_LOG_LEVEL_SUMMARY, message);                                             \
    }

#define LOG_CONNECTIONS(message)                                                                   \
    if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {                                                \
        logMessage(HELICS_LOG_LEVEL_CONNECTIONS, message);                                         \
    }

#define LOG_INTERFACES(message)                                                                    \
    if (logLevel >= HELICS_LOG_LEVEL_INTERFACES) {                                                 \
        logMessage(HELICS_LOG_LEVEL_INTERFACES, message);                                          \
    }

#define LOG_TIMING(message)                                                                        \
    if (logLevel >= HELICS_LOG_LEVEL_TIMING) {                                                     \
        logMessage(HELICS_LOG_LEVEL_TIMING, message);                                              \
    }
#define LOG_DATA_MESSAGES(message)                                                                 \
    if (logLevel >= HELICS_LOG_LEVEL_DATA) {                                                       \
        logMessage(HELICS_LOG_LEVEL_DATA, message);                                                \
    }
#define LOG_DEBUG_MESSAGES(message)                                                                \
    if (logLevel >= HELICS_LOG_LEVEL_DEBUG) {                                                      \
        logMessage(HELICS_LOG_LEVEL_DEBUG, message);                                               \
    }

#define LOG_TRACE(message)                                                                         \
    if (logLevel >= HELICS_LOG_LEVEL_TRACE) {                                                      \
        logMessage(HELICS_LOG_LEVEL_TRACE, message);                                               \
    }
