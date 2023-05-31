#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>

#include "parser.hpp"
#include "rest.hpp"

int expression(std::string expr, int& i);

int number(std::string expr, int& i) {
    int result = expr[i++] - '0';
    while (expr[i] >= '0' && expr[i] <= '9') {
        result = 10 * result + expr[i++] - '0';
    }
    return result;
}

int factor(std::string expr, int& i) {
    if (expr[i] >= '0' && expr[i] <= '9') {
        return number(expr, i);
    }
    else if (expr[i] == '(') {
        ++i; 
        int result = expression(expr, i);
        ++i; 
        return result;
    }
    else if (expr[i] == '-') {
        ++i; 
        return -1 * expression(expr, i);
    }
    return 0;
}

int term(std::string expr, int& i) {
    int result = factor(expr, i);
    while (expr[i] == '*' || expr[i] == '/') {
        if (expr[i++] == '*') {
            result *= factor(expr, i);
        }
        else {
            result /= factor(expr, i);
        }
    }
    return result;
}

int expression(std::string expr, int& i) {
    int result = term(expr, i);
    while (expr[i] == '+' || expr[i] == '-') {
        if (expr[i++] == '+') {
            result += term(expr, i);
        }
        else {
            result -= term(expr, i);
        }
    }
    return result;
}

int eval(std::string expr) {
    int init_idx = 0;
    return expression(expr, init_idx);
}

void parse_arithmetic_expr(std::shared_ptr<TokenList> list, std::shared_ptr<Token>& tok) {
    std::shared_ptr<Token> start = tok;
    std::shared_ptr<Token> end = start->next;

    int st_line_num = start->line_num;
    int paren_count = 1;
    int value = 0;

    std::string ans = "";

    while (paren_count) {
        if (!end) {
            throw std::runtime_error("Mismatched parens at line " + std::to_string(start->line_num));
        }
        switch (end->type) {
            case TokenType::TOKEN_TYPE_PLUS:
            case TokenType::TOKEN_TYPE_MINUS:
            case TokenType::TOKEN_TYPE_STAR:
            case TokenType::TOKEN_TYPE_FSLASH:
            case TokenType::TOKEN_TYPE_INT_LIT:
            case TokenType::TOKEN_TYPE_FLOAT_LIT:
            case TokenType::TOKEN_TYPE_CHAR_LIT:
                break;
            case TokenType::TOKEN_TYPE_LPAREN:
                ++paren_count;
                break;
            case TokenType::TOKEN_TYPE_RPAREN:
                --paren_count;
                break;
        }
        end = end->next;
    }

    while (start != end) {
        ans += start->str;
        start = start->next;
    }

    value = eval(ans);
    std::shared_ptr<Token> new_token = std::make_shared<Token>(ans, st_line_num, TokenType::TOKEN_TYPE_IMMEDIATE, value);
    list->push_back_token(new_token);
    tok = end->prev;
}

void parse_ident(std::shared_ptr<TokenList> list, std::shared_ptr<Token>& token) {
    Opcode op = str_to_op(token->str);

    if (op != Opcode::NOT_AN_OPCODE) {
        std::shared_ptr<Token> instruction_token = std::make_shared<Token>(token->str, token->line_num, TokenType::TOKEN_TYPE_INSTR, op);
        list->push_back_token(instruction_token);
    }

    else if (token->next && token->next->type == TokenType::TOKEN_TYPE_COLON) {
        std::shared_ptr<Token> label_token = std::make_shared<Token>(token->str, token->line_num, TokenType::TOKEN_TYPE_NEW_LABEL, 0);
        list->push_back_token(label_token);
        token = token->next;
    }
    else {
        std::shared_ptr<Token> label_ref_token = std::make_shared<Token>(token->str, token->line_num, TokenType::TOKEN_TYPE_LABEL_REF, 0);
        list->push_back_token(label_ref_token);
    }
}

void parse_register(std::shared_ptr<TokenList> list, std::shared_ptr<Token>& token) {
    Register r;
    if (token->next) {
        token = token->next;
        r = str_to_reg(token->str);
        
        if (r != Register::NOT_A_REGISTER) {
            std::shared_ptr<Token> register_token = std::make_shared<Token>(token->str, token->line_num, TokenType::TOKEN_TYPE_REGISTER, r);
            list->push_back_token(register_token);
            return;
        }
    }
    throw std::runtime_error("Expected register identifer at line " + std::to_string(token->line_num));
}

void self_evals(std::shared_ptr<TokenList> list, std::shared_ptr<Token>& tok) {
    list->push_back_token(tok);
}

void parse_directive(std::shared_ptr<TokenList> list, std::shared_ptr<Token>& tok) {
    self_evals(list, tok);
}

void parse_char_lit(std::shared_ptr<TokenList> list, std::shared_ptr<Token>& tok) {
    self_evals(list, tok);
}

void parse_string_lit(std::shared_ptr<TokenList> list, std::shared_ptr<Token>& tok) {
    self_evals(list, tok);
}

std::shared_ptr<TokenList> parse(std::shared_ptr<TokenList> list) {
    std::shared_ptr<TokenList> token_list = std::make_shared<TokenList>();
    std::shared_ptr<Token> tok = list->head;

    while (tok && tok->type != TokenType::TOKEN_TYPE_EOF) {
        switch (tok->type) {
            case TokenType::TOKEN_TYPE_COMMA: break;
            case TokenType::TOKEN_TYPE_LBRACE : self_evals(token_list, tok);break;
            case TokenType::TOKEN_TYPE_RBRACE : self_evals(token_list, tok);break;
            case TokenType::TOKEN_TYPE_IDENT: parse_ident(token_list, tok); break;
            case TokenType::TOKEN_TYPE_LPAREN: parse_arithmetic_expr(token_list, tok); break;
            case TokenType::TOKEN_TYPE_DOLLAR: parse_register(token_list, tok); break;
            case TokenType::TOKEN_TYPE_DIRECTIVE: parse_directive(token_list, tok); break;
            case TokenType::TOKEN_TYPE_CHAR_LIT: parse_char_lit(token_list, tok); break;
            case TokenType::TOKEN_TYPE_STRING_LIT: parse_string_lit(token_list, tok); break;

            default:
            case TokenType::TOKEN_TYPE_UNKNOWN:
                throw std::runtime_error("Error parsing token at line " + std::to_string(tok->line_num));
                return nullptr;
        }
        tok = tok->next;
    }
    return token_list;
}
