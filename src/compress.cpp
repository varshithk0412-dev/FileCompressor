#include "compress.hpp"
#include <iostream>

using namespace std;

namespace compress_project {
namespace compressor {

    // --- BitWriter Implementation ---

    BitWriter::BitWriter(ofstream& out_stream) 
        : out(out_stream), bit_buffer(0), bits_in_buffer(0), valid_bits_last_byte(0) {}

    BitWriter::~BitWriter() {
        // Ensure any remaining bits are flushed when the writer is destroyed
        flush(); 
    }

    void BitWriter::write_bits(uint32_t value, uint8_t num_bits) {
        // We write bits from left to right (Most Significant Bit to Least Significant Bit)
        for (int i = num_bits - 1; i >= 0; --i) {
            // Extract the specific bit at position 'i'
            uint8_t bit = (value >> i) & 1;
            
            // Shift our buffer left by 1 and insert the new bit
            bit_buffer = (bit_buffer << 1) | bit;
            bits_in_buffer++;

            // If the waiting room is full (8 bits), write it to the file
            if (bits_in_buffer == 8) {
                out.put(bit_buffer);
                bit_buffer = 0; // Reset the buffer
                bits_in_buffer = 0;
            }
        }
    }

    void BitWriter::flush() {
        if (bits_in_buffer > 0) {
            valid_bits_last_byte = bits_in_buffer;
            
            // Pad the remaining empty slots in the byte with zeros
            bit_buffer <<= (8 - bits_in_buffer);
            out.put(bit_buffer);
            
            bits_in_buffer = 0;
        } else {
            // If the buffer was exactly empty, the last written byte was 100% valid
            valid_bits_last_byte = 8; 
        }
    }

    uint8_t BitWriter::get_valid_bits_in_last_byte() const {
        return valid_bits_last_byte;
    }

    // --- Core Orchestrator Implementation ---

    void compress_file(const string& input_path, const string& output_path) {
        // 1. Read the Entire Input File
        // We open it in binary mode so we don't accidentally corrupt non-text files
        ifstream infile(input_path, ios::binary);
        if (!infile) {
            cerr << "Error: Could not open input file: " << input_path << endl;
            return;
        }
        
        // Read file contents into a uint8_t vector
        vector<uint8_t> input_data((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
        infile.close();

        // 2. LZSS Compression Phase
        cout << "Running LZSS Tokenizer..." << endl;
        vector<lzss::Token> tokens = lzss::encode(input_data);

        // 3. Huffman Phase (For Literals)
        // To keep this robust and simple, we apply Huffman encoding to the Literals,
        // and we write the Match offsets/lengths as fixed-width bits.
        cout << "Building Huffman Tree..." << endl;
        vector<uint8_t> literal_bytes;
        for (const auto& t : tokens) {
            if (!t.is_match) {
                literal_bytes.push_back(t.literal);
            }
        }
        
        vector<uint64_t> frequencies = huffman::calculate_frequencies(literal_bytes);
        auto huffman_tree = huffman::build_tree(frequencies);
        huffman::CodeTable codes = huffman::generate_codes(huffman_tree);

        // 4. Open the Output File
        ofstream outfile(output_path, ios::binary);
        if (!outfile) {
            cerr << "Error: Could not create output file: " << output_path << endl;
            return;
        }

        cout << "Writing Compressed File..." << endl;
        
        // 5. Write the Header
        // Dump the entire frequency table directly into the file. 
        // The decompressor will read exactly 257 uint64_t values to rebuild the tree.
        outfile.write(reinterpret_cast<const char*>(frequencies.data()), frequencies.size() * sizeof(uint64_t));
        
        // We need to write 'valid_bits_last_byte', but we don't know it yet!
        // We save the current file position, write a dummy '0', and will overwrite it at the end.
        streampos valid_bits_pos = outfile.tellp();
        uint8_t valid_bits_placeholder = 0;
        outfile.write(reinterpret_cast<const char*>(&valid_bits_placeholder), sizeof(uint8_t));

        // 6. Bit-Packing Phase
        // We use a block { } here so the BitWriter's destructor is automatically 
        // called (and flushes) before we seek back to fix the header.
        {
            BitWriter writer(outfile);

            for (const auto& t : tokens) {
                if (!t.is_match) {
                    // Flag 0 = Literal
                    writer.write_bits(0, 1);
                    
                    // Lookup and write the Huffman code
                    huffman::BitCode code = codes[t.literal];
                    writer.write_bits(code.code, code.length);
                } else {
                    // Flag 1 = Match
                    writer.write_bits(1, 1);
                    
                    // Write offset (16 bits) and length (8 bits) as raw fixed bits
                    writer.write_bits(t.offset, 16);
                    writer.write_bits(t.length, 8); 
                }
            }

            // 7. Write the End-of-Stream Token
            // We treat the EOS marker as a literal flag (0) so the decompressor knows to look at the tree
            writer.write_bits(0, 1); 
            huffman::BitCode eos_code = codes[huffman::EOS_SYMBOL];
            writer.write_bits(eos_code.code, eos_code.length);

            // Force the final padded byte to write
            writer.flush(); 
            
            // 8. Update the Header
            valid_bits_placeholder = writer.get_valid_bits_in_last_byte();
            outfile.seekp(valid_bits_pos); // Jump back to the placeholder spot
            outfile.write(reinterpret_cast<const char*>(&valid_bits_placeholder), sizeof(uint8_t));
        } // BitWriter goes out of scope and is destroyed here
        
        outfile.close();
        cout << "Compression Complete!" << endl;
    }

} // namespace compressor
} // namespace compress_project