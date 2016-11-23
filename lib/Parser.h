#pragma once

#include <string>
#include <vector>
#include <memory>
#include <istream>

#include "Lexer.h"

namespace xerxzema
{

class Expression
{
public:
	virtual std::string show();
};

class SymbolExpression : public Expression
{
public:
	SymbolExpression(std::unique_ptr<Token>&& token);
	std::unique_ptr<Token> token;
	std::string show();
};

class BinaryExpression : public Expression
{
public:
	BinaryExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::unique_ptr<Expression> lhs;
	std::unique_ptr<Expression> rhs;
};

class AddExpression : public BinaryExpression
{
public:
	AddExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();

};

class MulExpression : public BinaryExpression
{
public:
	MulExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
};

class ArgListExpression : public BinaryExpression
{
public:
	ArgListExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
};


std::unique_ptr<Expression> expression(Lexer& lexer, int right_bind = 0);
std::unique_ptr<Expression> null_denotation(Lexer& lexer, std::unique_ptr<Token>&& token);
std::unique_ptr<Expression> left_denotation(Lexer& lexer, std::unique_ptr<Expression>&& expr,
											std::unique_ptr<Token>&& token);
int left_bind(Token* token);

};
