#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "lexer.hpp"
#include "rest.hpp"


int is_part_of_ident(char c) {
    return c == '_' || isalnum(c);
}

int is_hex(char c) {
    return (c >= '0' && c <= '9') ||
        (c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'F');
}

char escape_hex(std::string str) {
    return strtol(str.c_str(), nullptr, 16);
}

char escape(char c) {
    switch (c) {
        default: return c;
        case 't': return '\t';
        case 'n': return '\n';
        case 'r': return '\r';
    }
}

char parse_char(std::string code, int& i) {
    char value = 0;
    if (code[i] != '\'') {
        /* TODO: Handle error, no valid start of char. */
    }
    ++i;

    if (code[i] == '\'') {
        /* TODO: Handle error, '' is not a valid char. */
    }
    else if (code[i] == '\\') {
        ++i;
        if ((code[i] == 'x') && (i + 1 < code.size())) {
            value = escape_hex(code.substr(i + 1));
            i += 3;
        }
        else {
            value = escape(code[i]);
            i += 2;
        }
    }
    else {
        value = code[i];
        i += 2;
    }
    return value;
}

std::string parse_string(std::string code, int& i) {
    if (code[i] != '"') {
        return "";
    }

    ++i;
    std::string ans{};

    while (i < code.size()) {
        if (code[i] == '"') {
            ans.push_back('\0');
            break;
        }
        else if (code[i] == '\\') {
            ++i;
            ans.push_back(escape(code[i]));
        }
        else {
            ans.push_back(code[i]);
        }
        ++i;
    }
    
    ++i;
    return ans;
}

std::shared_ptr<TokenList> tokenize(std::string code) {
    int token_line_num = 1, token_internal = 0;

    TokenType token_type;
    std::string token_str{};
    std::shared_ptr<TokenList> token_list = std::make_shared<TokenList>();

    for (int i = 0; (i < code.size()) && (code[i] != 0);) {
        if (isspace(code[i])) {
            if (code[i] == '\n') {
                ++token_line_num;
            }
            ++i;
            continue;
        }
        if (code[i] == '_' || isalpha(code[i])) {
            int start = i;
            ++i;
            for (; (i < code.size()) && (is_part_of_ident(code[i])); ++i);
            int end = i;
            token_str = code.substr(start, end - start);
            token_type = TokenType::TOKEN_TYPE_IDENT;
        }
        else if (isdigit(code[i])) {
            int start = i;
            ++i;
            for (; (i < code.size()) && (isdigit(code[i])); ++i);
            int end = i;
            token_str = code.substr(start, end - start);
            token_type = TokenType::TOKEN_TYPE_INT_LIT;
        }
        else if (code[i] == '#') {
            while ((code[i] != 0) && (code[i] != '\n') && (code[i] != '\r')) {
                ++i;
            }
            continue;
        }
        else if ((code[i] == '.') && (i + 1 < code.size()) && isalpha(code[i + 1])) {
            int start = i;
            ++i;
            for (; (i < code.size()) && (is_part_of_ident(code[i])); ++i);
            int end = i;
            token_str = code.substr(start, end - start);
            token_type = TokenType::TOKEN_TYPE_DIRECTIVE;
        }
        else if (code[i] == '\'') {
            token_str = "<char>";
            token_internal = parse_char(code, i);
            token_type = TokenType::TOKEN_TYPE_CHAR_LIT;
        }
        else if (code[i] == '"') {
            token_str = parse_string(code, i);
            token_type = TokenType::TOKEN_TYPE_STRING_LIT;
        }
        else {
            switch (code[i]) {
                case '+': token_str = "+"; token_type = TokenType::TOKEN_TYPE_PLUS; break;
                case '-': token_str = "-"; token_type = TokenType::TOKEN_TYPE_MINUS; break;
                case '*': token_str = "*"; token_type = TokenType::TOKEN_TYPE_STAR; break;
                case '/': token_str = "/"; token_type = TokenType::TOKEN_TYPE_FSLASH; break;
                case ',': token_str = ","; token_type = TokenType::TOKEN_TYPE_COMMA; break;
                case '.': token_str = "."; token_type = TokenType::TOKEN_TYPE_DOT; break;
                case ':': token_str = ":"; token_type = TokenType::TOKEN_TYPE_COLON; break;
                case '$': token_str = "$"; token_type = TokenType::TOKEN_TYPE_DOLLAR; break;
                case '(': token_str = "("; token_type = TokenType::TOKEN_TYPE_LPAREN; break;
                case ')': token_str = ")"; token_type = TokenType::TOKEN_TYPE_RPAREN; break;
                default: token_str = "(unknown)"; token_type = TokenType::TOKEN_TYPE_UNKNOWN; break;
            }
            ++i;
        }

        std::shared_ptr<Token> new_token = std::make_shared<Token>(token_str, token_line_num, token_type, token_internal);
        token_list->push_back_token(new_token);
        token_internal = 0;
    }

    std::shared_ptr<Token> eof_token = std::make_shared<Token>("EOF", -1, TokenType::TOKEN_TYPE_EOF, 0);
    token_list->push_back_token(eof_token);
    return token_list;
}