#include "parser.h"
#include <fmt/format.h>
#include <iostream>

namespace FoxLang {
Parser::Parser(std::vector<Token> *tokens)
	: tokens(tokens), current(tokens->begin()) {}

std::optional<std::shared_ptr<ExprAST>> Parser::parseNumberExpr() {
	auto ret = std::make_shared<NumberExprAST>(current->lexeme);
	current++;
	return ret;
}

std::optional<std::shared_ptr<ExprAST>> Parser::parseParenExpr() {
	current++;
	auto expression = parseExpression();

	if (!expression) return std::nullopt;

	if (current->type != TokenType::RIGHT_PAREN) {
		LogError(
			fmt::format("Expected right parenthesis on line {}", current->line)
				.c_str());
		return std::nullopt;
	}

	current++;
	return expression;
}

std::optional<std::shared_ptr<ExprAST>> Parser::parseIdentifierExpr() {
	std::string identifierString = current->lexeme;
	current++;

	if (current->type != TokenType::LEFT_PAREN) // Simple variable ref.
		return std::make_shared<VariableExprAST>(identifierString);
	current++;

	// Call.
	std::vector<std::shared_ptr<ExprAST>> Args;
	// if (current->type != TokenType::RIGHT_PAREN) {
	// while (true) {
	// if (auto Arg = parseExpression().value())
	// Args.push_back(std::move(Arg));
	// else
	// return std::nullopt;
	//
	// if (current->type == TokenType::RIGHT_PAREN) break;
	//
	// if (current->type != TokenType::COMMA) {
	// LogError("Expected ')' or ',' in argument list");
	// return std::nullopt;
	// }
	// current++;
	// }
	// }

	while (current->type != TokenType::RIGHT_PAREN) {
		auto arg = parseExpression();
		if (!arg) {
			LogError("unable to parse expression");
			return std::nullopt;
		}
		Args.push_back(std::move(arg.value()));

		if (current->type == TokenType::COMMA) current++;
	}

	// Eat the ')'.
	current++;

	return std::make_shared<CallExprAST>(identifierString, std::move(Args));
}

std::optional<std::shared_ptr<ExprAST>> Parser::parsePrimary() {
	switch (current->type) {
	default:
		return LogError(
			fmt::format(
				"unknown token {} when expecting an expression on line {}",
				current->lexeme, current->line)
				.c_str());
	case TokenType::IDENTIFIER:
		return parseIdentifierExpr();
	case TokenType::NUMBER:
		return parseNumberExpr();
	case TokenType::LEFT_PAREN:
		return parseParenExpr();
	}
}

std::optional<std::shared_ptr<ExprAST>> Parser::parseExpression() {
	auto lhs = parsePrimary();
	if (!lhs) return std::nullopt;

	return parseBinOpRHS(0, std::move(lhs));
}

std::optional<std::shared_ptr<StmtAST>> Parser::parseStatement() {
	if (current->type == TokenType::LET) return parseLet();
	if (current->type == TokenType::RETURN) return parseReturnStmt();

	return parseExprStatement();
}

std::optional<std::shared_ptr<ExprStmt>> Parser::parseExprStatement() {
	auto expr = parseExpression();
	if (!expr) {
		LogError("Expected expression");
		return std::nullopt;
	}

	if (current->type != TokenType::SEMICOLON) {
		LogError("expected ; after expression");
		return std::nullopt;
	}
	current++;

	return std::make_shared<ExprStmt>(std::move(expr.value()));
}

std::optional<std::shared_ptr<BlockAST>> Parser::parseBlock() {
	if (current->type != TokenType::LEFT_BRACKET) {
		LogError("Expected '{' to start block");
		return std::nullopt;
	}
	current++;

	std::vector<std::shared_ptr<StmtAST>> content;

	while (true) {
		auto stmt = parseStatement();
		if (!stmt) {
			LogError("Expected expression in block");
			return std::nullopt;
		}

		content.push_back(std::move(stmt.value()));
		if (current->type == TokenType::RIGHT_BRACKET) break;
	}

	current++;
	return std::make_shared<BlockAST>(std::move(content));
}

std::optional<std::shared_ptr<ExprAST>>
Parser::parseBinOpRHS(int precedence,
					  std::optional<std::shared_ptr<ExprAST>> lhs) {
	while (true) {
		int currentPrecedence = getPrecedence(current->type);

		if (currentPrecedence < precedence) return lhs;

		Token binaryOperator = *current;
		current++;

		auto rhs = parsePrimary();
		if (!rhs) return std::nullopt;

		if (precedence < getPrecedence(current->type)) {
			rhs = parseBinOpRHS(precedence + 1, std::move(rhs));
			if (!rhs) return std::nullopt;
		}

		lhs = std::make_shared<BinaryExprAST>(
			binaryOperator, std::move(lhs.value()), std::move(rhs.value()));
	}
}

std::optional<std::shared_ptr<TypeAST>> Parser::parseType() {
	if (current->type != TokenType::IDENTIFIER) {
		LogError("Unable to parse type");
		return std::nullopt;
	}

	std::string type = current->lexeme;
	current++;
	return std::make_shared<TypeAST>(type);
}

std::optional<std::shared_ptr<VarDecl>> Parser::parseLet() {
	// Move past `let` token, onto either a `mut` token or the var name
	current++;

	bool mut = false;
	if (current->type == TokenType::MUT) {
		mut = true;
		current++;
	}

	std::string name = current->lexeme;
	current++;
	auto type = parseType();

	if (!type) {
		LogError("Type inference is not yet implemented. If you wrote Haskell, "
				 "send me an email");
		return std::nullopt;
	}

	if (current->type == TokenType::SEMICOLON) {
		return std::make_shared<VarDecl>(name, std::nullopt, mut);
	}

	if (current->type != TokenType::EQUAL) {
		LogError("You need an equal sign zidiot");
		return std::nullopt;
	}
	current++; // move past = onto the expression

	auto value = parseExpression();
	if (!value) {
		LogError("You need an expression after the =");
		LogError("Im done with errors rn");
		return std::nullopt;
	}
	current++;

	return std::make_shared<VarDecl>(name, std::move(value), mut);
}

std::optional<std::shared_ptr<PrototypeAST>> Parser::parsePrototype() {
	if (current->type != TokenType::IDENTIFIER) {
		LogErrorP("Expected function name in prototype");
		return std::nullopt;
	}

	std::string name = current->lexeme;
	current++;

	if (current->type != TokenType::LEFT_PAREN) {
		LogErrorP("Expected '(' in function prototype");
		return std::nullopt;
	}

	std::vector<std::string> argNames;
	std::vector<std::shared_ptr<TypeAST>> typeNames;
	current++; // Consume the ( and begin parsing the args

	while (current->type == TokenType::IDENTIFIER) {
		argNames.push_back(current->lexeme);
		current++; // Consume the IDENTIFIER arg name

		auto type = parseType(); // Consumes IDENTIFIERs for the type
		if (!type) {
			LogErrorP("Unable to parse type");
			return std::nullopt;
		}

		typeNames.push_back(std::move(type.value()));

		if (current->type != TokenType::COMMA) {
			break;
		}

		current++;
	}

	if (current->type != TokenType::RIGHT_PAREN) {
		LogErrorP("Expected ')' in function prototype");
		return std::nullopt;
	}

	current++;
	auto retType = parseType();
	if (!retType) {
		LogError("Unable to parse return type of function");
		return std::nullopt;
	}

	return std::make_shared<PrototypeAST>(name, std::move(argNames),
										  std::move(typeNames),
										  std::move(retType.value()));
}

std::optional<std::shared_ptr<FunctionAST>> Parser::parseDefinition() {
	current++;
	auto proto = parsePrototype();
	if (!proto) return std::nullopt;

	auto expr = parseBlock();
	if (!expr) return std::nullopt;

	return std::make_shared<FunctionAST>(std::move(proto.value()),
										 std::move(expr.value()));
}

std::optional<std::shared_ptr<ReturnStmt>> Parser::parseReturnStmt() {
	current++;
	auto expr = parseExpression();
	if (!expr) return std::nullopt;
	current++;
	return std::make_shared<ReturnStmt>(std::move(expr.value()));
}

FileAST *Parser::parse() {
	std::vector<std::shared_ptr<AST>> fileNodes;
	while (true) {
		std::cout << (int)current->type << ", " << current->lexeme << std::endl;
		switch (current->type) {
		case TokenType::EOF_TOKEN:
			return new FileAST("test", std::move(fileNodes));
		case TokenType::SEMICOLON:
			current++;
			break;
		case TokenType::FUNC: {
			auto definition = parseDefinition();
			if (!definition) continue;
			fileNodes.push_back(std::move(definition.value()));
			// fileNodes.push_back(
			// static_cast<ExprAST *>(definition.value().release()));
		} break;
		default: {
			LogError(fmt::format("Unexpected character '{}'", current->lexeme)
						 .c_str());
			current++;
		} break;
		}
	}
}
} // namespace FoxLang
