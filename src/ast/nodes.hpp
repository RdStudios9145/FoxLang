#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../tokenstreamer.hpp"

namespace FoxLang {
class AST {
public:
	virtual ~AST() = default;
};

/// ExprAST - Base class for all expression nodes.
class ExprAST : public AST {
public:
	virtual ~ExprAST() = default;
};

class FileAST : public AST {
	std::vector<std::unique_ptr<ExprAST>> expressions;

public:
	FileAST(std::vector<std::unique_ptr<ExprAST>> expressions)
		: expressions(std::move(expressions)) {}
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
	std::string Val;

public:
	NumberExprAST(std::string Val) : Val(Val) {}
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
	std::string Name;

public:
	VariableExprAST(const std::string &Name) : Name(Name) {}
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
	Token Op;
	std::unique_ptr<ExprAST> LHS, RHS;

public:
	BinaryExprAST(Token Op, std::unique_ptr<ExprAST> LHS,
				  std::unique_ptr<ExprAST> RHS)
		: Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
	std::string Callee;
	std::vector<std::unique_ptr<ExprAST>> Args;

public:
	CallExprAST(const std::string &Callee,
				std::vector<std::unique_ptr<ExprAST>> Args)
		: Callee(Callee), Args(std::move(Args)) {}
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST {
	std::string Name;
	std::vector<std::string> Args;

public:
	PrototypeAST(const std::string &Name, std::vector<std::string> Args)
		: Name(Name), Args(std::move(Args)) {}

	const std::string &getName() const { return Name; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
	std::unique_ptr<PrototypeAST> Proto;
	std::unique_ptr<ExprAST> Body;

public:
	FunctionAST(std::unique_ptr<PrototypeAST> Proto,
				std::unique_ptr<ExprAST> Body)
		: Proto(std::move(Proto)), Body(std::move(Body)) {}
};

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
inline int getPrecedence(TokenType token) {
	// Make sure it's a declared binop.
	switch (token) {
	default:
		return -1;
	}
}

/// LogError* - These are little helper functions for error handling.
inline std::unique_ptr<ExprAST> LogError(const char *Str) {
	fprintf(stderr, "Error: %s\n", Str);
	return nullptr;
}

inline std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
	LogError(Str);
	return nullptr;
}

} // namespace FoxLang
