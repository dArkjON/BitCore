// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Copyright (c) 2014-2017 The Dash Core developers
// Copyright (c) 2019 Limxtec developers
// Copyright (c) 2019 BitCore developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * Server/client environment: argument handling, config file parsing,
 * thread wrappers, startup time
 */
#ifndef BITCORE_UTIL_H
#define BITCORE_UTIL_H

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

// Bitcoin 0.18 moved util functions to util/system.h
// This file now primarily includes that and adds Bitcore-specific extensions
#include <util/system.h>

#include <compat.h>
#include <fs.h>
#include <logging.h>
#include <sync.h>
#include <tinyformat.h>
#include <utiltime.h>

#include <atomic>
#include <exception>
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/signals2/signal.hpp>
#include <boost/thread/condition_variable.hpp> // for boost::thread_interrupted

// Application startup time (used for uptime calculation)
int64_t GetStartupTime();

// Dash
// Debugging macros

// Uncomment the following line to enable debugging messages
// or enable on a per file basis prior to inclusion of util.h
//#define ENABLE_BTX_DEBUG
#define ENABLE_DASH_DEBUG

#ifdef ENABLE_BTX_DEBUG
#define DBG( x ) x
#else
#define DBG( x )
#endif

//Bitcore Related Stuff

#ifdef ENABLE_DASH_DEBUG
    #define DBG_DASH( x ) x
#else
    #define DBG_DASH( x )
#endif

// Translation function _(), SetupEnvironment(), SetupNetworking() are defined in util/system.h

#ifndef WIN32
std::string ShellEscape(const std::string& arg);
#endif
void runCommand(const std::string& strCommand);

// Bitcore-specific: Dash version conversion functions
/**
 * @brief Converts version strings to 4-byte unsigned integer
 * @param strVersion version in "x.x.x" format (decimal digits only)
 * @return 4-byte unsigned integer, most significant byte is always 0
 * Throws std::bad_cast if format doesn't match.
 */
uint32_t StringVersionToInt(const std::string& strVersion);

/**
 * @brief Converts version as 4-byte unsigned integer to string
 * @param nVersion 4-byte unsigned integer, most significant byte is always 0
 * @return version string in "x.x.x" format (last 3 bytes as version parts)
 * Throws std::bad_cast if format doesn't match.
 */
std::string IntVersionToString(uint32_t nVersion);


/**
 * @brief Copy of the IntVersionToString, that returns "Invalid version" string
 * instead of throwing std::bad_cast
 * @param nVersion 4-byte unsigned integer, most significant byte is always 0
 * @return version string in "x.x.x" format (last 3 bytes as version parts)
 * or "Invalid version" if can't cast the given value
 */
std::string SafeIntVersionToString(uint32_t nVersion);

#endif // BITCORE_UTIL_H
