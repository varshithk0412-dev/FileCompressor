## FileCompressor

## Project Structure

```text
File compressor/
│
├── include/
│   ├── compress.hpp       # Compression pipeline interface
│   ├── decompress.hpp     # Decompression pipeline interface
│   ├── huffman.hpp        # Huffman coding declarations
│   └── lzss.hpp           # LZSS compression declarations
│
└── src/
    ├── compress.cpp       # Compression workflow implementation
    ├── decompress.cpp     # Decompression workflow implementation
    ├── huffman.cpp        # Huffman tree construction and encoding
    ├── lzss.cpp           # LZSS token generation and reconstruction
    └── main.cpp           # Entry point and command-line interface
```

## Module Responsibilities

### lzss.cpp / lzss.hpp

Implements the LZSS algorithm.

Responsibilities:

* Sliding window search
* Match detection
* Generation of literal and match tokens
* Reconstruction during decompression

### huffman.cpp / huffman.hpp

Implements Huffman Coding.

Responsibilities:

* Frequency analysis
* Huffman tree construction
* Prefix-code generation
* Encoding and decoding of token streams

### compress.cpp / compress.hpp

Coordinates the compression pipeline.

Workflow:

```text
Input File
     |
     v
LZSS Compression
     |
     v
Token Stream
     |
     v
Huffman Encoding
     |
     v
Compressed Output
```

### decompress.cpp / decompress.hpp

Coordinates the decompression pipeline.

Workflow:

```text
Compressed File
       |
       v
Huffman Decoding
       |
       v
LZSS Tokens
       |
       v
LZSS Reconstruction
       |
       v
Original File
```

### main.cpp

Provides the entry point of the application and handles user interaction for compression and decompression operations.
