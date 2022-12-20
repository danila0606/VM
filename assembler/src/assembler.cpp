#include <iostream>
#include <fstream>
#include <string>
#include <exception>

#include "lexer.hpp"
#include "parser.hpp"
#include "program-builder.hpp"
#include "file_format.hpp"
#include "rest.hpp"


void assemble(std::shared_ptr<TokenList> list, std::ofstream& ofile) {
    std::shared_ptr<ProgramInfo> prog_info = std::make_shared<ProgramInfo>(list->length);
    generate_symbols(prog_info, list);
    std::shared_ptr<Token> token = list->head;
    while (token) {
        compile_token(prog_info, token);
    }

    BinaryFile bf(prog_info->data, prog_info->text);
    bf.DropBinaryFile(ofile);
}

int main(int argc, char** argv) {
    std::string ifile_name;
    std::ifstream ifile;
    std::string ofile_name;
    std::ofstream ofile;

    char* arg = nullptr;

    if (argc == 1) {
        std::cerr << "usage: assembler ifile [-o ofile]" << std::endl;
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        arg = argv[i];
        if (arg == "-o") {
            if (i + 1 < argc) {
                ofile_name = std::string{argv[i + 1]};
                if (!ofile_name.empty()) {
                    ofile = std::ofstream{ofile_name};
                    ++i;
                    continue;
                }
            } else {
                std::cerr << "No output specified with -o flag" << std::endl;
            }
        } else {
            ifile_name = std::string{argv[i]};
            ifile = std::ifstream{ifile_name};
        }
    }

    if (!ifile) {
        std::cerr << "Error opening input file" << std::endl;
    }
    if (ofile_name.empty()) {
        ofile_name = ifile_name;
        size_t dot = ifile_name.rfind('.');
        if (dot < ifile_name.size()) {
            ofile_name = ifile_name.substr(0, dot);
        }
        ofile_name += ".bin";
        ofile = std::ofstream{ofile_name};
    }

    ifile.seekg(0, ifile.end);
    size_t length = ifile.tellg();
    ifile.seekg(0, ifile.beg);

    std::string file_contents(length, 0);
    ifile.read(file_contents.data(), length);

    auto token_list = tokenize(file_contents);
    auto pass_two = parse(token_list);
    if (pass_two) {
        assemble(pass_two, ofile);
    }

    return 0;
}