#include "gtest/gtest.h"
#include "src/compression/encoding.h"
#include "src/compression/encoding_rle.h"
#include "src/compression/encoding_dict.h"
#include "src/compression/encoding_delta.h"

TEST(Rle, Without_bitpack) {
    {
        std::vector<int32_t> input = {1, 1, 2};
        std::vector<char> expected = {
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // run_count=2
            '\x01','\x00','\x00','\x00',                             // value=1
            '\x02','\x00','\x00','\x00',                             // value=2
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=2
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=1
        };
        auto encoded = EncodeRle<int32_t>(input, false);
        EXPECT_EQ(encoded, expected);
        size_t offset = 0;
        EXPECT_EQ(DecodeRle<int32_t>(encoded, offset, 3, false), input);
    }
    {
        std::vector<float> input = {1.0f, 1.0f, 2.0f};
        std::vector<char> expected = {
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // run_count=2
            '\x00','\x00','\x80','\x3F',                             // value=1.0f
            '\x00','\x00','\x00','\x40',                             // value=2.0f
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=2
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=1
        };
        auto encoded = EncodeRle<float>(input, false);
        EXPECT_EQ(encoded, expected);
        size_t offset = 0;
        EXPECT_EQ(DecodeRle<float>(encoded, offset, 3, false), input);
    }
    {
        std::vector<std::string> input = {"ab", "ab", "cd"};
        std::vector<char> expected = {
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00',          // run_count=2
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00', 'a','b', // value="ab"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00', 'c','d', // value="cd"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00',          // count=2
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00',          // count=1
        };
        auto encoded = EncodeRle<std::string>(input, false);
        EXPECT_EQ(encoded, expected);
        size_t offset = 0;
        EXPECT_EQ(DecodeRle<std::string>(encoded, offset, 3, false), input);
    }
}

TEST(Rle, With_bitpack) {
    {
        std::vector<int32_t> input = {1, 1, 2};
        std::vector<char> expected = {
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // run_count=2
            '\x01','\x00','\x00','\x00',                             // value=1
            '\x02','\x00','\x00','\x00',                             // value=2
            '\x02', '\x06',                                          // bits=2, packed [2,1]
        };
        auto encoded = EncodeRle<int32_t>(input, true);
        EXPECT_EQ(encoded, expected);
        size_t offset = 0;
        EXPECT_EQ(DecodeRle<int32_t>(encoded, offset, 3, true), input);
    }
    {
        std::vector<float> input = {1.0f, 1.0f, 2.0f};
        std::vector<char> expected = {
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // run_count=2
            '\x00','\x00','\x80','\x3F',                             // value=1.0f
            '\x00','\x00','\x00','\x40',                             // value=2.0f
            '\x02', '\x06',                                          // bits=2, packed [2,1]
        };
        auto encoded = EncodeRle<float>(input, true);
        EXPECT_EQ(encoded, expected);
        size_t offset = 0;
        EXPECT_EQ(DecodeRle<float>(encoded, offset, 3, true), input);
    }
    {
        std::vector<std::string> input = {"ab", "ab", "cd"};
        std::vector<char> expected = {
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00',         // run_count=2
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','a','b', // value="ab"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','c','d', // value="cd"
            '\x02', '\x06',                                                  // bits=2, packed [2,1]
        };
        auto encoded = EncodeRle<std::string>(input, true);
        EXPECT_EQ(encoded, expected);
        size_t offset = 0;
        EXPECT_EQ(DecodeRle<std::string>(encoded, offset, 3, true), input);
    }
}

