#include "rest.hpp"

enum class FieldType {
    INT32,
    CLASS
};

struct ClassField {

    FieldType type;
    uint32_t field; // for type == CLASS : class number, for type == INT32 : it's value
};

struct ClassDescription {

    std::vector<ClassField> fields;
    std::vector<uint32_t> methods_addrs;
    uint32_t ctor_addr;

    uint32_t cl_size;
};

struct SadVM {

    SadVM(const std::vector<uint8_t>& data, const std::vector<uint8_t>& text);
    void run();
    void execute(uint32_t ins);

    std::vector<int32_t> registers;
    std::vector<uint32_t> reg_types; // 0 - uint32_t, 1 - pointer on object, 2 - pointer on array of pointers

    std::vector<uint8_t> memspace;
    uint32_t program_halted = 0;

    uint32_t classes_count = 0;
    std::vector<ClassDescription> class_descriptions;

    std::vector<std::pair<uint32_t, uint32_t>> heap_places;
};

struct Frame final {
    int32_t saved_registers[12];
    int32_t saved_types[12];
    uint32_t source_pc = 0;
};