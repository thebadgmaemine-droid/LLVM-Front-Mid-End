#pragma once
#ifndef parser_H_
#define parser_H_
#include "ExprAST.h"
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
//-----------------------------BASE DEF-------------------------------//
using llvm;
// Binop precedence : Holds the precedence for Binary values 
static std::map<char, int> BinopPrecedence;
static int GetTokPrecedence() {
    if (!isascii(Curtok))
        return -1;
    // Delcared -> safe
    int TokPrec = BinopPrecedence[Curtok]; // Cannot be 0 ( default value ), so is either negative or positive ( negative acts as return -1; and postiive acts as return n;
    if (TokPrec <= 0)
        return -1;
    return TokPrec;
}
/* parse_expression()
    return parse_expression_1(parse_primary(), 0)
parse_expression_1(lhs, min_precedence)
    lookahead := peek next token
    while lookahead is a binary operator whose precedence is >= min_precedence
        op := lookahead
        advance to next token
        rhs := parse_primary ()
        lookahead := peek next token
        while lookahead is a binary operator whose precedence is greater
                 than op's, or a right-associative operator
                 whose precedence is equal to op's
            rhs := parse_expression_1 (rhs, precedence of op + (1 if lookahead precedence is greater, else 0))
            lookahead := peek next token
        lhs := the result of applying op with operands lhs and rhs
    return lhs */
    // Def
static std::unique_ptr<ExprAST> ParseExpression();
static std::unique_ptr<ExprAST> ParsePrimary();
static std::unique_ptr<ExprAST> LogError(const char* Str);
static std::unique_ptr<ExprAST> ParseIdentifierExpr();
//-----------------------------FUNCTIONS DEF-------------------------------//
static std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(NumVal);
    getNextToken();
    return std::move(Result);
}

static std::unique_ptr<ExprAST> ParseParentExpr() {
    getNextToken();
    auto V = ParseExpression();
    if (!V) {
        return nullptr;
    }
    if (Curtok != ')') {
        LogError("Expected )");
    }
}
}
static std::unique_ptr<ExprAST> ParsePrimary() {
    switch (Curtok) {
    default:
        return LogError("Expected an expression");
    case (static_cast<int>(Token::tok_number)):
        return ParseNumberExpr();
    case (static_cast<int>(Token::tok_identifier)):
        return ParseIdentifierExpr();
    case 'C':
        return ParseParentExpr();

    }
}
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = IdentifierStr;
    getNextToken();
    // For Calling
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (Curtok != '(') {
        return std::make_unique<VariableExprAST>(IdName);
    }

    if (Curtok != ')') {
        while (true) {
            if (auto Arg = ParseExpression()) {
                Args.push_back(std::move(Arg));
            }
            else
                return nullptr;


            if (Curtok == ')')
                break;

            if (Curtok != ',')
                return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }
    getNextToken();
    return std::make_unique<FunctionExprAST>(IdName, std::move(Args));
}
static std::unique_ptr<ExprAST> ParseExpression() {
    return ParsePrimary();
}
static std::unique_ptr<ExprAST> ParseBinOpRHS(const int ExprPrec, std::unique_ptr<ExprAST> LHS) {
    while (true) {
        int TokPrec = GetTokPrecedence();
        if (TokPrec < ExprPrec) {
            return LHS;

            int BinOp = Curtok;
            getNextToken();
            auto RHS = ParsePrimary();
            if (!RHS) {
                return nullptr;

            }
        }

    }
    
    FunctionExprAST(const std::string & Calee,
        std::vector<std::unique_ptr<ExprAST>> Args)
        : Calee(Calee), Args(std::move(Args)) {
    }
    std::unique_ptr<ExprAST> LogErrorP(const char* Str) {
        LogError(Str); m
        return nullptr;
    } 


#endif