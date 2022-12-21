#include <iostream>
#include <string>
#include <chrono>

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

    auto start = std::chrono::steady_clock::now();
    vm.run();
    auto end = std::chrono::steady_clock::now();
    auto ans = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << std::endl << "Time spent = " << ans.count() << " microseconds" << std::endl;
    
    return 0;
}