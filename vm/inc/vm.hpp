#include "rest.hpp"

struct SadVM {

    SadVM(const std::vector<uint8_t>& data, const std::vector<uint8_t>& text);
    void run();
    void execute(uint32_t ins);

    std::vector<uint32_t> registers;
    std::vector<uint8_t> memspace;
    uint32_t program_halted = 0;
};

struct Frame final {

    uint32_t saved_registers[8];
    uint32_t source_pc = 0;
};