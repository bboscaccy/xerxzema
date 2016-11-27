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
	ns(n), def(d), prog(nullptr), valid(true)
{
}

void HandleCodeDefinitionSignature::handle_default(xerxzema::Expression *e)
{
	//TODO error.
	valid = false;
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
		state = ProcessState::OutputArgs;
		rhs->accept(*this);
	}
	else
	{
		valid = false;
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
	else
	{
		valid = false;
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
	else
	{
		valid = false;
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
	else if(state == ProcessState::OutputArgs)
	{
		e->lhs->accept(*this);
		state = ProcessState::OutputArgs;
		e->rhs->accept(*this);
	}
	else
	{
		valid = false;
	}
}

void HandleCodeDefinitionSignature::visit(xerxzema::AnnotationExpression *e)
{
	if(state == ProcessState::InputArgs || state == ProcessState::OutputArgs)
	{
		current_arg_name = "";
		current_arg_type = "";
		auto last_state = state;
		state = ProcessState::GetArgName;
		e->lhs->accept(*this);
		state = ProcessState::GetArgType;
		e->rhs->accept(*this);

		if(ns->is_type(current_arg_type))
		{
			if(last_state == ProcessState::InputArgs)
				prog->add_input(current_arg_name, ns->type(current_arg_type));
			if(last_state == ProcessState::OutputArgs)
				prog->add_output(current_arg_name, ns->type(current_arg_type));
		}
		else
		{
			valid = false;
		}
	}
}

HandleStatement::HandleStatement(Program* p, Expression* e) : program(p), expr(e), valid(true)
{
}

void HandleStatement::process()
{
	state = ProcessState::Entry;
	expr->accept(*this);
}

void HandleStatement::visit(Statement *e)
{
	//is valid top expression next?
	//bind, call, bang, merge, etc..
}

void HandleStatement::visit(StatementBlock *e)
{
	for(auto& s: e->expressions)
	{
		e->accept(*this);
	}
}

void HandleStatement::visit(xerxzema::WithStatement *e)
{

}

void HandleStatement::handle_default(xerxzema::Expression *e)
{

}

};
