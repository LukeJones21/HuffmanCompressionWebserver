#ifndef BSTREAM_H_
#define BSTREAM_H_

#include <cstddef>
#include <fstream>
#include <iostream>

class BinaryInputStream {
public:
    explicit BinaryInputStream(std::vector<bool> &ifs);

    bool GetBit();
    char GetChar();
    int GetInt();

private:
    std::vector<bool> ifs;
    char buffer = 0;
    size_t avail = 0;
    unsigned int cur_bit = 0;

    // Helpers
    void RefillBuffer();
};

BinaryInputStream::BinaryInputStream(std::vector<bool> &ifs) : ifs(ifs) { }

void BinaryInputStream::RefillBuffer() {
    
    if (cur_bit == ifs.size())
        throw std::underflow_error("No more characters to read");
    
    buffer = 0;
    //Read the next byte from the input stream
    for (unsigned int i = 0; i < ifs.size(); ++i) {
        if (ifs[i]) {
            buffer |= 1 << i;
        }
    }
    avail = 8;
}

bool BinaryInputStream::GetBit() {
    bool bit;

    if (!avail)
        RefillBuffer();

    avail--;
    bit = ((buffer >> avail) & 1) == 1;

#if 0  // Switch to 1 for debug purposes
  if (bit)
    std::cout << '1' << std::endl;
  else
    std::cout << '0' << std::endl;
#endif

    return bit;
}

char BinaryInputStream::GetChar() {
    char c = 0;
    for (int i = 8 - 1; i >= 0; i--) {
        c = ((c << 1) | GetBit());
    }

    return c;
}

int BinaryInputStream::GetInt() {
    int c = 0;
    for (unsigned int i = (sizeof(int) * 8); i > 0; i--) {
        c = ((c << 1) | GetBit());
    }
    return c;
}

class BinaryOutputStream {
  public:
    explicit BinaryOutputStream(std::vector<bool> &ofs);
    ~BinaryOutputStream();

    void Close();

    void PutBit(bool bit);
    void PutChar(char byte);
    void PutInt(int word);

  private:
    std::vector<bool> &ofs;
    char buffer = 0;
    size_t count = 0;

    // Helpers
    void FlushBuffer();
};

BinaryOutputStream::BinaryOutputStream(std::vector<bool> &ofs) : ofs(ofs) { }

BinaryOutputStream::~BinaryOutputStream() {
    Close();
}

void BinaryOutputStream::Close() {
    FlushBuffer();
}

void BinaryOutputStream::FlushBuffer() {
    // Nothing to flush
    if (!count)
        return;

    // If buffer isn't complete, pad with 0s before writing
    if (count > 0)
        buffer <<= (8 - count);

    // Write to output stream
    for (int i = 0; i < 8; ++i) {
        ofs.push_back((buffer >> i) & 1);
    }

    // Reset buffer
    buffer = 0;
    count = 0;
}

void BinaryOutputStream::PutBit(bool bit) {
    // Make some space and add bit to buffer
    buffer <<= 1;
    if (bit)
        buffer |= 1;

    // If buffer is full, write it
    if (++count == 8)
        FlushBuffer();
}

void BinaryOutputStream::PutChar(char byte) {
    bool a[8];

    for (int i = 0; i < 8; i++) {
        a[i] = byte & 1;
        byte = byte >> 1;
    }

    for (int i = 8 - 1; i >= 0; i--) 
        PutBit(a[i]);
}

void BinaryOutputStream::PutInt(int word) {
    bool a[sizeof(int)*8];
    for (unsigned int i = 0; i < (sizeof(int)*8); i++) {
        a[i] = word & 1;
        word = word >> 1;
    }
    for (unsigned int i = (sizeof(int)*8); i > 0; i--) {
        PutBit(a[i-1]);
    }
}

#endif  // BSTREAM_H_
