#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

using namespace std;

namespace compress_project {
namespace huffman {

    // --- Configuration Constants ---
    // 0-255 are raw byte literals. 256 can be used as a special End-of-Stream (EOS) marker.
    constexpr uint16_t ALPHABET_SIZE = 257; 
    constexpr uint16_t EOS_SYMBOL = 256;

    // --- Data Structures ---

    // A node structure to build our binary Huffman Tree
    struct HuffmanNode {
        uint16_t symbol; // The raw character or length marker
        uint64_t frequency; // How often this symbol appeared
        
        shared_ptr<HuffmanNode> left;
        shared_ptr<HuffmanNode> right;

        // Constructor for leaf nodes (actual symbols)
        HuffmanNode(uint16_t sym, uint64_t freq) 
            : symbol(sym), frequency(freq), left(nullptr), right(nullptr) {}

        // Constructor for internal nodes (combined frequencies)
        HuffmanNode(uint64_t freq, shared_ptr<HuffmanNode> l, shared_ptr<HuffmanNode> r) 
            : symbol(0), frequency(freq), left(l), right(r) {}
    };

    // For comparing nodes inside the priority queue (min-heap)
    struct NodeComparator {
        bool operator()(const shared_ptr<HuffmanNode>& a, const shared_ptr<HuffmanNode>& b) {
            return a->frequency > b->frequency; // Higher frequency gets lower priority (min-heap)
        }
    };

    // A structure to store our final Huffman code tracking variable bit-length
    struct BitCode {
        uint32_t code;   // The bit pattern (e.g., 0b1011)
        uint8_t length;  // How many bits wide the pattern is (e.g., 4 bits)
    };

    // A table map linking our symbols to their newly minted variable-length bit paths
    using CodeTable = unordered_map<uint16_t, BitCode>;

    // --- Core Algorithm Signatures ---

    /**
     * @brief Generates a frequency table from a raw block of data.
     * @param data The raw input bytes.
     * @return A vector where the index is the symbol and the value is its count.
     */
    vector<uint64_t> calculate_frequencies(const vector<uint8_t>& data);

    /**
     * @brief Builds a Huffman Tree using a priority queue greedy strategy.
     * @param frequencies The calculated frequency mapping.
     * @return The root node pointer of the constructed Huffman Tree.
     */
    shared_ptr<HuffmanNode> build_tree(const vector<uint64_t>& frequencies);

    /**
     * @brief Traverses the Huffman Tree to generate variable-length bit-codes for each symbol.
     * @param root The root node of the Huffman Tree.
     * @return A map linking symbols to their exact BitCode path.
     */
    CodeTable generate_codes(const shared_ptr<HuffmanNode>& root);

} // namespace huffman
} // namespace compress_project