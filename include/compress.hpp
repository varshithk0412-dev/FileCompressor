#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include "lzss.hpp"
#include "huffman.hpp"

namespace compress_project {
namespace compressor {

    // --- Bit Packing Utility ---
    // Standard file streams (std::ofstream) can only write whole bytes (8 bits).
    // This class acts as a waiting room. It collects incoming variable-length 
    // bit codes until it has a full 8 bits, and then writes them to the file.
    class BitWriter {
    public:
        // Constructor takes a reference to an open file stream
        explicit BitWriter(std::ofstream& out_stream);
        
        // The destructor automatically flushes any leftover bits when we are done
        ~BitWriter();

        /**
         * @brief Packs variable-length bits into the internal buffer.
         * @param value The actual bit pattern (e.g., from our huffman::BitCode)
         * @param num_bits How many bits of that value are valid
         */
        void write_bits(uint32_t value, uint8_t num_bits);

        /**
         * @brief Forces any partially filled buffer to be written to the file.
         */
        void flush();

        /**
         * @brief Tells us how many bits in the final byte are actually our data.
         * This is crucial for the file header so the decompressor knows when to stop.
         */
        uint8_t get_valid_bits_in_last_byte() const;

    private:
        std::ofstream& out;          // The actual file stream
        uint8_t bit_buffer;          // The 8-bit waiting room
        uint8_t bits_in_buffer;      // How many bits are currently in the waiting room
        uint8_t valid_bits_last_byte;// Tracks the final trailing bits
    };

    // --- Core Orchestrator ---

    /**
     * @brief The main forward pipeline: File -> LZSS -> Huffman -> BitWriter -> File
     * @param input_path The path to the original uncompressed file.
     * @param output_path The path to save the compressed file (e.g., compressed.bin).
     */
    void compress_file(const std::string& input_path, const std::string& output_path);

} // namespace compressor
} // namespace compress_project