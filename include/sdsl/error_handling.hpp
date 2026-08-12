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

/*
 * By default, SDSL_THROW(exception_type, message) throws
 * exception_type(message), matching sdsl-lite's traditional behavior. Some
 * environments (notably Bazel builds embedding sdsl-lite in exceptions-free
 * code, such as Google's DeepVariant) need to build with -fno-exceptions.
 * Defining SDSL_NO_EXCEPTIONS at build time switches SDSL_THROW to a
 * non-throwing fatal-error path instead: it logs the message with Abseil's
 * ABSL_LOG(FATAL) if SDSL_USE_ABSEIL_LOGGING is also defined, or otherwise
 * prints it to stderr and calls std::abort(). Either way, exception_type is
 * not evaluated, so it does not need to be a complete type when exceptions
 * are disabled.
 *
 * A few sdsl-lite call sites throw exceptions whose constructors don't fit
 * the (exception_type, message) shape: std::bad_alloc takes no arguments,
 * and the hugepage allocator's out-of-memory paths throw std::system_error
 * with an ENOMEM error code alongside the message. SDSL_THROW_BAD_ALLOC()
 * and SDSL_THROW_ERRNO(message) cover those two cases specifically, so that
 * the exceptions-enabled behavior at those sites is preserved exactly (down
 * to the exception type and, for the errno case, the error code) while
 * still degrading to a logged message when exceptions are off.
 */

#if defined(SDSL_NO_EXCEPTIONS)

#if defined(SDSL_USE_ABSEIL_LOGGING)
#include "absl/log/absl_log.h"
#define SDSL_THROW(exception_type, message) ABSL_LOG(FATAL) << (message)
#define SDSL_THROW_BAD_ALLOC() ABSL_LOG(FATAL) << "out of memory"
#define SDSL_THROW_ERRNO(message) ABSL_LOG(FATAL) << (message)
#else
#include <cstdlib>
#include <iostream>
#define SDSL_THROW(exception_type, message) \
    do { std::cerr << (message) << std::endl; std::abort(); } while (0)
#define SDSL_THROW_BAD_ALLOC() \
    do { std::cerr << "out of memory" << std::endl; std::abort(); } while (0)
#define SDSL_THROW_ERRNO(message) \
    do { std::cerr << (message) << std::endl; std::abort(); } while (0)
#endif

#else

#include <stdexcept>
#include <new>
#include <system_error>
#include <cerrno>
#define SDSL_THROW(exception_type, message) throw exception_type(message)
#define SDSL_THROW_BAD_ALLOC() throw std::bad_alloc()
#define SDSL_THROW_ERRNO(message) throw std::system_error(ENOMEM, std::system_category(), message)

#endif

#endif
