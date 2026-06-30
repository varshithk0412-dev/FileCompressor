#pragma once

#include <cstdint>
#include <vector>

using namespace std;

namespace compress_project {
namespace lzss {

    constexpr uint16_t WINDOW_SIZE = 4096; 
    constexpr uint16_t MAX_MATCH_LENGTH = 255; 
    constexpr uint16_t MIN_MATCH_LENGTH = 3;

    struct Token {
        bool is_match;      
        uint8_t literal;    
        uint16_t offset;    
        uint16_t length;    
        
        static Token make_literal(uint8_t c) {
            return {false, c, 0, 0};
        }
        
        static Token make_match(uint16_t off, uint16_t len) {
            return {true, 0, off, len};
        }
    };


    vector<Token> encode(const vector<uint8_t>& input);

    vector<uint8_t> decode(const vector<Token>& tokens);

} 
} 