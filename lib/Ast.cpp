#include "Ast.h"

namespace xerxzema
{

SymbolExpression::SymbolExpression(std::unique_ptr<Token>&& t) : token(std::move(t))
{
}

std::string SymbolExpression::show()
{
	return "(symbol " + token->data + ")";
}

BinaryExpression::BinaryExpression(std::unique_ptr<Expression>&& l,
								   std::unique_ptr<Expression>&& r) :
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

SubExpression::SubExpression(std::unique_ptr<Expression>&& l, std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(l), std::move(r))
{
}

std::string SubExpression::show()
{
	return "(sub " + lhs->show() + " " + rhs->show() + ")";
}


ArgListExpression::ArgListExpression(std::unique_ptr<Expression>&& l,
									 std::unique_ptr<Expression>&& r) :
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

NegateExpression::NegateExpression(std::unique_ptr<Expression>&& e) : expr(std::move(e))
{
}

std::string NegateExpression::show()
{
	return "(negate " + expr->show() + ")";
}

CallExpression::CallExpression(std::unique_ptr<Expression>&& t,
							   std::unique_ptr<Expression>&& a ) :
	target(std::move(t)), args(std::move(a))
{
}

std::string CallExpression::show()
{
	if(args)
		return "(call " + target->show() + " " + args->show() + ")";
	else
		return "(call " + target->show() + " (unit))";
}

AssignExpression::AssignExpression(std::unique_ptr<Expression>&& l,
								   std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(l), std::move(r))
{
}

std::string AssignExpression::show()
{
	return "(assign " + lhs->show() + " " + rhs->show() + ")";
}

BindExpression::BindExpression(std::unique_ptr<Expression>&& l,
							   std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(l), std::move(r))
{
}

std::string BindExpression::show()
{
	return "(bind " + lhs->show() + " " + rhs->show() + ")";
}

InvalidNullDetonation::InvalidNullDetonation(std::unique_ptr<Token>&& t) : token(std::move(t))
{
}

std::string InvalidNullDetonation::show()
{
	return "(invalid-null (token '" + token->data + "'))";
}

InvalidLeftDetonation::InvalidLeftDetonation(std::unique_ptr<Expression>&& e,
											 std::unique_ptr<Token>&& t)
	: token(std::move(t)), expr(std::move(e))
{
}

std::string InvalidLeftDetonation::show()
{
	return "(invalid-left " + expr->show() + " (token '" +  token->data + "'))";
}


std::string Expression::show()
{
	return "unimplemented";
}
};
