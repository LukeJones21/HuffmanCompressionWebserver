#ifndef HUFFMAN_H_
#define HUFFMAN_H_

#include <array>
#include <cstddef>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

#include "bstream.h"
#include "pqueue.h"

class HuffmanNode {
public:
    explicit HuffmanNode(char ch, size_t freq,
                         HuffmanNode *left = nullptr,
                         HuffmanNode *right = nullptr)
            : ch_(ch), freq_(freq), left_(left), right_(right) { }


    bool IsLeaf() {
        // Node is a leaf if it doesn't have any children
        return left_ == nullptr && right_ == nullptr;
    }

    bool operator < (const HuffmanNode &n) const {
        // In case of equality, make it deterministic based on character
        if (freq_ == n.freq_)
            return ch_ < n.ch_;
        // Otherwise compare frequencies
        return freq_ < n.freq_;
    }

    size_t freq() { return freq_; }
    size_t data() { return ch_; }
    HuffmanNode* left() { return left_; }
    HuffmanNode* right() { return right_; }

private:
    char ch_;
    size_t freq_;
    HuffmanNode *left_, *right_;
};

class Huffman {
  public:
    static void Compress(std::ifstream &ifs, std::ofstream &ofs);

    static void Decompress(std::ifstream &ifs, std::ofstream &ofs);

  private:
    // Helper methods...
    static void PreorderRecur(HuffmanNode *n, 
                              BinaryOutputStream& bos, 
                              std::string bits, std::map<char, 
                              std::string>& code_table);

    static void BuildCodeTable(BinaryInputStream& bis, 
                               std::map<std::string, 
                               char>& code_table);
    static void BuildCodeTableHelper(HuffmanNode* node, 
                                BinaryInputStream& bis,
                                std::string bits, 
                                std::map<std::string, 
                                char>& code_table);
};

void Huffman::Compress(std::ifstream &ifs, std::ofstream &ofs) {
    // array of all possible ASCII values
    int chars[128] = {0};
    PQueue<HuffmanNode> pq;
    char c;
    // count frequency of every char in input file and put into ASCII array
    while (ifs.get(c)) {
        chars[c + 0]++;
    }
    // create min priority queue, ordered by frequency
    for (int i = 0; i < 128; i++) {
        if (chars[i] != 0)
            pq.Push(HuffmanNode(i, chars[i]));
    }
    if (pq.Size() == 0)
        return;
    // Create Huffman Tree
    while (pq.Size() > 1) {
        // Create Huffman Node out of the top of the priority queue
        HuffmanNode *n1 = new HuffmanNode(pq.Top().data(), pq.Top().freq(), 
                                          pq.Top().left(),pq.Top().right());
        // Remove Top Huffman Node from priority queue
        pq.Pop();
        HuffmanNode *n2 = new HuffmanNode(pq.Top().data(), pq.Top().freq(), 
                                          pq.Top().left(),pq.Top().right());
        pq.Pop();
        // Push a new Huffman Node into priority queue with 
        // the two popped Huffman Nodes as its children
        pq.Push(HuffmanNode(0, n1->freq() + n2->freq(), n1, n2));
    }

    std::map<char, std::string> code_table;
    BinaryOutputStream bos(ofs);
    // Create the root of the Huffman Tree
    HuffmanNode *root = new HuffmanNode(pq.Top().data(), pq.Top().freq(),
                                        pq.Top().left(), pq.Top().right());
    // Traverse the entire Huffman Tree
    PreorderRecur(root, bos, "", code_table);
        
    // Put total number of characters in input file in output file
    bos.PutInt(pq.Top().freq());
    // Reset input file to read again
    ifs.clear();
    ifs.seekg(0, std::ios::beg);
    // Traverse input file
    while (ifs.get(c)) {
        // Get new bit value of char from code table
        std::string temp = code_table.at(c);
        // Place each bit into output file
        for (unsigned int i = 0; i < temp.length(); i++) {
            if (temp[i] == '0')
                bos.PutBit(0);
            else 
                bos.PutBit(1);
        }
        
    }
}

