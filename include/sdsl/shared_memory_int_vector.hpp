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
/*! \file shared_memory_int_vector.hpp
    \brief shared_memory_int_vector.hpp contains publish_int_vector() and
           attach_int_vector(), which place an int_vector's bits into a named
           Boost.Interprocess shared memory segment (or attach to bits a
           prior call already placed there), building on the
           shared-memory-wrapping int_vector constructor in int_vector.hpp.
*/
#ifndef INCLUDED_SDSL_SHARED_MEMORY_INT_VECTOR
#define INCLUDED_SDSL_SHARED_MEMORY_INT_VECTOR

#if !defined(SDSL_ENABLE_SHARED_MEMORY)
#error "sdsl/shared_memory_int_vector.hpp requires building with SDSL_ENABLE_SHARED_MEMORY"
#endif

#include "int_vector.hpp"
#include "error_handling.hpp"

#include <boost/interprocess/managed_shared_memory.hpp>

#include <algorithm>
#include <cstring>
#include <string>

namespace sdsl
{

namespace bi = boost::interprocess;

/*
  publish_int_vector() copies an int_vector's bits into a freshly allocated,
  named buffer in an existing Boost.Interprocess shared memory segment,
  returning a second int_vector that wraps that buffer directly (via the
  (size, width, data, loaded_from_shared_memory) constructor in
  int_vector.hpp) rather than copying it again into its own heap memory.
  attach_int_vector() finds a buffer a prior publish_int_vector() call
  placed under the same name -- in the same segment or, if the segment was
  opened independently by another process that mapped it at the same
  address, a different process's handle to it -- and wraps it the same way,
  without copying.

  Both functions leave int_vector's own default (owning) behavior completely
  unchanged: they are separate, opt-in functions for callers who
  specifically want this, gated behind SDSL_ENABLE_SHARED_MEMORY.

  Neither function creates, destroys, or otherwise manages the lifetime of
  `shared_memory` itself -- that is the caller's responsibility, same as
  with Boost.Interprocess generally. The int_vector each function returns
  does not own the memory it wraps (its destructor and bit_resize() are
  no-ops; see loaded_from_shared_memory() in int_vector.hpp), so it must not
  outlive `shared_memory`, and the caller must remove the underlying segment
  itself once no attached int_vector is using it.
*/

//! Element count and bit width, stored in shared memory alongside the
//! packed words under a name derived from the caller's chosen name, so that
//! attach_int_vector() -- which starts with no int_vector of its own to
//! consult -- can recover the shape of the published data.
struct shared_int_vector_header
{
    uint64_t size;  //!< Number of elements, as int_vector<t_width>::size() would report.
    uint8_t  width; //!< Bits per element.
};

namespace shared_memory_int_vector_detail
{
    inline std::string header_name(const std::string& name) { return name + ".sdsl_shm_header"; }
    inline std::string words_name(const std::string& name) { return name + ".sdsl_shm_words"; }

    // int_vector's own memory_manager::resize() always allocates at least
    // one 64-bit word, even for a zero-length vector (see the "+ 64" in its
    // allocated_bytes computation in memory_management.hpp): rank/select
    // structures rely on being able to read one word past bit_size() to
    // answer rank(size()). Matching that here, rather than allocating zero
    // words when source.capacity() is 0, keeps a published vector's shape
    // consistent with what a normal (non-shared) int_vector<t_width> of the
    // same size would look like, and sidesteps Boost's handling of
    // zero-length array construction.
    template<uint8_t t_width>
    inline size_t word_count(const int_vector<t_width>& v)
    {
        return std::max<size_t>(v.capacity() / 64, 1);
    }
}

//! Copies `source`'s bits into a freshly allocated buffer named `name` in
//! `shared_memory`, and returns an int_vector wrapping that buffer.
/*! Throws (via SDSL_THROW) if `name` is already in use in the segment. */
template<uint8_t t_width>
int_vector<t_width> publish_int_vector(bi::managed_shared_memory& shared_memory, const std::string& name, const int_vector<t_width>& source)
{
    size_t num_words = shared_memory_int_vector_detail::word_count(source);

    shared_int_vector_header* header =
        shared_memory.construct<shared_int_vector_header>(shared_memory_int_vector_detail::header_name(name).c_str())();
    uint64_t* words =
        shared_memory.construct<uint64_t>(shared_memory_int_vector_detail::words_name(name).c_str())[num_words](uint64_t(0));
    if (header == nullptr || words == nullptr) {
        SDSL_THROW(std::runtime_error, "publish_int_vector: an object named \"" + name + "\" already exists in shared memory");
    }

    header->size = source.size();
    header->width = source.width();
    size_t data_words = source.capacity() / 64;
    if (data_words > 0) {
        std::memcpy(words, source.data(), data_words * sizeof(uint64_t));
    }

    return int_vector<t_width>(header->size, header->width, words, true);
}

//! Attaches to an int_vector previously published under `name` by
//! publish_int_vector() in `shared_memory`.
/*! Throws (via SDSL_THROW) if no such object exists. */
template<uint8_t t_width>
int_vector<t_width> attach_int_vector(bi::managed_shared_memory& shared_memory, const std::string& name)
{
    shared_int_vector_header* header =
        shared_memory.find<shared_int_vector_header>(shared_memory_int_vector_detail::header_name(name).c_str()).first;
    uint64_t* words =
        shared_memory.find<uint64_t>(shared_memory_int_vector_detail::words_name(name).c_str()).first;
    if (header == nullptr || words == nullptr) {
        SDSL_THROW(std::runtime_error, "attach_int_vector: no object named \"" + name + "\" in shared memory");
    }

    return int_vector<t_width>(header->size, header->width, words, true);
}

} // namespace sdsl

#endif
