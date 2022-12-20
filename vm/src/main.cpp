#include <iostream>
#include <string>

#include "vm.hpp"
#include "../FileFormat/file_format.hpp"


int main(int argc, char **argv) {

    if (argc != 2) {
        std::cout << "Must be 1 arg - name of binary file" << std::endl;
        return -1;
    }

    std::string input_file{argv[1]};

    BinaryFile bin_file;
    try {
        bin_file = BinaryFile(input_file);
    }
    catch (std::exception& e) {
        std::cerr << e.what()<<std::endl;
        exit(-1);
    }

    SadVM vm(bin_file.data, bin_file.text);

    vm.run();
    
    return 0;
}