TEST(Dict, Without_bitpack) {
    {
        std::vector<int32_t> input = {3, 1, 3, 2};
        std::vector<char> expected = {
            '\x03','\x00','\x00','\x00','\x00','\x00','\x00','\x00',  // dict_size=3
            '\x03','\x00','\x00','\x00',                              // dict[0]=3
            '\x01','\x00','\x00','\x00',                              // dict[1]=1
            '\x02','\x00','\x00','\x00',                              // dict[2]=2
            '\x00','\x00','\x00','\x00',                              // code[0]=0
            '\x01','\x00','\x00','\x00',                              // code[1]=1
            '\x00','\x00','\x00','\x00',                              // code[2]=0
            '\x02','\x00','\x00','\x00',                              // code[3]=2
        };
        auto encoded = EncodeDict<int32_t>(input, false);
        EXPECT_EQ(encoded, expected);
        size_t offset = 0;
        EXPECT_EQ(DecodeDict<int32_t>(encoded, offset, 4, false), input);
    }
    {
        std::vector<float> input = {1.0f, 1.0f, 2.0f};
        std::vector<char> expected = {
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00',  // dict_size=2
            '\x00','\x00','\x80','\x3F',                              // dict[0]=1.0f
            '\x00','\x00','\x00','\x40',                              // dict[1]=2.0f
            '\x00','\x00','\x00','\x00',                              // code[0]=0
            '\x00','\x00','\x00','\x00',                              // code[1]=0
            '\x01','\x00','\x00','\x00',                              // code[2]=1
        };
        auto encoded = EncodeDict<float>(input, false);
        EXPECT_EQ(encoded, expected);
        size_t offset = 0;
        EXPECT_EQ(DecodeDict<float>(encoded, offset, 3, false), input);
    }
    {
        std::vector<std::string> input = {"x", "y", "x"};
        std::vector<char> expected = {
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00',     // dict_size=2
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00','x', // dict[0]="x"
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00','y', // dict[1]="y"
            '\x00','\x00','\x00','\x00',                                 // code[0]=0
            '\x01','\x00','\x00','\x00',                                 // code[1]=1
            '\x00','\x00','\x00','\x00',                                 // code[2]=0
        };
        auto encoded = EncodeDict<std::string>(input, false);
        EXPECT_EQ(encoded, expected);
        size_t offset = 0;
        EXPECT_EQ(DecodeDict<std::string>(encoded, offset, 3, false), input);
    }
}

TEST(Dict, With_bitpack) {
    {
        std::vector<int32_t> input = {3, 1, 3, 2};
        std::vector<char> expected = {
            '\x03','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // dict_size=3
            '\x03','\x00','\x00','\x00',                              // dict[0]=3
            '\x01','\x00','\x00','\x00',                              // dict[1]=1
            '\x02','\x00','\x00','\x00',                              // dict[2]=2
            '\x02', '\x84',                                           // bits=2, packed [0,1,0,2]
        };
        auto encoded = EncodeDict<int32_t>(input, true);
        EXPECT_EQ(encoded, expected);
        size_t offset = 0;
        EXPECT_EQ(DecodeDict<int32_t>(encoded, offset, 4, true), input);
    }
    {
        std::vector<float> input = {1.0f, 1.0f, 2.0f};
        std::vector<char> expected = {
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00',  // dict_size=2
            '\x00','\x00','\x80','\x3F',                              // dict[0]=1.0f
            '\x00','\x00','\x00','\x40',                              // dict[1]=2.0f
            '\x01', '\x04',                                           // bits=1, packed [0,0,1]
        };
        auto encoded = EncodeDict<float>(input, true);
        EXPECT_EQ(encoded, expected);
        size_t offset = 0;
        EXPECT_EQ(DecodeDict<float>(encoded, offset, 3, true), input);
    }
    {
        std::vector<std::string> input = {"x", "y", "x"};
        std::vector<char> expected = {
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00',      // dict_size=2
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', 'x', // dict[0]="x"
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', 'y', // dict[1]="y"
            '\x01', '\x02',                                               // bits=1, packed [0,1,0]
        };
        auto encoded = EncodeDict<std::string>(input, true);
        EXPECT_EQ(encoded, expected);
        size_t offset = 0;
        EXPECT_EQ(DecodeDict<std::string>(encoded, offset, 3, true), input);
    }
}

TEST(Delta, Without_bitpack) {
    std::vector<int32_t> input = {10, 13, 15};
    std::vector<char> expected = {
        '\x0A','\x00','\x00','\x00', // delta[0]=10
        '\x03','\x00','\x00','\x00', // delta[1]=3
        '\x02','\x00','\x00','\x00', // delta[2]=2
    };
    auto encoded = EncodeDelta<int32_t>(input, false);
    EXPECT_EQ(encoded, expected);
    size_t offset = 0;
    EXPECT_EQ(DecodeDelta<int32_t>(encoded, offset, 3, false), input);
}

TEST(Delta, With_bitpack) {
    std::vector<int32_t> input = {10, 13, 15};
    std::vector<char> expected = {
        '\x0A','\x00','\x00','\x00', // first=10
        '\x02', '\x0B',              // bits=2, packed [3,2]
    };
    auto encoded = EncodeDelta<int32_t>(input, true);
    EXPECT_EQ(encoded, expected);
    size_t offset = 0;
    EXPECT_EQ(DecodeDelta<int32_t>(encoded, offset, 3, true), input);
}

