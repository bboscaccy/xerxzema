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

class SubExpression : public BinaryExpression
{
public:
	SubExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
};

class MulExpression : public BinaryExpression
{
public:
	MulExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
};

class DivExpression : public BinaryExpression
{
public:
	DivExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
};

class ModExpression : public BinaryExpression
{
public:
	ModExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
};

class PowExpression : public BinaryExpression
{
public:
	PowExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
};

class ArgListExpression : public BinaryExpression
{
public:
	ArgListExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
};

class GroupExpression : public Expression
{
public:
	GroupExpression(std::unique_ptr<Expression>&& expr);
	std::unique_ptr<Expression> expr;
	std::string show();
};

class NegateExpression : public Expression
{
public:
	NegateExpression(std::unique_ptr<Expression>&& expr);
	std::unique_ptr<Expression> expr;
	std::string show();
};

class CallExpression : public Expression
{
public:
	CallExpression(std::unique_ptr<Expression>&& target, std::unique_ptr<Expression>&& args);
	std::unique_ptr<Expression> target;
	std::unique_ptr<Expression> args;
	std::string show();
};

class AssignExpression : public BinaryExpression
{
public:
	AssignExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
};

class BindExpression : public BinaryExpression
{
public:
	BindExpression(std::unique_ptr<Expression>&& lhs, std::unique_ptr<Expression>&& rhs);
	std::string show();
};


class InvalidNullDetonation : public Expression
{
public:
	InvalidNullDetonation(std::unique_ptr<Token>&& token);
	std::unique_ptr<Token> token;
	std::string show();
};

class InvalidLeftDetonation : public Expression
{
public:
	InvalidLeftDetonation(std::unique_ptr<Expression>&& expr, std::unique_ptr<Token>&& token);
	std::unique_ptr<Expression> expr;
	std::unique_ptr<Token> token;
	std::string show();
};



};
