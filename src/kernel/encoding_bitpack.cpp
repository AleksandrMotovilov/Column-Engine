#include "src/kernel/encoding_bitpack.h"

int BitsRequired(uint64_t max_value) {
    if (max_value == 0) {
        return 1;
    }
    int bits = 1;
    while (bits < 64 && (static_cast<uint64_t>(1) << bits) <= max_value) {
        bits++;
    }
    return bits;
}
