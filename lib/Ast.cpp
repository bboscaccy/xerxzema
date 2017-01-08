#include "Ast.h"

namespace xerxzema
{

Expression::Expression(std::unique_ptr<Token>&& t) : token(std::move(t))
{
}

SymbolExpression::SymbolExpression(std::unique_ptr<Token>&& t) : Expression(std::move(t))
{
}

std::string SymbolExpression::show()
{
	return "(symbol " + token->data + ")";
}

void SymbolExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

RealExpression::RealExpression(std::unique_ptr<Token>&& t) : Expression(std::move(t))
{
}

std::string RealExpression::show()
{
	return "(real " + token->data + ")";
}

void RealExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

StringExpression::StringExpression(std::unique_ptr<Token>&& t) : Expression(std::move(t))
{
}

std::string StringExpression::show()
{
	return "(string " + token->data + ")";
}

void StringExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}



BinaryExpression::BinaryExpression(std::unique_ptr<Token>&& t,
								   std::unique_ptr<Expression>&& l,
								   std::unique_ptr<Expression>&& r) :
	lhs(std::move(l)), rhs(std::move(r)), Expression(std::move(t))

{
}

AnnotationExpression::AnnotationExpression(std::unique_ptr<Token>&& t,
										   std::unique_ptr<Expression>&& l,
										   std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(t), std::move(l), std::move(r))

{
}

std::string AnnotationExpression::show()
{
	return "(annotation " + lhs->show() + " " + rhs->show() + ")";
}

void AnnotationExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

MulExpression::MulExpression(std::unique_ptr<Token>&& t,
							 std::unique_ptr<Expression>&& l,
							 std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(t), std::move(l), std::move(r))

{
}

void MulExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

std::string MulExpression::show()
{
	return "(mul " + lhs->show() + " " + rhs->show() + ")";
}

DivExpression::DivExpression(std::unique_ptr<Token>&& t,
							 std::unique_ptr<Expression>&& l,
							 std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(t), std::move(l), std::move(r))

{
}

void DivExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

std::string DivExpression::show()
{
	return "(div " + lhs->show() + " " + rhs->show() + ")";
}

ModExpression::ModExpression(std::unique_ptr<Token>&& t,
							 std::unique_ptr<Expression>&& l,
							 std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(t), std::move(l), std::move(r))

{
}

void ModExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

std::string ModExpression::show()
{
	return "(mod " + lhs->show() + " " + rhs->show() + ")";
}

PowExpression::PowExpression(std::unique_ptr<Token>&& t,
							 std::unique_ptr<Expression>&& l,
							 std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(t), std::move(l), std::move(r))

{
}

std::string PowExpression::show()
{
	return "(pow " + lhs->show() + " " + rhs->show() + ")";
}

void PowExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

AddExpression::AddExpression(std::unique_ptr<Token>&& t,
							 std::unique_ptr<Expression>&& l,
							 std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(t), std::move(l), std::move(r))
{
}

void AddExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

std::string AddExpression::show()
{
	return "(add " + lhs->show() + " " + rhs->show() + ")";
}

SubExpression::SubExpression(std::unique_ptr<Token>&& t,
							 std::unique_ptr<Expression>&& l,
							 std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(t), std::move(l), std::move(r))
{
}

std::string SubExpression::show()
{
	return "(sub " + lhs->show() + " " + rhs->show() + ")";
}

void SubExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

ArgListExpression::ArgListExpression(std::unique_ptr<Token>&& t,
									 std::unique_ptr<Expression>&& l,
									 std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(t), std::move(l), std::move(r))
{
}

std::string ArgListExpression::show()
{
	return "(arg-list " + lhs->show() + " " + rhs->show() + ")";
}

void ArgListExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

GroupExpression::GroupExpression(std::unique_ptr<Token>&& t,
								 std::unique_ptr<Expression>&& e) :
	expr(std::move(e)), Expression(std::move(t))
{
}

std::string GroupExpression::show()
{
	return "(group " + expr->show() + ")";
}

void GroupExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

NegateExpression::NegateExpression(std::unique_ptr<Token>&& t,
								   std::unique_ptr<Expression>&& e) :
	expr(std::move(e)), Expression(std::move(t))
{
}

void NegateExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

std::string NegateExpression::show()
{
	return "(negate " + expr->show() + ")";
}

CallExpression::CallExpression(std::unique_ptr<Token>&& tk,
							   std::unique_ptr<Expression>&& t,
							   std::unique_ptr<Expression>&& a ) :
	target(std::move(t)), args(std::move(a)), Expression(std::move(tk))
{
}

SampleExpression::SampleExpression(std::unique_ptr<Token>&& t,
								   std::unique_ptr<Expression>&& e) :
	expr(std::move(e)), Expression(std::move(t))
{
}

