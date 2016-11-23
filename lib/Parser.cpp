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
	return -1;
}

std::unique_ptr<Expression> null_denotation(Lexer& lexer, std::unique_ptr<Token>&& token)
{
	if(token->type == TokenType::Symbol)
		return std::make_unique<SymbolExpression>(std::move(token));
	return nullptr;
}

std::unique_ptr<Expression> left_denotation(Lexer& lexer, std::unique_ptr<Expression>&& expr,
											std::unique_ptr<Token>&& token)
{
	if(token->type == TokenType::Add)
		return std::make_unique<AddExpression>(std::move(expr), expression(lexer, 10));
	if(token->type == TokenType::Mul)
		return std::make_unique<MulExpression>(std::move(expr), expression(lexer, 20));
	return nullptr;
}


};
