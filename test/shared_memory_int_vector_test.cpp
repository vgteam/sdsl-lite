#include "sdsl/int_vector.hpp"
#include "gtest/gtest.h"

#include <iostream>
#include <random>

#ifdef SDSL_ENABLE_SHARED_MEMORY
#include "sdsl/shared_memory_int_vector.hpp"

#include <unistd.h>
#include <boost/interprocess/shared_memory_object.hpp>

#include <string>
#endif

namespace
{

#ifdef SDSL_ENABLE_SHARED_MEMORY

namespace bi = boost::interprocess;

// Two independent bi::managed_shared_memory handles to the same named
// segment stand in for two separate processes: nothing here is shared
// except the segment name, so a real cross-process attach has to work the
// same way this does.
class SharedMemoryIntVectorTest : public ::testing::Test
{
public:
    std::string segment_name;

    void SetUp() override
    {
        this->segment_name = "sdsl_test_shared_memory_" + std::to_string(::getpid());
        bi::shared_memory_object::remove(this->segment_name.c_str());
    }

    void TearDown() override
    {
        bi::shared_memory_object::remove(this->segment_name.c_str());
    }
};

TEST_F(SharedMemoryIntVectorTest, AttachFromIndependentHandle)
{
    sdsl::int_vector<> source(1000, 0, 37);
    std::mt19937_64 rng(0x5344534C);
    std::uniform_int_distribution<uint64_t> distribution(0, (1ULL << 37) - 1);
    for (size_t i = 0; i < source.size(); i++) {
        source[i] = distribution(rng);
    }

    bi::managed_shared_memory writer_segment(bi::create_only, this->segment_name.c_str(), 4 * 1024 * 1024);
    sdsl::int_vector<> writer = sdsl::publish_int_vector(writer_segment, "vec", source);
    ASSERT_EQ(writer.size(), source.size()) << "Published int_vector has the wrong size";
    EXPECT_EQ(writer.width(), source.width()) << "Published int_vector has the wrong width";
    for (size_t i = 0; i < source.size(); i++) {
        EXPECT_EQ(writer[i], source[i]) << "Published int_vector has the wrong value at offset " << i;
    }

    // A second, independent handle to the same segment stands in for a second process.
    bi::managed_shared_memory reader_segment(bi::open_only, this->segment_name.c_str());
    sdsl::int_vector<> reader = sdsl::attach_int_vector<0>(reader_segment, "vec");
    ASSERT_EQ(reader.size(), source.size()) << "Attached int_vector has the wrong size";
    EXPECT_EQ(reader.width(), source.width()) << "Attached int_vector has the wrong width";
    for (size_t i = 0; i < source.size(); i++) {
        EXPECT_EQ(reader[i], source[i]) << "Attached int_vector has the wrong value at offset " << i;
    }

    // Both int_vectors wrap the same underlying shared memory, so a write
    // through one must be visible through the other.
    writer[0] = writer[0] ^ 1;
    EXPECT_EQ(reader[0], writer[0]) << "Write through the writer was not visible through the reader";
}

#ifndef SDSL_NO_EXCEPTIONS
// With SDSL_NO_EXCEPTIONS, this failure path reports through SDSL_THROW's
// non-throwing branch (see error_handling.hpp), which cannot be caught by
// ASSERT_THROW, so there is nothing left here to check in that build.
TEST_F(SharedMemoryIntVectorTest, AttachToMissingObjectFails)
{
    bi::managed_shared_memory segment(bi::create_only, this->segment_name.c_str(), 1024 * 1024);
    ASSERT_THROW((sdsl::attach_int_vector<0>(segment, "does_not_exist")), std::runtime_error)
        << "Attaching to a nonexistent shared-memory object should fail instead of silently succeeding";
}
#endif

TEST_F(SharedMemoryIntVectorTest, CopyFromSharedIsIndependent)
{
    sdsl::int_vector<> source(10, 0, 8);
    for (size_t i = 0; i < source.size(); i++) { source[i] = i; }

    bi::managed_shared_memory segment(bi::create_only, this->segment_name.c_str(), 1024 * 1024);
    sdsl::int_vector<> shared = sdsl::publish_int_vector(segment, "vec", source);

    // A copy of a shared-memory-backed int_vector must be an independent,
    // owned vector: this exercises the fix for the bug in Mobin Asri's
    // original shared-memory patch, where the copy constructor propagated
    // shared_memory_flag from the source, causing bit_resize() to skip
    // allocating the copy's own storage while the copy still memcpy'd into
    // it.
    sdsl::int_vector<> copy(shared);
    EXPECT_FALSE(copy.loaded_from_shared_memory()) << "A copy of a shared-memory int_vector should own its own memory";
    ASSERT_EQ(copy.size(), shared.size());
    for (size_t i = 0; i < shared.size(); i++) {
        EXPECT_EQ(copy[i], shared[i]);
    }

    // Mutating the copy must not affect the shared original.
    copy[0] = copy[0] + 1;
    EXPECT_NE(copy[0], shared[0]);

    sdsl::int_vector<> assigned;
    assigned = shared;
    EXPECT_FALSE(assigned.loaded_from_shared_memory()) << "Assigning from a shared-memory int_vector should own its own memory";
    ASSERT_EQ(assigned.size(), shared.size());
    for (size_t i = 0; i < shared.size(); i++) {
        EXPECT_EQ(assigned[i], shared[i]);
    }
}

#endif // SDSL_ENABLE_SHARED_MEMORY

} // namespace

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    if (argc < 2) {
        // LCOV_EXCL_START
        std::cout << "Usage: " << argv[0] << " tmp_dir" << std::endl;
        return 1;
        // LCOV_EXCL_STOP
    }
    return RUN_ALL_TESTS();
}