TEST(IntegerVector, All_flag_combinations) {
    std::vector<int32_t> input = {3, 3, 5, 7, 5};
    size_t n = 5;

    {
        size_t flags = 0;
        auto enc = EncodeIntegerVector<int32_t>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x03','\x00','\x00','\x00', // value=3
            '\x03','\x00','\x00','\x00', // value=3
            '\x05','\x00','\x00','\x00', // value=5
            '\x07','\x00','\x00','\x00', // value=7
            '\x05','\x00','\x00','\x00', // value=5
        }));
        EXPECT_EQ(DecodeIntegerVector<int32_t>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagBitPackInt;
        auto enc = EncodeIntegerVector<int32_t>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x03',         // bits=3
            '\x5B', '\x5F', // packed [3,3,5,7,5]
        }));
        EXPECT_EQ(DecodeIntegerVector<int32_t>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagRleInt;
        auto enc = EncodeIntegerVector<int32_t>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x04','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // run_count=4
            '\x03','\x00','\x00','\x00',                             // value=3
            '\x05','\x00','\x00','\x00',                             // value=5
            '\x07','\x00','\x00','\x00',                             // value=7
            '\x05','\x00','\x00','\x00',                             // value=5
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=2
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=1
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=1
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=1
        }));
        EXPECT_EQ(DecodeIntegerVector<int32_t>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagRleInt | kFlagBitPackInt;
        auto enc = EncodeIntegerVector<int32_t>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x04','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // run_count=4
            '\x03','\x00','\x00','\x00',                             // value=3
            '\x05','\x00','\x00','\x00',                             // value=5
            '\x07','\x00','\x00','\x00',                             // value=7
            '\x05','\x00','\x00','\x00',                             // value=5
            '\x02', '\x56',                                          // bits=2, packed [2,1,1,1]
        }));
        EXPECT_EQ(DecodeIntegerVector<int32_t>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagDeltaInt | kFlagBitPackInt;
        auto enc = EncodeIntegerVector<int32_t>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x03','\x00','\x00','\x00',                              // first=3
            '\x20',                                                   // bits=32
            '\x00','\x00','\x00','\x00',                              // diff[0]=0
            '\x02','\x00','\x00','\x00',                              // diff[1]=2
            '\x02','\x00','\x00','\x00',                              // diff[2]=2
            '\xFE','\xFF','\xFF','\xFF',                              // diff[3]=-2 (uint32=0xFFFFFFFE)
        }));
        EXPECT_EQ(DecodeIntegerVector<int32_t>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagDeltaInt | kFlagRleInt;
        auto enc = EncodeIntegerVector<int32_t>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x04','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // run_count=4
            '\x03','\x00','\x00','\x00',                             // value=3
            '\x00','\x00','\x00','\x00',                             // value=0
            '\x02','\x00','\x00','\x00',                             // value=2
            '\xFE','\xFF','\xFF','\xFF',                             // value=-2
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=1
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=1
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=2
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=1
        }));
        EXPECT_EQ(DecodeIntegerVector<int32_t>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagDeltaInt | kFlagRleInt | kFlagBitPackInt;
        auto enc = EncodeIntegerVector<int32_t>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x04','\x00','\x00','\x00','\x00','\x00','\x00','\x00',  // run_count=4
            '\x03','\x00','\x00','\x00',                              // value=3
            '\x00','\x00','\x00','\x00',                              // value=0
            '\x02','\x00','\x00','\x00',                              // value=2
            '\xFE','\xFF','\xFF','\xFF',                              // value=-2
            '\x02', '\x65',                                           // bits=2, packed [1,1,2,1]
        }));
        EXPECT_EQ(DecodeIntegerVector<int32_t>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagDictInt;
        auto enc = EncodeIntegerVector<int32_t>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x03','\x00','\x00','\x00','\x00','\x00','\x00','\x00',  // dict_size=3
            '\x03','\x00','\x00','\x00',                              // dict[0]=3
            '\x05','\x00','\x00','\x00',                              // dict[1]=5
            '\x07','\x00','\x00','\x00',                              // dict[2]=7
            '\x00','\x00','\x00','\x00',                              // code[0]=0
            '\x00','\x00','\x00','\x00',                              // code[1]=0
            '\x01','\x00','\x00','\x00',                              // code[2]=1
            '\x02','\x00','\x00','\x00',                              // code[3]=2
            '\x01','\x00','\x00','\x00',                              // code[4]=1
        }));
        EXPECT_EQ(DecodeIntegerVector<int32_t>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagDictInt | kFlagBitPackInt;
        auto enc = EncodeIntegerVector<int32_t>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x03','\x00','\x00','\x00','\x00','\x00','\x00','\x00',  // dict_size=3
            '\x03','\x00','\x00','\x00',                              // dict[0]=3
            '\x05','\x00','\x00','\x00',                              // dict[1]=5
            '\x07','\x00','\x00','\x00',                              // dict[2]=7
            '\x02', '\x90', '\x01',                                   // bits=2, packed [0,0,1,2,1]
        }));
        EXPECT_EQ(DecodeIntegerVector<int32_t>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagDeltaInt | kFlagDictInt;
        auto enc = EncodeIntegerVector<int32_t>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x04','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // dict_size=4
            '\x03','\x00','\x00','\x00',                              // dict[0]=3
            '\x00','\x00','\x00','\x00',                              // dict[1]=0
            '\x02','\x00','\x00','\x00',                              // dict[2]=2
            '\xFE','\xFF','\xFF','\xFF',                              // dict[3]=-2
            '\x00','\x00','\x00','\x00',                              // code[0]=0
            '\x01','\x00','\x00','\x00',                              // code[1]=1
            '\x02','\x00','\x00','\x00',                              // code[2]=2
            '\x02','\x00','\x00','\x00',                              // code[3]=2
            '\x03','\x00','\x00','\x00',                              // code[4]=3
        }));
        EXPECT_EQ(DecodeIntegerVector<int32_t>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagDeltaInt | kFlagDictInt | kFlagBitPackInt;
        auto enc = EncodeIntegerVector<int32_t>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x04','\x00','\x00','\x00','\x00','\x00','\x00','\x00',  // dict_size=4
            '\x03','\x00','\x00','\x00',                              // dict[0]=3
            '\x00','\x00','\x00','\x00',                              // dict[1]=0
            '\x02','\x00','\x00','\x00',                              // dict[2]=2
            '\xFE','\xFF','\xFF','\xFF',                              // dict[3]=-2
            '\x02', '\xA4', '\x03',                                   // bits=2, packed [0,1,2,2,3]
        }));
        EXPECT_EQ(DecodeIntegerVector<int32_t>(enc, n, flags), input);
    }
}

