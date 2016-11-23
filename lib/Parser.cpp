#include "Parser.h"

namespace xerxzema
{

SymbolExpression::SymbolExpression(std::unique_ptr<Token>&& t) : token(std::move(t))
{
}

std::string SymbolExpression::show()
{
	return "(symbol " + token->data + ")";
}

BinaryExpression::BinaryExpression(std::unique_ptr<Expression>&& l, std::unique_ptr<Expression>&& r) :
	lhs(std::move(l)), rhs(std::move(r))

{
}

MulExpression::MulExpression(std::unique_ptr<Expression>&& l, std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(l), std::move(r))

{
}

std::string MulExpression::show()
{
	return "(mul " + lhs->show() + " " + rhs->show() + ")";
}

AddExpression::AddExpression(std::unique_ptr<Expression>&& l, std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(l), std::move(r))
{
}

std::string AddExpression::show()
{
	return "(add " + lhs->show() + " " + rhs->show() + ")";
}

ArgListExpression::ArgListExpression(std::unique_ptr<Expression>&& l, std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(l), std::move(r))
{
}

std::string ArgListExpression::show()
{
	return "(arg-list " + lhs->show() + " " + rhs->show() + ")";
}

GroupExpression::GroupExpression(std::unique_ptr<Expression>&& e) : expr(std::move(e))
{
}

std::string GroupExpression::show()
{
	return "(group " + expr->show() + ")";
}

CallExpression::CallExpression(std::unique_ptr<Expression>&& t, std::unique_ptr<Expression>&& a ) :
	target(std::move(t)), args(std::move(a))
{
}

std::string CallExpression::show()
{
	return "(call " + target->show() + " " + args->show() + ")";
}

std::string Expression::show()
{
	return "unimplemented";
}

std::unique_ptr<Expression> expression(Lexer& lexer, int right_bind)
{
	auto token = lexer.get();
	auto left = null_denotation(lexer, std::move(token));
	while(right_bind < left_bind(lexer.peek()))
	{
		token = lexer.get(); //advance
		left = left_denotation(lexer, std::move(left), std::move(token));
	}
	return left;
}

int left_bind(Token* token)
{
	if(token->type == TokenType::Symbol)
		return 0;
	if(token->type == TokenType::Add)
		return 10;
	if(token->type == TokenType::Mul)
		return 20;
	if(token->type == TokenType::Seperator)
		return 5;
	if(token->type == TokenType::GroupBegin)
		return 1000;
	return -1;
}

std::unique_ptr<Expression> null_denotation(Lexer& lexer, std::unique_ptr<Token>&& token)
{
	if(token->type == TokenType::Symbol)
		return std::make_unique<SymbolExpression>(std::move(token));
	if(token->type == TokenType::GroupBegin)
	{
		auto v = std::make_unique<GroupExpression>(expression(lexer));
		if(lexer.peek()->type == TokenType::GroupEnd)
		{
			lexer.get();
			return v;
		}
		else
		{
			//TODO error/invalid expression.
		}
	}
	return nullptr;
}

std::unique_ptr<Expression> left_denotation(Lexer& lexer, std::unique_ptr<Expression>&& expr,
											std::unique_ptr<Token>&& token)
{
	if(token->type == TokenType::Add)
		return std::make_unique<AddExpression>(std::move(expr), expression(lexer, 10));
	if(token->type == TokenType::Mul)
		return std::make_unique<MulExpression>(std::move(expr), expression(lexer, 20));
	if(token->type == TokenType::Seperator)
		return std::make_unique<ArgListExpression>(std::move(expr), expression(lexer, 5));
	if(token->type == TokenType::GroupBegin)
	{
		auto v = std::make_unique<CallExpression>(std::move(expr), expression(lexer, 0));
		if(lexer.peek()->type == TokenType::GroupEnd)
		{
			lexer.get();
			return v;
		}
		else
		{
			//TODO error/invalid expression.
		}
	}
	return nullptr;
}


};
