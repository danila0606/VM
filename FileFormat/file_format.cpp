#include "file_format.hpp"
#include <fstream>

BinaryFile::BinaryFile(std::vector<unsigned char>& new_data, 
                       std::vector<unsigned char>& new_text) :
                       data(new_data), text(new_text) {

}

void BinaryFile::DropBinaryFile(std::ofstream& ofile) {

    uint32_t data_len = static_cast<uint32_t>(data.size());
    ofile.write((char*)(&data_len), 4);

    uint32_t text_len = static_cast<uint32_t>(text.size());
    ofile.write((char*)(&text_len), 4);
    
    ofile.write((char*)data.data(), data_len);
    ofile.write((char*)text.data(), text_len);
}

BinaryFile::BinaryFile(std::string& filename) {
    
    std::ifstream ifile;
    ifile.open(filename);

    if (!ifile) {
        throw std::runtime_error("Can't open binary file: " + filename);
    }

    uint32_t data_len = 0;
    ifile.read((char*)(&data_len), 4);
    data.resize(data_len);

    uint32_t text_len = 0;
    ifile.read((char*)(&text_len), 4);
    text.resize(text_len);

    ifile.read((char*)data.data(), data_len);
    ifile.read((char*)text.data(), text_len);

    ifile.close();
}