void Huffman::PreorderRecur(HuffmanNode *n, BinaryOutputStream& bos, 
          std::string bits, std::map<char, std::string>& code_table) {
    // If the current node is a leaf
    if (n->IsLeaf()) {
        // Put a 1 into the output file
        bos.PutBit(true);
        // Followed by the char value
        bos.PutChar(n->data());
        // Insert the chars new bit code into the code table
        code_table.insert(std::pair<char, std::string>(n->data(), bits));
        // delete the current leaf node
        delete n;
        return;
    }
    // If the current node is an internal node
    // Put a 0 into the output file
    bos.PutBit(false);
    // Recurse on the left subtree adding a 0 to code table string
    PreorderRecur(n->left(), bos, bits + "0", code_table);
    // Recurse on the right subtree adding a 1 to code table string
    PreorderRecur(n->right(), bos, bits + "1", code_table);
    // After traversing both subtrees, delete current node
    delete n;
}

void Huffman::Decompress(std::ifstream &ifs, std::ofstream &ofs) {
    BinaryInputStream bis(ifs);
    
    std::map<std::string, char> code_table;
    // Create code table
    BuildCodeTable(bis, code_table);
    // Get total number of chars out of input file
    int freq = bis.GetInt();
    // Read file until no more chars left
    for (int i = 0; i < freq; i++) {
      std::string bits = "";
      // Find whether the current bit string is in code table
      auto itr = code_table.find(bits);
      // Keep adding to bit string until string matches element in code table
      while (itr == code_table.end()) {
        bool bit;
        // Attempt to get next bit from input file
        try {
            bit = bis.GetBit();
        } // If fail then only one char in code table
        catch (std::underflow_error const&) { }
        
        if (bit == 0)
          bits += "0";
        else
          bits += "1";
        // Check if bit string matches element in code table
        itr = code_table.find(bits);
      }
      // Put char matching bit string into output file
      ofs.put(itr->second);
  }
  ofs.close();
    
}


void Huffman::BuildCodeTable(BinaryInputStream& bis, std::map<std::string, char>& code_table) {
    HuffmanNode* root = new HuffmanNode(0,0);
    bool first_bit;
    
    try {
        // Get first bit of input file
        first_bit = bis.GetBit();
    } catch(std::underflow_error const&) {
        // If empty file
        exit(0);
    }
    
    // if the first bit is a 1
    if (first_bit == 1) {
        // Get the following char value
        char temp = bis.GetChar();
        // Insert the char value with code "0" into code table
        code_table.insert(std::pair<std::string, char>("0", temp));
        return;
    }
    // If the first bit is a 0
    // Build the rest of the Huffman tree
    BuildCodeTableHelper (root, bis, "", code_table);
}

void Huffman::BuildCodeTableHelper(HuffmanNode* node, BinaryInputStream& bis,
                         std::string bits, std::map<std::string, char>& code_table) {
    // If the current node is a leaf
    if (node->data() != 0) {
        // Insert the current bit string along 
        // with the char value into code table
        code_table.insert(std::pair<std::string, char>(bits, node->data()));
        // delete current node
        delete node;
        return;
    }
    // Reset node to give new children
    delete node;
    // Get the next bit in the input file
    bool bit = bis.GetBit();
    if (bit == 0) {
        // Make current node's left child an internal Huffman Node
        node = new HuffmanNode(0,0, new HuffmanNode(0,0), nullptr);
    } else {
        // Make current node's left child a leaf node
        node = new HuffmanNode(0, 0, new HuffmanNode(bis.GetChar(), 0), node->right());
    }
    // Recurse on the newly created left
    // Huffman Node adding a "0" to bit string
    BuildCodeTableHelper(node->left(), bis, bits + "0", code_table);
    // After traversing entire left subtree, delete current node
    delete node;

    // Get next bit from input file
    bit = bis.GetBit();
    if (bit == 0) {
        // Make current node's right child an internal Huffman Node
        node = new HuffmanNode(0, 0, nullptr, new HuffmanNode(0, 0));
    } else {
        // Make current node's right child a leaf node
        node = new HuffmanNode(0, 0, node->left(), 
                                new HuffmanNode(bis.GetChar(), 0));
    }
    // Recurse on the newly created right
    // Huffman node adding a "1" to bit string
    BuildCodeTableHelper(node->right(), bis, bits + "1", code_table);
    // After traversing entire right subtree, delete current node
    delete node;
}


#endif  // HUFFMAN_H_
