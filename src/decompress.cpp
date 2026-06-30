#include "decompress.hpp"
#include <iostream>

using namespace std;

namespace compress_project {
namespace decompressor {

    // --- BitReader Implementation ---

    BitReader::BitReader(ifstream& in_stream, uint8_t valid_bits_last_byte) 
        : in(in_stream), bit_buffer(0), bits_remaining(0), 
          valid_bits_last(valid_bits_last_byte), is_eof(false) {
        
        if (in.peek() == EOF) {
            is_eof = true;
        }
    }

    bool BitReader::has_more_bits() {
        if (is_eof && bits_remaining == 0) {
            return false;
        }
        
        if (in.peek() == EOF) {
            // Compare remaining bits against the trailing valid bits rule
            if (bits_remaining <= (8 - valid_bits_last)) {
                return false;
            }
        }
        
        return true;
    }

    uint8_t BitReader::read_bit() {
        if (bits_remaining == 0) {
            char next_byte;
            if (in.get(next_byte)) {
                bit_buffer = static_cast<uint8_t>(next_byte);
                bits_remaining = 8;
            } else {
                is_eof = true;
                return 0; // Failsafe
            }
        }

        bits_remaining--;
        return (bit_buffer >> bits_remaining) & 1;
    }

    uint32_t BitReader::read_bits(uint8_t num_bits) {
        uint32_t result = 0;
        for (int i = 0; i < num_bits; ++i) {
            result = (result << 1) | read_bit();
        }
        return result;
    }

    // --- Core Orchestrator Implementation ---

    void decompress_file(const string& input_path, const string& output_path) {
        ifstream infile(input_path, ios::binary);
        if (!infile) {
            cerr << "Error: Could not open input file: " << input_path << endl;
            return;
        }

        cout << "Reading Header..." << endl;

        // Read the 2056-byte frequency table
        vector<uint64_t> frequencies(huffman::ALPHABET_SIZE, 0);
        infile.read(reinterpret_cast<char*>(frequencies.data()), frequencies.size() * sizeof(uint64_t));

        // Read the single trailing valid bits rule
        uint8_t valid_bits_last_byte = 0;
        infile.read(reinterpret_cast<char*>(&valid_bits_last_byte), sizeof(uint8_t));

        cout << "Rebuilding Huffman Tree..." << endl;
        auto huffman_tree_root = huffman::build_tree(frequencies);

        cout << "Decoding Bitstream into LZSS Tokens..." << endl;
        vector<lzss::Token> tokens;
        BitReader reader(infile, valid_bits_last_byte);

        while (reader.has_more_bits()) {
            uint8_t flag = reader.read_bit();

            if (flag == 0) {
                // Literal Flag: Traverse the Huffman Tree
                auto current_node = huffman_tree_root;
                
                // Edge case: If the tree is only a single node, consume the forced 1-bit padding
                if (!current_node->left && !current_node->right) {
                    reader.read_bit(); 
                } else {
                    // Standard traversal down the branches
                    while (current_node->left || current_node->right) {
                        uint8_t bit = reader.read_bit();
                        if (bit == 0) {
                            current_node = current_node->left;
                        } else {
                            current_node = current_node->right;
                        }
                    }
                }

                // Hard stop if we hit the artificial EOS marker
                if (current_node->symbol == huffman::EOS_SYMBOL) {
                    break;
                }

                tokens.push_back(lzss::Token::make_literal(static_cast<uint8_t>(current_node->symbol)));

            } else {
                // Match Flag: Bypass tree, read fixed-width offset and length
                uint16_t offset = static_cast<uint16_t>(reader.read_bits(16));
                uint16_t length = static_cast<uint16_t>(reader.read_bits(8));
                
                tokens.push_back(lzss::Token::make_match(offset, length));
            }
        }
        infile.close();

        cout << "Reconstructing Original File Data..." << endl;
        vector<uint8_t> original_data = lzss::decode(tokens);

        ofstream outfile(output_path, ios::binary);
        if (!outfile) {
            cerr << "Error: Could not create output file: " << output_path << endl;
            return;
        }
        
        outfile.write(reinterpret_cast<const char*>(original_data.data()), original_data.size());
        outfile.close();

        cout << "Decompression Complete! File successfully restored." << endl;
    }

} // namespace decompressor
} // namespace compress_project