#ifndef EXPRAST_H_
#define EXPRAST_H_

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

enum class Token {
    tok_eof = -1, tok_def = -2, tok_extern = -3,
    tok_identifier = -4, tok_number = -5
};

inline std::string IdentifierStr;
inline double NumVal = 0.0;

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
            if (LastChar == '.')
                HasDot = true;
            Number += static_cast<char>(LastChar);
            LastChar = std::getchar();
        } while (std::isdigit(static_cast<unsigned char>(LastChar)) ||
            (!HasDot && LastChar == '.'));
        NumVal = std::strtod(Number.c_str(), nullptr);
        return static_cast<int>(Token::tok_number);
    }

    const int ThisChar = LastChar;
    LastChar = std::getchar();
    return ThisChar;
}

class ExprAST {
public:
    virtual ~ExprAST() = default;
    virtual Value *codegen() = 0;
};
// Literals expression class
class NumberExprAST : public ExprAST {
    double Val;
public:
    NumberExprAST(const double val) : Val(Val) {}
    Value* codegen() override;
};

// For binary
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;

public:
    BinaryExprAST(const char Op, std::unique_ptr<ExprAST> LHS,
        std::unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {
    }
    Value* codegen() override;
};
//
// For variable reference
class VariableExprAST : public ExprAST {
    const std::string Name;
public:
    VariableExprAST(const std::string& name) : Name(name) {}
    Value* codegen() override;
};
// For function call

class FunctionExprAST : public ExprAST {
    const std::string Calee; // I hate how this is spelled, but I don't want to change it now
    const std::vector<std::unique_ptr<ExprAST>> Args;
public:
    // This implements the constructor for the function call expression AST node.
    // It takes a string; the name of the function called (Calee)
    // + A vector of unique pointers to ExprAST objects as the arguments to function calls.
    FunctionExprAST(const std::string& Calee,
        std::vector<std::unique_ptr<ExprAST>> Args)
        : Calee(Calee), Args(std::move(Args)) {

    }
};

class PrototypeAST : public ExprAST {
    const std::string name;
public:
    PrototypeAST(const std::string& name, std::vector<std::string> Args)
        : name(name), Args(std::move(Args)) {
    } // Implement prototype constructor 
// okay sorry , std::move cast argument to an rvalue ( if you remember, is essential non-guarded memory ) reference -> compiler can steal resources all i wants


};

class FunctionAST : public ExprAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;
public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {
    }

};


// Parser
static int Curtok; // Current token, parser and lexer looks at this
static int getNextToken() {
    return Curtok = gettok();  // Don't shadow value Curtok
}     // Goes to next token

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
static std::unique_ptr<ExprAST> ParseNumberExpr() {


 static std::unique_ptr<ExprAST> ParseExpression() {
        return ParsePrimary();
 
 
 
 
 
 }









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

// Error logging
std::unique_ptr<ExprAST> LogError(const char* Str) {
    std::cerr << "Error:" << Str << "\n";
    return nullptr;

}
// std::unique_ptr<ExprAST> LogError(const char *Str)
// Primary
// def
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

}



   
    
    std::unique_ptr<ExprAST> LogErrorP(const char* Str) {
        LogError(Str);
        return nullptr;
    }

    static std::unique_ptr<LLVMContext> TheContext;
    static std::unique_ptr<IRBuilder<>> Builder; // <- "keeps track of the current insertion point (the active llvm::BasicBlock and an iterator position within it)."
    static std::unique_ptr<Module> TheModule;
    static std::map<std::string, Value*> NamedValues;

    Value* LogErrorV(const char* str) {
        LogError(str);
        return nullptr;
    }

    Value* NumberExprAST::codegen() {
        ConstantFP::get(*TheContext, APFloat(Val));
    }

    Value* VariablExprAST::codegen() {
        Value V* = NamedValues[Name];
        if (!V) {
            LogErrorV("Unknown variable name");
        }
        return V;
    }
    Value* BinaryExprAST::codegen() {
        Value* L = LHS->codegen();
        Value* R = RHS->codegen();
        if (!L || !R) {
            return nullptr;
        }
        Switch(Op) {
    case '+':
        return Builder->CreateFAdd(L, R, "addtmp");
    case '-':
        return Builder->CreateFSub(L, R, "addtmp");
    case '*':
        return Builder->CreateFMul(L, R, "addtmp");
    case '<':
        L = Builder->CreateFCmpULT(L, R, "cmptmp");
        return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp");
    default:
        return LogErrorV("Invalid binary operator");

        };

    };
    Value* CallExprAST::codegen() {
        Function* CalleeF = TheModule->getFunction(Callee);
        if (!CalleeF) {
            return LogErrorV("Unknown function referenced");
        }
        if (CalleeF->arg_size() != Args.size()) {
            return LogErrorV("Incorrect # Arguments passed");
        }
        std::vector<Value*> ArgsV;
        for (unsigned i = 0; e = Args.size(); i != e; i++) {
            ArgsV.push_back(Args[i]->codegen());
            if (!ArgsV.back()) {
                return nullptr;
            }

            return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
        }
    }
    unsigned Idx = 0;
    for (auto& Arg : args()<-F )
        Arg.setName(Args[Idx++]);

    return F;




#endif 

