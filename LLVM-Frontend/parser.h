#pragma once
#ifndef PARSER_H_
#define PARSER_H_

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ExprAST.h"

namespace llvm_frontend {

enum class Token {
    tok_eof = -1,
    tok_def = -2,
    tok_extern = -3,
    tok_identifier = -4,
    tok_number = -5
};

inline std::string IdentifierStr;
inline double NumVal = 0.0;
inline int Curtok = ' ';
inline std::map<char, int> BinopPrecedence;

inline int gettok() {
    static int LastChar = ' ';
    while (std::isspace(static_cast<unsigned char>(LastChar)))
        LastChar = std::getchar();
    if (LastChar == EOF)
        return static_cast<int>(Token::tok_eof);
    if (std::isalpha(static_cast<unsigned char>(LastChar)) || LastChar == '_') {
        IdentifierStr.clear();
        do {
            IdentifierStr += static_cast<char>(LastChar);
            LastChar = std::getchar();
        } while (std::isalnum(static_cast<unsigned char>(LastChar)) || LastChar == '_');
        if (IdentifierStr == "DEFINE" || IdentifierStr == "PROCEDURE")
            return static_cast<int>(Token::tok_def);
        if (IdentifierStr == "EXTERN")
            return static_cast<int>(Token::tok_extern);
        return static_cast<int>(Token::tok_identifier);
    }
    if (std::isdigit(static_cast<unsigned char>(LastChar)) || LastChar == '.') {
        std::string Number;
        bool HasDot = false;
        do {
            if (LastChar == '.') HasDot = true;
            Number += static_cast<char>(LastChar);
            LastChar = std::getchar();
        } while (std::isdigit(static_cast<unsigned char>(LastChar)) || (!HasDot && LastChar == '.'));
        NumVal = std::strtod(Number.c_str(), nullptr);
        return static_cast<int>(Token::tok_number);
    }
    int ThisChar = LastChar;
    LastChar = std::getchar();
    return ThisChar;
}

inline int getNextToken() { return Curtok = gettok(); }

inline int GetTokPrecedence() {
    if (!isascii(Curtok)) return -1;
    auto It = BinopPrecedence.find(static_cast<char>(Curtok));
    return It == BinopPrecedence.end() || It->second <= 0 ? -1 : It->second;
}

inline std::unique_ptr<ExprAST> LogError(const char *Message) {
    std::cerr << "Error: " << Message << '\n';
    return nullptr;
}

inline std::unique_ptr<ExprAST> ParseExpression();

inline std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(NumVal);
    getNextToken();
    return std::move(Result);
}

inline std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken();
    auto Value = ParseExpression();
    if (!Value) return nullptr;
    if (Curtok != ')') return LogError("Expected ')' ");
    getNextToken();
    return Value;
}

inline std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = IdentifierStr;
    getNextToken();
    if (Curtok != '(')
        return std::make_unique<VariableExprAST>(std::move(IdName));

    std::vector<std::unique_ptr<ExprAST>> Args;
    getNextToken();
    if (Curtok != ')') {
        while (true) {
            auto Arg = ParseExpression();
            if (!Arg) return nullptr;
            Args.push_back(std::move(Arg));
            if (Curtok == ')') break;
            if (Curtok != ',') return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }
    getNextToken();
    return std::make_unique<FunctionExprAST>(std::move(IdName), std::move(Args));
}

inline std::unique_ptr<ExprAST> ParsePrimary() {
    switch (Curtok) {
    default: return LogError("Expected an expression");
    case static_cast<int>(Token::tok_identifier): return ParseIdentifierExpr();
    case static_cast<int>(Token::tok_number): return ParseNumberExpr();
    case '(': return ParseParenExpr();
    }
}

inline std::unique_ptr<ExprAST> ParseBinOpRHS(int ExpressionPrecedence, std::unique_ptr<ExprAST> LHS) {
    while (true) {
        int TokenPrecedence = GetTokPrecedence();
        if (TokenPrecedence < ExpressionPrecedence) return LHS;
        int BinaryOperator = Curtok;
        getNextToken();
        auto RHS = ParsePrimary();
        if (!RHS) return nullptr;
        int NextPrecedence = GetTokPrecedence();
        if (TokenPrecedence < NextPrecedence)
            RHS = ParseBinOpRHS(TokenPrecedence + 1, std::move(RHS));
        if (!RHS) return nullptr;
        LHS = std::make_unique<BinaryExprAST>(static_cast<char>(BinaryOperator), std::move(LHS), std::move(RHS));
    }
}

inline std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS) return nullptr;
    return ParseBinOpRHS(0, std::move(LHS));
}

}

#endif // PARSER_H_