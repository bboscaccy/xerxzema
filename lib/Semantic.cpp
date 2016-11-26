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
		auto args = e->args.get();
		state = ProcessState::Name;
		target->accept(*this);
		state = ProcessState::InputArgs;
		args->accept(*this);
	}
}

void HandleCodeDefinitionSignature::visit(SymbolExpression* e)
{
	if(state == ProcessState::Name)
	{
		prog = ns->get_program(e->token->data);
	}
	else if(state == ProcessState::GetArgName)
	{
		current_arg_name = e->token->data;
	}
	else if(state == ProcessState::GetArgType)
	{
		current_arg_type = e->token->data;
	}
}

void HandleCodeDefinitionSignature::visit(xerxzema::ArgListExpression *e)
{
	if(state == ProcessState::InputArgs)
	{
		e->lhs->accept(*this);
		state = ProcessState::InputArgs;
		e->rhs->accept(*this);
	}
}

void HandleCodeDefinitionSignature::visit(xerxzema::AnnotationExpression *e)
{
	if(state == ProcessState::InputArgs)
	{
		current_arg_name = "";
		current_arg_type = "";
		state = ProcessState::GetArgName;
		e->lhs->accept(*this);
		state = ProcessState::GetArgType;
		e->rhs->accept(*this);

		if(ns->is_type(current_arg_type))
		{
			prog->add_input(current_arg_name, ns->type(current_arg_type));
		}
	}
}

};
