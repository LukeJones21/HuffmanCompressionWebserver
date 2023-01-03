#include <iostream>
#include <fstream>
#include "huffman.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <zapfile> <outputfile>" << std::endl;
    exit(1);
  }
  std::ifstream input_file(argv[1], std::ios::in | std::ios::binary);
  if (!input_file.is_open()) {
    std::cerr << "Error: cannot open zap file " << argv[1] << std::endl;
    exit(1);
  }
  std::ofstream output_file(argv[2], std::ios::out 
                    | std::ios::trunc | std::ios::binary);
  Huffman::Decompress(input_file, output_file);
  std::cout << "Decompressed zap file " << argv[1]
            << " into output file " << argv[2] << std::endl;
}
