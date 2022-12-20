#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>

struct BinaryFile {

    BinaryFile(std::vector<unsigned char>& new_data, std::vector<unsigned char>& new_text);
    BinaryFile(std::string& file_name);
    BinaryFile() = default;

    void DropBinaryFile(std::ofstream& ofile);

    std::vector<uint8_t> data;
    std::vector<uint8_t> text;
};

