#include <iostream>
#include <string>
#include <cstring>
#include "compress.hpp"
#include "decompress.hpp"

using namespace std;

// Helper function to print instructions if the user types something wrong
void print_usage(const char* program_name) {
    cout << "====================================================" << endl;
    cout << "   LZSS + Huffman Binary File Compressor (CLI)      " << endl;
    cout << "====================================================" << endl;
    cout << "Usage:" << endl;
    cout << "  Compression:   " << program_name << " -c <input_file> <output_file.bin>" << endl;
    cout << "  Decompression: " << program_name << " -d <compressed_file.bin> <output_file>" << endl;
    cout << "====================================================" << endl;
}

int main(int argc, char* argv[]) {
    // A valid command requires exactly 4 arguments:
    // argv[0] = program name, argv[1] = flag (-c/-d), argv[2] = input path, argv[3] = output path
    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    string flag = argv[1];
    string input_path = argv[2];
    string output_path = argv[3];

    if (flag == "-c" || flag == "--compress") {
        cout << "[Starting Compression Pipeline]" << endl;
        cout << "Source File: " << input_path << endl;
        cout << "Destination: " << output_path << endl;
        
        compress_project::compressor::compress_file(input_path, output_path);
        
    } else if (flag == "-d" || flag == "--decompress") {
        cout << "[Starting Decompression Pipeline]" << endl;
        cout << "Compressed File: " << input_path << endl;
        cout << "Restored Target: " << output_path << endl;
        
        compress_project::decompressor::decompress_file(input_path, output_path);
        
    } else {
        cerr << "Error: Invalid flag '" << flag << "'" << endl;
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}