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
static int Curtok; // Current token, parser and lexer looks at this
static int getNextToken() {
    return Curtok = gettok();
}     // Goes to next token 
enum class Token {
    tok_eof = -1, tok_def = -2, tok_extern = -3, tok_identifier = -4, tok_number = -5
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
    virtual Value* codegen() = 0; };
// Literals expression class
class NumberExprAST : public ExprAST {
    double Val;

public:
    explicit NumberExprAST(double Val)
        : Val(Val) {
    }
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
// For variable reference
class VariableExprAST : public ExprAST {
    const std::string Name;
public:
    explicit VariableExprAST(const std::string& name) : Name(name) {}
    Value* codegen() override;
};
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;

public:
    PrototypeAST(
        std::string Name,
        std::vector<std::string> Args
    )
        : Name(std::move(Name)),
        Args(std::move(Args)) {
    }

    const std::string& getName() const {
        return Name;
    }

    Function* codegen();
};

class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;
public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                std::unique_ptr<ExprAST> Body
    )
        : Proto(std::move(Proto)),
          Body(std::move(Body)) {}
    Function* codegen();
};
    inline static std::unique_ptr<LLVMContext> TheContext;
	inline static std::unique_ptr<IRBuilder> Builder; // Add inline function to create a new IRBuilder instance
    inline static std::unique_ptr<Module> TheModule;
    inline static std::map<std::string, Value*> NamedValues;
   
    Value* LogErrorV(const char* str) {
        LogError(str);
        return nullptr;
    }
    //---------------------------------------------------------------------------//
    Value* NumberExprAST::codegen() {
        return ConstantFP::get(*TheContext, APFloat(Val));
    }
    Value* VariableExprAST::codegen() {
        const auto It = NamedValues.find(Name);
        // Using NamedValue[Name] creates a new value if not found
        if (It == NamedValues.end()) {
            return LogErrorV("Unknown variable name");
        }

        return It->second;
    }
    Value* BinaryExprAST::codegen() {
        Value* L = LHS->codegen();
        Value* R = RHS->codegen();
        if (!L || !R) {
            return nullptr;
        }
        switch (Op) {
    case '+':
        return Builder->CreateFAdd(L, R, "addtmp");
    case '-':
        return Builder->CreateFSub(L, R, "addtmp");
    case '*':
        return Builder->CreateFMul(L, R, "addtmp");
    case '<': {
        L = Builder->CreateFCmpULT(L, R, "cmptmp");
    }
        return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp");
    default:
        return LogErrorV("Invalid binary operator");

        }
    }

    //---------------------------------------------------------//
    class FunctionExprAST : public ExprAST {
        std::string Callee;
        std::vector<std::unique_ptr<ExprAST>> Args;
        public:
            FunctionExprAST(
                std::string Callee, std::vector<std::unique_ptr<ExprAST>> Args) : Callee(std::move(Callee)), Args(std::move(Args)) {
			}
    };

    Value* FunctionExprAST::codegen() {
        Function* CalleeF = TheModule->getFunction(Callee);

        if (!CalleeF)
            return LogErrorV("Unknown function referenced");
        if (CalleeF->arg_size() != Args.size())
            return LogErrorV("Incorrect number of arguments passed");
        std::vector<Value*> ArgsV;
        ArgsV.reserve(Args.size());
        for (const auto& Arg : Args) {
            Value* ArgValue = Arg->codegen();
            if (!ArgValue)
                return nullptr;
            ArgsV.push_back(ArgValue);
        }
        return Builder->CreateCall(
            CalleeF,
            ArgsV,
            "calltmp"
        );
    }


    Function* PrototypeAST::codegen() {
        std::vector<Type*> Doubles(
            Args.size()),
            Type::getDoubleTy(*TheContext)
            );
        FunctionType* FT =
            FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);

        Function* F =
            Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());
    }
    unsigned Idx = 0;
    for (auto& Arg : F->args())
        Arg.setName(Args[Idx++]);
    return F;

    Builder->CreateRet(RetVal);
    if (verifyFunction(*TheFunction, &errs())) {
        TheFunction->eraseFromParent();
        return nullptr;
    }
#endif 

