#include "Semantic.h"

namespace xerxzema
{

HandleTopLevelExpression::HandleTopLevelExpression(Namespace* n) : ns(n)
{
}

void HandleTopLevelExpression::handle_default(xerxzema::Expression *e)
{
	//TODO flag this as invalid
	valid = false;
}

void HandleTopLevelExpression::visit(xerxzema::CodeDefinition *e)
{
	valid = true;
}

HandleCodeDefinitionSignature::HandleCodeDefinitionSignature(Namespace* n, CodeDefinition* d) :
	ns(n), def(d), prog(nullptr)
{
}

void HandleCodeDefinitionSignature::handle_default(xerxzema::Expression *e)
{
	//TODO error.
}

void HandleCodeDefinitionSignature::process()
{
	state = ProcessState::Entry;
	def->signature->accept(*this);
}

void HandleCodeDefinitionSignature::visit(BindExpression *e)
{
	if(state == ProcessState::Entry)
	{
		auto lhs = e->lhs.get();
		auto rhs = e->rhs.get();
		state = ProcessState::Lhs;
		lhs->accept(*this);
		state = ProcessState::Rhs;
		rhs->accept(*this);
	}
}

void HandleCodeDefinitionSignature::visit(CallExpression* e)
{
	if(state == ProcessState::Lhs)
	{
		auto target = e->target.get();
		state = ProcessState::Name;
		target->accept(*this);
	}
}

void HandleCodeDefinitionSignature::visit(SymbolExpression* e)
{
	if(state == ProcessState::Name)
	{
		prog = ns->get_program(e->token->data);
	}
}
};
