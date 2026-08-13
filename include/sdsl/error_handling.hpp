/* sdsl - succinct data structures library
    Copyright (C) 2008-2013 Simon Gog

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see http://www.gnu.org/licenses/ .
*/
/*! \file error_handling.hpp
    \brief error_handling.hpp contains SDSL_THROW and related macros, which
           sdsl-lite uses to report unrecoverable errors, so that the
           error-reporting mechanism can be selected at build time.
*/
#ifndef INCLUDED_SDSL_ERROR_HANDLING
#define INCLUDED_SDSL_ERROR_HANDLING

// SDSL_THROW always constructs its argument, even when not thrown.
#include <stdexcept>
#include <new>
#include <system_error>
#include <cerrno>

/*
 * SDSL_THROW(exception) throws the given exception object, unless
 * SDSL_NO_EXCEPTIONS is defined, in which case it logs exception.what() and
 * calls std::abort() instead (via Abseil if SDSL_USE_ABSEIL_LOGGING is set).
 */

#if defined(SDSL_NO_EXCEPTIONS)

#if defined(SDSL_USE_ABSEIL_LOGGING)
#include "absl/log/absl_log.h"
#define SDSL_THROW(exception) ABSL_LOG(FATAL) << (exception).what()
#else
#include <cstdlib>
#include <iostream>
#define SDSL_THROW(exception) \
    do { std::cerr << (exception).what() << std::endl; std::abort(); } while (0)
#endif

#else

#define SDSL_THROW(exception) throw (exception)

#endif

#endif
