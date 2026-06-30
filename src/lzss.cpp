#include "lzss.hpp"
#include <algorithm>

// It is perfectly safe and standard to use this in a .cpp file!
using namespace std; 

namespace compress_project {
namespace lzss {

    vector<Token> encode(const vector<uint8_t>& input) {
        vector<Token> tokens;
        size_t pos = 0;
        const size_t input_size = input.size();

        while (pos < input_size) {
            uint16_t best_match_distance = 0;
            uint16_t best_match_length = 0;

            // Calculate how far back we can look (bounded by WINDOW_SIZE and current pos)
            size_t window_start = (pos > WINDOW_SIZE) ? (pos - WINDOW_SIZE) : 0;

            // Search the window for the longest match
            for (size_t search_pos = window_start; search_pos < pos; ++search_pos) {
                uint16_t current_match_length = 0;

                // Count how many bytes match between the search position and the current position
                while (current_match_length < MAX_MATCH_LENGTH && 
                       pos + current_match_length < input_size &&
                       input[search_pos + current_match_length] == input[pos + current_match_length]) {
                    
                    current_match_length++;
                }

                // If we found a longer match than we had before, save it
                if (current_match_length > best_match_length) {
                    best_match_length = current_match_length;
                    
                    // Distance is how far *back* from our current position the match starts
                    best_match_distance = static_cast<uint16_t>(pos - search_pos);
                }
            }

            // Decide whether to output a Literal or a Match
            if (best_match_length >= MIN_MATCH_LENGTH) {
                tokens.push_back(Token::make_match(best_match_distance, best_match_length));
                pos += best_match_length; // Skip ahead by the length of the match
            } else {
                tokens.push_back(Token::make_literal(input[pos]));
                pos++; // Move forward by 1 byte
            }
        }

        return tokens;
    }

    vector<uint8_t> decode(const vector<Token>& tokens) {
        vector<uint8_t> output;

        for (const auto& token : tokens) {
            if (!token.is_match) {
                // It's a Literal: just append the raw byte
                output.push_back(token.literal);
            } else {
                // It's a Match: copy 'length' bytes from 'distance' bytes back in the output
                size_t start_copy_idx = output.size() - token.offset;
                
                for (uint16_t i = 0; i < token.length; ++i) {
                    // We read character by character so it handles overlapping matches correctly!
                    // (e.g., distance 1, length 5 will copy the same character 5 times)
                    output.push_back(output[start_copy_idx + i]);
                }
            }
        }

        return output;
    }

} // namespace lzss
} // namespace compress_project