#pragma once

#include "./nodes.hpp"
#include <memory>

namespace FoxLang {
class Parser {
public:
	Parser(std::vector<Token> *);
	FileAST *parse();

private:
	std::vector<Token> *tokens;
	std::vector<Token>::iterator current;

private:
	std::optional<std::unique_ptr<ExprAST>> parseNumberExpr();
	std::optional<std::unique_ptr<ExprAST>> parseParenExpr();
	std::optional<std::unique_ptr<ExprAST>> parseIdentifierExpr();
	std::optional<std::unique_ptr<ExprAST>> parsePrimary();
	std::optional<std::unique_ptr<ExprAST>> parseExpression();
	std::optional<std::unique_ptr<TypeAST>> parseType();
	std::optional<std::unique_ptr<PrototypeAST>> parsePrototype();
	std::optional<std::unique_ptr<FunctionAST>> parseDefinition();

	std::optional<std::unique_ptr<ExprAST>>
	parseBinOpRHS(int, std::optional<std::unique_ptr<ExprAST>>);
};
} // namespace FoxLang
