#include "huffman.hpp"
#include <queue>

// It is safe to use this inside the .cpp file
using namespace std;

namespace compress_project {
namespace huffman {

    vector<uint64_t> calculate_frequencies(const vector<uint8_t>& data) {
        // Initialize an array of ALPHABET_SIZE (257) with all zeros
        vector<uint64_t> frequencies(ALPHABET_SIZE, 0);

        // Count every byte in the input data
        for (uint8_t byte : data) {
            frequencies[byte]++;
        }

        // We MUST manually add a frequency for our End-of-Stream marker.
        // The decompressor needs this to know exactly when to stop reading bits.
        frequencies[EOS_SYMBOL] = 1;

        return frequencies;
    }

    shared_ptr<HuffmanNode> build_tree(const vector<uint64_t>& frequencies) {
        // Create a min-heap priority queue using our custom NodeComparator
        priority_queue<
            shared_ptr<HuffmanNode>, 
            vector<shared_ptr<HuffmanNode>>, 
            NodeComparator
        > pq;

        // Step 1: Create a leaf node for every symbol that actually appeared
        for (uint16_t i = 0; i < ALPHABET_SIZE; ++i) {
            if (frequencies[i] > 0) {
                pq.push(make_shared<HuffmanNode>(i, frequencies[i]));
            }
        }

        // Step 2: Merge the lowest frequency nodes until only 1 root node remains
        while (pq.size() > 1) {
            // Grab the two nodes with the lowest frequencies
            shared_ptr<HuffmanNode> left = pq.top();
            pq.pop();
            
            shared_ptr<HuffmanNode> right = pq.top();
            pq.pop();

            // Create a new internal parent node combining their frequencies
            uint64_t combined_freq = left->frequency + right->frequency;
            auto parent = make_shared<HuffmanNode>(combined_freq, left, right);

            // Push the new parent back into the queue
            pq.push(parent);
        }

        // The final remaining node is the root of our complete Huffman Tree
        return pq.top();
    }

    // --- Helper function for generating codes ---
    // We use recursion to walk down the tree. Going left adds a '0', right adds a '1'.
    void generate_codes_recursive(const shared_ptr<HuffmanNode>& node, 
                                  uint32_t current_code, 
                                  uint8_t current_length, 
                                  CodeTable& table) {
        // Base case: If we hit a null node, just return
        if (!node) return;

        // If both children are null, we are at a leaf node (an actual symbol)
        if (!node->left && !node->right) {
            table[node->symbol] = {current_code, current_length};
            return;
        }

        // Recursive case: Traverse left (shift code left and add 0)
        generate_codes_recursive(node->left, current_code << 1, current_length + 1, table);
        
        // Recursive case: Traverse right (shift code left and add 1 using bitwise OR)
        generate_codes_recursive(node->right, (current_code << 1) | 1, current_length + 1, table);
    }

    CodeTable generate_codes(const shared_ptr<HuffmanNode>& root) {
        CodeTable table;
        if (!root) return table;

        // EDGE CASE FIX: If the tree is only a single node (e.g., only the EOS symbol exists),
        // we must force it to have a 1-bit code so the bit-writer actually writes something.
        if (!root->left && !root->right) {
            table[root->symbol] = {0, 1}; // Assign a 1-bit code of '0'
            return table;
        }

        // Standard recursive traversal
        generate_codes_recursive(root, 0, 0, table);
        return table;
    }

} // namespace huffman
} // namespace compress_project