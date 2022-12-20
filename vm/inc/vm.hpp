#include "rest.hpp"

struct SadVM {

    SadVM(const std::vector<uint8_t>& data, const std::vector<uint8_t>& text);
    void run();
    void execute(uint32_t ins);

    std::vector<uint32_t> registers;
    std::vector<uint8_t> memspace;
    uint32_t program_halted = 0;
};