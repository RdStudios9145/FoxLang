#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "tokens.h"

namespace FoxLang {
class Tokenizer {
public:
	Tokenizer(std::string *source);

	std::vector<Token> *Tokenize();

private:
	inline bool AtEnd();

	void currentToken();

	bool isDigit(char c);

	bool isAlpha(char c);

	bool isAlphaNumeric(char c);

	bool isAlphaNumericUnicode(char c);

	bool isANUcheckBack(int n);

	char peek();

	char peekNext(int n);

	char peekBack(int n);

	char advance();

	void addToken(TokenType token);

	bool match(char expected);

	char peekNext();

	void string();

	void number();

	void identifier();

private:
	std::string *source;
	std::vector<Token> tokens;
	unsigned long int current = 0;
	unsigned long int start = 0;
	int line = 1;

	// clang-format off
	struct {
		std::string name;
		TokenType value;
	} keywords[13] = {
		{.name = "struct",	.value = TokenType::STRUCT},
		{.name = "else",	.value = TokenType::ELSE},
		{.name = "false",	.value = TokenType::FALSE},
		{.name = "for",		.value = TokenType::FOR},
		{.name = "fn",		.value = TokenType::FUNC},
		{.name = "if",		.value = TokenType::IF},
		{.name = "return",	.value = TokenType::RETURN},
		{.name = "self",	.value = TokenType::SELF},
		{.name = "true",	.value = TokenType::TRUE},
		{.name = "let",		.value = TokenType::LET},
		{.name = "const",	.value = TokenType::CONST},
		{.name = "while",	.value = TokenType::WHILE},
		{.name = "extern",	.value = TokenType::EXTERN}
	};

	// clang-format on
};
} // namespace FoxLang