std::string SampleExpression::show()
{
	return "(sample " + expr->show() + ")";
}

void SampleExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

std::string CallExpression::show()
{
	if(args)
		return "(call " + target->show() + " " + args->show() + ")";
	else
		return "(call " + target->show() + " (unit))";
}

void CallExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

AssignExpression::AssignExpression(std::unique_ptr<Token>&& t,
								   std::unique_ptr<Expression>&& l,
								   std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(t), std::move(l), std::move(r))
{
}

std::string AssignExpression::show()
{
	return "(assign " + lhs->show() + " " + rhs->show() + ")";
}

void AssignExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

BindExpression::BindExpression(std::unique_ptr<Token>&& t,
							   std::unique_ptr<Expression>&& l,
							   std::unique_ptr<Expression>&& r) :
	BinaryExpression(std::move(t), std::move(l), std::move(r))
{
}

std::string BindExpression::show()
{
	return "(bind " + lhs->show() + " " + rhs->show() + ")";
}

void BindExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

InvalidNullDetonation::InvalidNullDetonation(std::unique_ptr<Token>&& t) :
	Expression(std::move(t))
{
}

std::string InvalidNullDetonation::show()
{
	return "(invalid-null (token '" + token->data + "'))";
}

void InvalidNullDetonation::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

InvalidLeftDetonation::InvalidLeftDetonation(std::unique_ptr<Token>&& t,
											 std::unique_ptr<Expression>&& e)
	: Expression(std::move(t)), expr(std::move(e))
{
}

std::string InvalidLeftDetonation::show()
{
	return "(invalid-left " + expr->show() + " (token '" +  token->data + "'))";
}

void InvalidLeftDetonation::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}
StatementBlock::StatementBlock(std::unique_ptr<Token>&& t) : Expression(std::move(t))
{
}

std::string	StatementBlock::show()
{
	std::string res = "{";
	for(auto& e:expressions)
	{
		res += e->show();
	}
	res += "}";
	return res;
}

void StatementBlock::add(std::unique_ptr<Expression>&& expr)
{
	expressions.push_back(std::move(expr));
}

void StatementBlock::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

Statement::Statement(std::unique_ptr<Token>&& t,
					 std::unique_ptr<Expression>&& e) :
	Expression(std::move(t)), expr(std::move(e))
{
}

std::string Statement::show()
{
	return "[" + expr->show() + "]";
}

void Statement::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

WithStatement::WithStatement(std::unique_ptr<Token>&& t,
							 std::unique_ptr<Expression>&& l,
							 std::unique_ptr<Expression>&& r) :
	Expression(std::move(t)), with_clause(std::move(l)), statements(std::move(r))
{
}

std::string WithStatement::show()
{
	return "[with " + with_clause->show() + " " + statements->show() + "]";
}

void WithStatement::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

CodeDefinition::CodeDefinition(std::unique_ptr<Token>&& d,
							   std::unique_ptr<Expression>&& l,
							   std::unique_ptr<Expression>&& r) :
	Expression(std::move(d)), signature(std::move(l)), body(std::move(r))
{
}

std::string CodeDefinition::show()
{
	return "[" + token->data + " " + signature->show() + " "
		+ body->show() + "]";
}

void CodeDefinition::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

SequenceExpression::SequenceExpression(std::unique_ptr<Token>&& d,
									   std::unique_ptr<Expression>&& i) :
	Expression(std::move(d)), items(std::move(i))
{
}

void SequenceExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

std::string SequenceExpression::show()
{
	return "(seq " + items->show() + ")";
}

ListExpression::ListExpression(std::unique_ptr<Token>&& d,
									   std::unique_ptr<Expression>&& i) :
	Expression(std::move(d)), items(std::move(i))
{
}

void ListExpression::accept(xerxzema::AstVisitor &v)
{
	v.visit(this);
}

std::string ListExpression::show()
{
	return "(list " + items->show() + ")";
}

std::string Expression::show()
{
	return "unimplemented";
}

void AstVisitor::visit(StatementBlock *e)
{
	handle_default(e);
}
void AstVisitor::visit(Statement *e)
{
	handle_default(e);
}
void AstVisitor::visit(WithStatement *e)
{
	handle_default(e);
}
void AstVisitor::visit(CodeDefinition *e)
{
	handle_default(e);
}
void AstVisitor::visit(SymbolExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(RealExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(StringExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(AnnotationExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(AddExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(SubExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(MulExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(DivExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(ModExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(PowExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(ArgListExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(GroupExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(SampleExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(CallExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(BindExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(AssignExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(NegateExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(SequenceExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(ListExpression *e)
{
	handle_default(e);
}
void AstVisitor::visit(InvalidLeftDetonation *e)
{
	handle_default(e);
}
void AstVisitor::visit(InvalidNullDetonation *e)
{
	handle_default(e);
}
};
