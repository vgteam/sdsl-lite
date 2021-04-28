#include "sdsl/simple_sds.hpp"
#include "gtest/gtest.h"

#include <cstdio>
#include <cstdlib>

using namespace sdsl::simple_sds;

namespace
{

//-----------------------------------------------------------------------------

struct ByteArray
{
    std::vector<std::uint8_t> bytes;
    size_t                    sum = 0;

    void simple_sds_serialize(std::ostream& out) const
    {
        serialize_vector(this->bytes, out);
        serialize_value(this->sum, out);
    }

    void simple_sds_load(std::istream& in)
    {
        this->bytes = load_vector<std::uint8_t>(in);
        this->sum = load_value<size_t>(in);
        size_t real_sum = 0;
        for (auto value : this->bytes) { real_sum += value; }
        if (real_sum != this->sum) {
            throw InvalidData("Incorrect sum");
        }
    }

    size_t simple_sds_size() const
    {
        return vector_size(this->bytes) + value_size(this->sum);
    }

    bool operator==(const ByteArray& another) const
    {
        return (this->bytes == another.bytes && this->sum == another.sum);
    }
};

struct ComplexStructure
{
    std::pair<size_t, size_t> header;
    bool                      has_byte_array;
    ByteArray                 byte_array;
    std::vector<double>       numbers;

    void simple_sds_serialize(std::ostream& out) const
    {
        serialize_value(this->header, out);
        if (this->has_byte_array) {
            serialize_option(this->byte_array, out);
        } else {
            empty_option(out);
        }
        serialize_vector(this->numbers, out);
    }

    void simple_sds_load(std::istream& in)
    {
        this->header = load_value<std::pair<size_t, size_t>>(in);
        this->has_byte_array = load_option(this->byte_array, in);
        this->numbers = load_vector<double>(in);
    }

    size_t simple_sds_size() const
    {
        size_t result = value_size(this->header);
        result += (this->has_byte_array ? option_size(this->byte_array) : empty_option_size());
        result += vector_size(this->numbers);
        return result;
    }

    bool operator==(const ComplexStructure& another) const
    {
        return (this->header == another.header &&
                this->has_byte_array == another.has_byte_array &&
                this->byte_array == another.byte_array &&
                this->numbers == another.numbers);
    }
};

//-----------------------------------------------------------------------------

void check_complex_structure(const ComplexStructure& original, size_t expected_size)
{
    ASSERT_EQ(original.simple_sds_size(), expected_size) << "Invalid serialization size in elements";

    char buffer[] = "simple-sds-XXXXXX";
    int fail = mkstemp(buffer);
    ASSERT_NE(fail, -1) << "Temporary file creation failed";
    std::string filename(buffer);

    serialize_to(original, filename);
    size_t file_size = 0;
    struct stat stat_buf;
    if (stat(filename.c_str(), &stat_buf) == 0) { file_size = stat_buf.st_size; }
    EXPECT_EQ(file_size, expected_size * sizeof(element_type)) << "Invalid file size";

    ComplexStructure loaded;
    load_from(loaded, filename);
    EXPECT_EQ(loaded, original) << "Invalid loaded structure";

    std::ifstream in(filename, std::ios_base::binary);
    in.seekg(value_size(original.header) * sizeof(element_type));
    skip_option(in);
    std::vector<double> numbers = load_vector<double>(in);
    EXPECT_EQ(numbers, original.numbers) << "Invalid numbers after skipping the optional structure";

    std::remove(filename.c_str());
}

//-----------------------------------------------------------------------------

TEST(SimpleSDSTest, Empty)
{
    ComplexStructure original = {
        { 123, 456, },
        false,
        { { }, 0, },
        { },
    };
    size_t expected_size = 2 + 1 + 1;
    check_complex_structure(original, expected_size);
}

TEST(SimpleSDSTest, Numbers)
{
    ComplexStructure original = {
        { 123, 456, },
        false,
        { { }, 0, },
        { 1.0, 2.0, 3.0, 5.0, },
    };
    size_t expected_size = 2 + 1 + 5;
    check_complex_structure(original, expected_size);
}

TEST(SimpleSDSTest, EmptyBytes)
{
    ComplexStructure original = {
        { 123, 456, },
        true,
        { { }, 0, },
        { },
    };
    size_t expected_size = 2 + 3 + 1;
    check_complex_structure(original, expected_size);
}

TEST(SimpleSDSTest, BytesWithPadding)
{
    ComplexStructure original = {
        { 123, 456, },
        true,
        { { 1, 2, 3, 5, 8, }, 19, },
        { },
    };
    size_t expected_size = 2 + 4 + 1;
    check_complex_structure(original, expected_size);
}

TEST(SimpleSDSTest, BytesAndNumbers)
{
    ComplexStructure original = {
        { 123, 456, },
        true,
        { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, }, 136, },
        { 1.0, 2.0, 3.0, 5.0 },
    };
    size_t expected_size = 2 + 5 + 5;
    check_complex_structure(original, expected_size);
}

TEST(SimpleSDSTest, BytesAndNumbersWithPadding)
{
    ComplexStructure original = {
        { 123, 456, },
        true,
        { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, }, 120, },
        { 1.0, 2.0, 3.0, 5.0 },
    };
    size_t expected_size = 2 + 5 + 5;
    check_complex_structure(original, expected_size);
}

//-----------------------------------------------------------------------------

} // anonymous namespace

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

//-----------------------------------------------------------------------------