TEST(FloatVector, All_flag_combinations) {
    std::vector<float> input = {1.0f, 1.0f, 2.0f, 3.0f, 2.0f};
    size_t n = 5;

    {
        size_t flags = 0;
        auto enc = EncodeFloatVector<float>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x00','\x00','\x80','\x3F', // value=1.0f
            '\x00','\x00','\x80','\x3F', // value=1.0f
            '\x00','\x00','\x00','\x40', // value=2.0f
            '\x00','\x00','\x40','\x40', // value=3.0f
            '\x00','\x00','\x00','\x40', // value=2.0f
        }));
        EXPECT_EQ(DecodeFloatVector<float>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagRleFloat;
        auto enc = EncodeFloatVector<float>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x04','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // run_count=4
            '\x00','\x00','\x80','\x3F',                             // value=1.0f
            '\x00','\x00','\x00','\x40',                             // value=2.0f
            '\x00','\x00','\x40','\x40',                             // value=3.0f
            '\x00','\x00','\x00','\x40',                             // value=2.0f
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=2
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=1
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=1
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // count=1
        }));
        EXPECT_EQ(DecodeFloatVector<float>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagRleFloat | kFlagBitPackFloat;
        auto enc = EncodeFloatVector<float>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x04','\x00','\x00','\x00','\x00','\x00','\x00','\x00',  // run_count=4
            '\x00','\x00','\x80','\x3F',                              // value=1.0f
            '\x00','\x00','\x00','\x40',                              // value=2.0f
            '\x00','\x00','\x40','\x40',                              // value=3.0f
            '\x00','\x00','\x00','\x40',                              // value=2.0f
            '\x02', '\x56',                                           // bits=2, packed [2,1,1,1]
        }));
        EXPECT_EQ(DecodeFloatVector<float>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagDictFloat;
        auto enc = EncodeFloatVector<float>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x03','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // dict_size=3
            '\x00','\x00','\x80','\x3F',                             // dict[0]=1.0f
            '\x00','\x00','\x00','\x40',                             // dict[1]=2.0f
            '\x00','\x00','\x40','\x40',                             // dict[2]=3.0f
            '\x00','\x00','\x00','\x00',                             // code[0]=0
            '\x00','\x00','\x00','\x00',                             // code[1]=0
            '\x01','\x00','\x00','\x00',                             // code[2]=1
            '\x02','\x00','\x00','\x00',                             // code[3]=2
            '\x01','\x00','\x00','\x00',                             // code[4]=1
        }));
        EXPECT_EQ(DecodeFloatVector<float>(enc, n, flags), input);
    }
    {
        size_t flags = kFlagDictFloat | kFlagBitPackFloat;
        auto enc = EncodeFloatVector<float>(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x03','\x00','\x00','\x00','\x00','\x00','\x00','\x00', // dict_size=3
            '\x00','\x00','\x80','\x3F',                             // dict[0]=1.0f
            '\x00','\x00','\x00','\x40',                             // dict[1]=2.0f
            '\x00','\x00','\x40','\x40',                             // dict[2]=3.0f
            '\x02', '\x90', '\x01',                                  // bits=2, packed [0,0,1,2,1]
        }));
        EXPECT_EQ(DecodeFloatVector<float>(enc, n, flags), input);
    }
}

