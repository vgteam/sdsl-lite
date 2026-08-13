#include "sdsl/int_vector.hpp"
#include "gtest/gtest.h"

#include <iostream>
#include <vector>

namespace
{

#ifdef SDSL_ENABLE_SHARED_MEMORY

// Stands in for a block of memory int_vector did not allocate itself, e.g. shared memory.
std::vector<uint64_t> make_external_buffer(size_t words)
{
    return std::vector<uint64_t>(words, 0);
}

// 20 elements * 64 bits each == 20 uint64_t words, matching the buffer size exactly.
TEST(SharedMemoryIntVectorTest, DoesNotFreeExternalBuffer)
{
    std::vector<uint64_t> external_buffer = make_external_buffer(20);
    {
        sdsl::int_vector<0> view(20, 64, external_buffer.data(), true);
        EXPECT_TRUE(view.loaded_from_shared_memory());
        for (size_t i = 0; i < view.size(); i++) { view[i] = i; }
        for (size_t i = 0; i < view.size(); i++) { EXPECT_EQ(view[i], i); }
    }
    EXPECT_EQ(external_buffer.size(), 20u);
}

TEST(SharedMemoryIntVectorTest, TwoViewsOfTheSameBufferShareData)
{
    std::vector<uint64_t> external_buffer = make_external_buffer(20);
    sdsl::int_vector<0> first(20, 64, external_buffer.data(), true);
    sdsl::int_vector<0> second(20, 64, external_buffer.data(), true);
    first[0] = 12345;
    EXPECT_EQ(second[0], 12345u);
}

TEST(SharedMemoryIntVectorTest, CopyFromSharedIsIndependent)
{
    std::vector<uint64_t> external_buffer = make_external_buffer(2);
    sdsl::int_vector<0> shared(10, 8, external_buffer.data(), true);
    for (size_t i = 0; i < shared.size(); i++) { shared[i] = i; }

    sdsl::int_vector<0> copy(shared);
    EXPECT_FALSE(copy.loaded_from_shared_memory());
    ASSERT_EQ(copy.size(), shared.size());
    for (size_t i = 0; i < shared.size(); i++) { EXPECT_EQ(copy[i], shared[i]); }
    copy[0] = copy[0] + 1;
    EXPECT_NE(copy[0], shared[0]);

    sdsl::int_vector<0> assigned;
    assigned = shared;
    EXPECT_FALSE(assigned.loaded_from_shared_memory());
    ASSERT_EQ(assigned.size(), shared.size());
    for (size_t i = 0; i < shared.size(); i++) { EXPECT_EQ(assigned[i], shared[i]); }
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
