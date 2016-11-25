#include "Parser.h"

namespace xerxzema
{


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
	if(token->type == TokenType::Sub)
		return 10;
	if(token->type == TokenType::Mul)
		return 20;
	if(token->type == TokenType::Div)
		return 20;
	if(token->type == TokenType::Mod)
		return 20;
	if(token->type == TokenType::Pow)
		return 30;
	if(token->type == TokenType::Seperator)
		return 5;
	if(token->type == TokenType::Assign)
		return 4;
	if(token->type == TokenType::Bind)
		return 1;
	if(token->type == TokenType::GroupBegin)
		return 1000;
	if(token->type == TokenType::Term)
		return 1;
	return -1;
}

std::unique_ptr<Expression> null_denotation(Lexer& lexer, std::unique_ptr<Token>&& token)
{
	if(token->type == TokenType::Symbol)
		return std::make_unique<SymbolExpression>(std::move(token));
	if(token->type == TokenType::Sub)
		return std::make_unique<NegateExpression>(expression(lexer, 1000));
	if(token->type == TokenType::GroupBegin)
	{
		if(lexer.peek()->type == TokenType::GroupEnd)
		{
			//TODO unit expression
		}

		auto v = std::make_unique<GroupExpression>(expression(lexer, 0));
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
	if(token->type == TokenType::BlockBegin)
	{
		auto b = std::make_unique<StatementBlock>();
		while(true)
		{
			if(lexer.peek()->type == TokenType::BlockEnd ||
			   lexer.peek()->type == TokenType::Eof)
				break;
			b->add(expression(lexer, 0));
		}

		if(lexer.peek()->type == TokenType::BlockEnd)
		{
			lexer.get();
			return b;
		}
		//TODO missing block end...
	}
	return std::make_unique<InvalidNullDetonation>(std::move(token));
}

std::unique_ptr<Expression> left_denotation(Lexer& lexer, std::unique_ptr<Expression>&& expr,
											std::unique_ptr<Token>&& token)
{
	if(token->type == TokenType::Add)
		return std::make_unique<AddExpression>(std::move(expr), expression(lexer, 10));
	if(token->type == TokenType::Sub)
		return std::make_unique<SubExpression>(std::move(expr), expression(lexer, 10));
	if(token->type == TokenType::Mul)
		return std::make_unique<MulExpression>(std::move(expr), expression(lexer, 20));
	if(token->type == TokenType::Div)
		return std::make_unique<DivExpression>(std::move(expr), expression(lexer, 20));
	if(token->type == TokenType::Mod)
		return std::make_unique<ModExpression>(std::move(expr), expression(lexer, 20));
	if(token->type == TokenType::Pow)
		return std::make_unique<PowExpression>(std::move(expr), expression(lexer, 29));
	if(token->type == TokenType::Seperator)
		return std::make_unique<ArgListExpression>(std::move(expr), expression(lexer, 5));
	if(token->type == TokenType::Assign)
		return std::make_unique<AssignExpression>(std::move(expr), expression(lexer, 4));
	if(token->type == TokenType::Bind)
		return std::make_unique<BindExpression>(std::move(expr), expression(lexer, 1));
	if(token->type == TokenType::Term)
		return std::make_unique<Statement>(std::move(expr));
	if(token->type == TokenType::GroupBegin)
	{
		if(lexer.peek()->type == TokenType::GroupEnd)
		{
			//TODO unit expression.
		}
		auto v = std::make_unique<CallExpression>(std::move(expr), expression(lexer, 0));
		if(lexer.peek()->type == TokenType::GroupEnd)
		{
			lexer.get();
			return v;
		}
		else
		{
			return std::make_unique<InvalidLeftDetonation>(std::move(v), std::move(token));
		}
	}
	return std::make_unique<InvalidLeftDetonation>(std::move(expr), std::move(token));
}


};