TEST(StringVector, All_flag_combinations) {
    std::vector<std::string> input = {"ab", "ab", "cd", "ef", "cd"};
    size_t n = 5;

    {
        size_t flags = 0;
        auto enc = EncodeStringVector(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','a','b', // value="ab"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','a','b', // value="ab"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','c','d', // value="cd"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','e','f', // value="ef"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','c','d', // value="cd"
        }));
        EXPECT_EQ(DecodeStringVector(enc, n, flags), input);
    }
    {
        size_t flags = kFlagRleStr;
        auto enc = EncodeStringVector(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x04','\x00','\x00','\x00','\x00','\x00','\x00','\x00',          // run_count=4
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','a','b',  // value="ab"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','c','d',  // value="cd"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','e','f',  // value="ef"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','c','d',  // value="cd"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00',          // count=2
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00',          // count=1
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00',          // count=1
            '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x00',          // count=1
        }));
        EXPECT_EQ(DecodeStringVector(enc, n, flags), input);
    }
    {
        size_t flags = kFlagRleStr | kFlagBitPackStr;
        auto enc = EncodeStringVector(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x04','\x00','\x00','\x00','\x00','\x00','\x00','\x00',          // run_count=4
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','a','b',  // value="ab"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','c','d',  // value="cd"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','e','f',  // value="ef"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','c','d',  // value="cd"
            '\x02', '\x56',                                                   // bits=2, packed [2,1,1,1]
        }));
        EXPECT_EQ(DecodeStringVector(enc, n, flags), input);
    }
    {
        size_t flags = kFlagDictStr;
        auto enc = EncodeStringVector(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x03','\x00','\x00','\x00','\x00','\x00','\x00','\x00',          // dict_size=3
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','a','b',  // dict[0]="ab"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','c','d',  // dict[1]="cd"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','e','f',  // dict[2]="ef"
            '\x00','\x00','\x00','\x00',                                      // code[0]=0
            '\x00','\x00','\x00','\x00',                                      // code[1]=0
            '\x01','\x00','\x00','\x00',                                      // code[2]=1
            '\x02','\x00','\x00','\x00',                                      // code[3]=2
            '\x01','\x00','\x00','\x00',                                      // code[4]=1
        }));
        EXPECT_EQ(DecodeStringVector(enc, n, flags), input);
    }
    {
        size_t flags = kFlagDictStr | kFlagBitPackStr;
        auto enc = EncodeStringVector(input, flags);
        EXPECT_EQ(enc, (std::vector<char>{
            '\x03','\x00','\x00','\x00','\x00','\x00','\x00','\x00',          // dict_size=3
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','a','b',  // dict[0]="ab"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','c','d',  // dict[1]="cd"
            '\x02','\x00','\x00','\x00','\x00','\x00','\x00','\x00','e','f',  // dict[2]="ef"
            '\x02', '\x90', '\x01',                                           // bits=2, packed [0,0,1,2,1]
        }));
        EXPECT_EQ(DecodeStringVector(enc, n, flags), input);
    }
}
