#include "Parser.h"
#include "Diagnostics.h"

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
	if(token->type == TokenType::Real)
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
	if(token->type == TokenType::Delimit)
		return 100;
	if(token->type == TokenType::GroupBegin)
		return 1000;
	if(token->type == TokenType::Term)
		return 1;
	return -1;
}

std::unique_ptr<Expression> null_denotation(Lexer& lexer, std::unique_ptr<Token>&& token)
{
	auto diag = token.get();

	if(token->type == TokenType::Symbol)
		return std::make_unique<SymbolExpression>(std::move(token));
	if(token->type == TokenType::Real)
		return std::make_unique<RealExpression>(std::move(token));
	if(token->type == TokenType::Sub)
		return std::make_unique<NegateExpression>(std::move(token), expression(lexer, 1000));
	if(token->type == TokenType::Sample)
		return std::make_unique<SampleExpression>(std::move(token), expression(lexer, 1000));
	if(token->type == TokenType::GroupBegin)
	{
		if(lexer.peek()->type == TokenType::GroupEnd)
		{
			//TODO unit expression
		}
		auto v = std::make_unique<GroupExpression>(std::move(token), expression(lexer, 0));
		if(lexer.peek()->type == TokenType::GroupEnd)
		{
			lexer.get();
			return v;
		}
		else
		{
			emit_error(diag, "Missing closing parenthesis");
			return std::make_unique<InvalidNullDetonation>(std::move(v->token));
		}
	}
	if(token->type == TokenType::BlockBegin)
	{
		auto b = std::make_unique<StatementBlock>(std::move(token));
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
		emit_error(diag, "Missing closing brace");
		return std::make_unique<InvalidNullDetonation>(std::move(b->token));
	}
	if(token->type == TokenType::With)
	{
		auto clause = expression(lexer, 0);
		auto instructions = expression(lexer, 0);
		return std::make_unique<WithStatement>(std::move(token),
											   std::move(clause), std::move(instructions));
	}
	if(token->type == TokenType::ProgKeyword ||
	   token->type == TokenType::UgenKeyword ||
	   token->type == TokenType::FuncKeyword)
	{
		auto sig = expression(lexer, 0);
		auto body = expression(lexer, 0);
		return std::make_unique<CodeDefinition>(std::move(token), std::move(sig),
												std::move(body));
	}
	emit_error(diag, "I can't understand this");
	return std::make_unique<InvalidNullDetonation>(std::move(token));
}

std::unique_ptr<Expression> left_denotation(Lexer& lexer, std::unique_ptr<Expression>&& expr,
											std::unique_ptr<Token>&& token)
{
	auto diag = token.get();
	if(token->type == TokenType::Add)
		return std::make_unique<AddExpression>(std::move(token),
											   std::move(expr), expression(lexer, 10));
	if(token->type == TokenType::Sub)
		return std::make_unique<SubExpression>(std::move(token),
											   std::move(expr), expression(lexer, 10));
	if(token->type == TokenType::Mul)
		return std::make_unique<MulExpression>(std::move(token),
											   std::move(expr), expression(lexer, 20));
	if(token->type == TokenType::Div)
		return std::make_unique<DivExpression>(std::move(token),
											   std::move(expr), expression(lexer, 20));
	if(token->type == TokenType::Mod)
		return std::make_unique<ModExpression>(std::move(token),
											   std::move(expr), expression(lexer, 20));
	if(token->type == TokenType::Pow)
		return std::make_unique<PowExpression>(std::move(token),
											   std::move(expr), expression(lexer, 29));
	if(token->type == TokenType::Delimit)
		return std::make_unique<AnnotationExpression>(std::move(token),
													  std::move(expr), expression(lexer, 100));
	if(token->type == TokenType::Seperator)
		return std::make_unique<ArgListExpression>(std::move(token),
												   std::move(expr), expression(lexer, 5));
	if(token->type == TokenType::Assign)
		return std::make_unique<AssignExpression>(std::move(token),
												  std::move(expr), expression(lexer, 4));
	if(token->type == TokenType::Bind)
		return std::make_unique<BindExpression>(std::move(token),
												std::move(expr), expression(lexer, 1));
	if(token->type == TokenType::Term)
		return std::make_unique<Statement>(std::move(token),
										   std::move(expr));
	if(token->type == TokenType::GroupBegin)
	{
		if(lexer.peek()->type == TokenType::GroupEnd)
		{
			//TODO unit expression.
		}
		auto v = std::make_unique<CallExpression>(std::move(token),
												  std::move(expr), expression(lexer, 0));
		if(lexer.peek()->type == TokenType::GroupEnd)
		{
			lexer.get();
			return v;
		}
		else
		{
			emit_error(diag, "Missing closing parenthesis");
			return std::make_unique<InvalidLeftDetonation>(std::move(v->token), std::move(expr));
		}
	}
	emit_error(token.get(), "I can't understand this");
	return std::make_unique<InvalidLeftDetonation>(std::move(token), std::move(expr));
}


};
