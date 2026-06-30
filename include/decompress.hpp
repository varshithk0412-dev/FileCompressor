#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include "lzss.hpp"
#include "huffman.hpp"

namespace compress_project {
namespace decompressor {

    // --- Bit Unpacking Utility ---
    // Standard file streams (std::ifstream) can only read whole bytes (8 bits).
    // This class reads full bytes from the disk and dishes them out bit-by-bit.
    class BitReader {
    public:
        /**
         * @brief Constructor for the BitReader.
         * @param in_stream Reference to an open input file stream.
         * @param valid_bits_last_byte How many bits in the absolute final byte of the file 
         * are valid data (read from the file header).
         */
        BitReader(std::ifstream& in_stream, uint8_t valid_bits_last_byte);

        /**
         * @brief Reads a single bit from the file.
         * @return 0 or 1 depending on the bit read.
         */
        uint8_t read_bit();

        /**
         * @brief Reads multiple bits at once and combines them into an integer.
         * Useful for extracting fixed-width 8-bit lengths and 16-bit offsets.
         * @param num_bits Number of bits to read (maximum 32).
         * @return The combined integer value of the read bits.
         */
        uint32_t read_bits(uint8_t num_bits);

        /**
         * @brief Checks if we have hit the technical end of our bitstream.
         * Uses file boundaries and the valid bits count to prevent reading garbage padding.
         */
        bool has_more_bits();

    private:
        std::ifstream& in;           // The actual file stream
        uint8_t bit_buffer;          // Holds the current byte we are parsing
        uint8_t bits_remaining;      // How many unread bits are left in 'bit_buffer'
        uint8_t valid_bits_last;     // Copy of the trailing valid bits rule
        bool is_eof;                 // Tracks if the underlying file is empty
    };

    // --- Core Orchestrator ---

    /**
     * @brief The main reverse pipeline: Read Header -> Rebuild Tree -> BitReader -> LZSS Tokens -> Decode -> File
     * @param input_path The path to the compressed binary file (e.g., compressed.bin).
     * @param output_path The path where the original restored file will be saved.
     */
    void decompress_file(const std::string& input_path, const std::string& output_path);

} // namespace decompressor
} // namespace compress_project