#include "src/compression/encoding_lz4.h"

std::vector<char> Lz4Compress(const std::vector<char>& input) {
    int uncompressed_size = static_cast<int>(input.size());
    int max_compressed_size = LZ4_compressBound(uncompressed_size);
    std::vector<char> output(2 * sizeof(int) + max_compressed_size);
    std::memcpy(output.data(), &uncompressed_size, sizeof(int));
    int compressed_size = LZ4_compress_default(
        input.data(),
        output.data() + 2 * sizeof(int),
        uncompressed_size,
        max_compressed_size
    );
    if (compressed_size == 0) {
        throw std::runtime_error("LZ4_compress_default failed");
    }
    std::memcpy(output.data() + sizeof(int), &compressed_size, sizeof(int));
    output.resize(2 * sizeof(int) + compressed_size);
    return output;
}

std::vector<char> Lz4Decompress(const std::vector<char>& input) {
    int uncompressed_size;
    int compressed_size;
    std::memcpy(&uncompressed_size, input.data(), sizeof(int));
    std::memcpy(&compressed_size, input.data() + sizeof(int), sizeof(int));
    std::vector<char> output(uncompressed_size);
    int status = LZ4_decompress_safe(
        input.data() + 2 * sizeof(int),
        output.data(),
        compressed_size,
        uncompressed_size
    );
    if (status < uncompressed_size) {
        throw std::runtime_error("LZ4_decompress_safe failed");
    }
    return output;
}
